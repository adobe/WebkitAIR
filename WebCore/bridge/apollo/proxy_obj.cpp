/*
 * Copyright (C) 2007 Adobe Systems Incorporated.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Adobe Systems Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ADOBE SYSTEMS INCORPORATED "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ADOBE SYSTEMS INCORPORATED
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"
#include "proxy_obj.h"
#include "JSObject.h"
#include "JSArray.h"
#include "ArrayPrototype.h"
#include "BooleanObject.h"
#include "NumberObject.h"
#include "StringPrototype.h"
#include "StringObject.h"
#include "JSLock.h"
#include "Error.h"
#include <WebKitApollo/helpers/WebScriptProxyHelper.h>

#include <wtf/Vector.h>
#include <stdlib.h>
#include "PropertyNameArray.h"
#include <wtf/MathExtras.h>

#include "JSDOMWindow.h"
#include "JSDOMWindowShell.h"
#include "ScriptController.h"
#include <Frame.h>
#include "JSNodeList.h"
#include "JSHTMLCollection.h"
#include "Structure.h"
#include "JSImmediate.h"
#include "StringBuffer.h"

/* **DESIGN NOTE**
   WebKit already has a proxying/bridging mechanism implemented by RuntimeObjectImp and friends. Why
   are we not using that? 
   1. That mechanism seems to be built primarily to wrap strongly typed languages, as it has a built-in
      notion of classes, methods being distinct from props, etc. There is a bunch of machinery there
      that we simply do not need in order to essentially bridge JavaScript to JavaScript.
   2. That mechanism introduces an extra layer of indirection ("Instance") that we do not need and which
      will have a performance cost.
   3. Our bridging system is known to be JS<-->AS so we can modify it as we see fit to make this work
      really well. 
   4. That mechanism doesn't support bridging to the same depth that we are going for. We are trying
      to transparently bridge JS<-->AS in every way, including e.g. GC bridging.

   So the summary is that doing it ourselves is simpler, higher performance, and gives us more flexibility.
*/


/*
DESIGN NOTE - Discussion of Interpreters, ExecState, Context 

Background
----------
In JavaScriptCore, you need an ExecState object to execute pretty much any JavaScript code. An ExecState object
references both a Context object (used to track the current scope chain, "this", etc.) and an Interpreter object 
(tracks script timeout, points at a global object, points at built-in obj protos, etc.). In WebKit, there is a 
one-to-one correspondance between frames and Interpreters, with the Interpreter pointing at the window object 
of the frame as its global object.

When code calling in from outside JavaScriptCore (e.g. from plugin or AS3 code) wants to run JavaScript code, it must 
come up with an ExecState to run JS code, do property gets, etc. The typical way of doing this is to acquire an 
Interpreter in some way, and then ask it for its "globalExec" which is an ExecState with no Context and which points
back at the Interpreter, and then feed this ExecState into methods that need it. Using globalExec in this manner is 
basically equivalent to running code in the global scope of the Interpreter's associated frame.

There are a variety of places where the state provided in the Interpreter/ExecState/Context is relevant. The most
common things accessed are the exception info in the ExecState, the ScopeChain and "this" info in the Context, and
the global object/frame in the Interpreter. Note that in most cases, this information is updated as execution flows
through JavaScriptCore (e.g. when a function is called a new Context is created which points at the function's 
scope chain and the ExecState is updated to point at the new Context), and the initial values should affect only the 
first code called with the Interpreter/ExecState/Context. Unfortunately, at the time of this writing (10/16/2007) 
there are a number of places where the current ("dynamic") Interpreter's global object is accessed incorrectly. 
These are bugs in Webkit and are detailed in the Interpreter::globalObject() section in scriptbridging.txt. 

It is important to note that JavaScriptCore does not segregate objects belonging to different frames (windows) or
maintain chokepoints through which these objects must communicate. Same origin checks are made only when property
gets/sets/calls are made upon a window object. If the window object returns a property, using that property or
storing it does not require any further security checks. Further, calling from code in one frame (window) into 
code in another frame (window) does not cause the current Interpreter to change to be associated with the target
frame or in any way alter the execution state to indicate transition between frames (though the scope chain of
the target function will of course have the target frame as its root).


How we deal with Interpreter/ExecState/Context in the bridging code
-------------------------------------------------------------------
Our JSCoreVariant class, which wraps a JSValue (JavaScriptCore value or object reference) so that it can be used from 
outside JavaScriptCore (e.g. by AS3 code), must be able to setup an ExecState (as described above) in order to do property 
gets, sets, etc. when the other side of the bridge (e.g. AS3 code) requests them. This is achieved by hanging onto a 
pointer to an Interpreter in the JSCoreVariant, and then when an ExecState is needed, asking the Interpreter for its 
"globalExec" (see above). We maintain a map from JSValue/Interpreter pairs to JSCoreVariants so that we can re-use the
JSCoreVariant objects whenever possible.

Invalidation: because JSCoreVariant hangs onto a raw pointer to an Interpreter (has no reference or ownership of
Interpreter) and Interpreters are bound to the lifetime of frames, it is necessary to make JSCoreVariant objects
"dead" when the Interpreters they point at are destroyed. We cannot actually destroy the JSCoreVariant, as it is 
pointed at by AS3 objects, so it goes dead and starts throwing exceptions when used.

Choosing an Interpreter: when we create a JSCoreVariant, we need to give it a pointer to an Interpreter. Taking a cue
from how WebKit handles calls to JS coming in from plugins, we simply take the current "dynamic" Interpreter. This
is basically the Interpreter associated with the current ExecState, which was setup when JavaScriptCore was entered
for the current flow of control. 

Why it matters what Interpreter we use: There are several reasons why it matters which Interpreter we associate with
a JSCoreVariant and use for calls, gets, sets, etc. coming into the JSCoreVariant:
- Interpreter destruction requires JSCoreVariant invalidation (going dead). If the JSCoreVariant wraps a JSObject
  that logically belongs to frame A (e.g. a DOM node) but references an Interpreter associated with frame B, and frame
  B is destroyed, the JSCoreVariant becomes "dead" though logically this should not be needed.
- Same-origin security checks are made based on the current Interpreter's frame's URL. These should be OK regardless
  of what Interpreter we store, as code using that Interpreter did reach the JSObject for the JSCoreVariant
- As mentioned above, bugs in WebKit cause the global object from the current Interpreter to be used. This will differ
  depending on what Interpreter is associated with a JSCoreVariant, which depends upon where execution started when
  we reached the JSCoreVariant. I have reviewed all of these cases, and most of them are rare and not super-horrible.
- Relative URLs passed to window.open() and window.location() are made absolute based on the current Interpreter's 
  frame's URL. This is a little surprising, but is normal browser behavior.
- If we wind up needing to hand the same JSValue to AS3 more than once, we will need to get a JSCoreVariant each time.
  In order for object equivalency calculations to work properly on the AS3 side, we want to reuse the exact same
  JSCoreVariant each time (providing that it has not been destructed). However, if upon needing a JSCoreVariant the 
  second time for a given JSValue, the current Interpreter differs from the one pointed at by the existing JSCoreVariant
  we create a new JSCoreVariant, breaking equivalency and reducing memory efficiency. We do this for security reasons
  (though I'm not sure I buy this totally...) and also because it would be confusing for invalidation and relative URL
  behavior to have it differ depending on whether a pre-existing JSCoreVariant was in existence.

Other approaches: as you can read above, there are clearly some drawbacks to our approach in referencing a particular
Interpreter from a JSCoreVariant. Specifically, it may be made dead before this is really needed, the wrong global 
object may be used in certain (admittedly rare) cases due to WebKit bugs, equivalency of JSCoreVariants may not match 
equivalency of the underlying JSValues, and relative URLs may be fixed up in a surprising way when on a call stack 
beginning with an operation upon the JSCoreVariant. So what other approaches did we consider?
- WebKit approach: WebKit behaves as we do, with the exception that when it goes to wrap a JS window object, it does not 
  associate the current Interpreter with the wrapper, instead it associates the Interpreter for the window (frame) object. 
  This yields somewhat nicer "deadness" behavior, though at the price of complexity and more confusing relative URL fixup 
  behavior. Complexity is added because it is now necessary to make same origin checks when the wrapper for the window object
  is accessed, because it must not be origin-checked with ITS interpreter but with the one in effect when the wrapper was
  created. Read the section in scriptbridging.txt entitled "Info on how Webkit handles bridging its JavaScript objects 
  and Netscape plugins etc." for more details. I decided not to duplicate Webkit's strategy because I did not think
  the benefits outweighed the drawbacks. If we eventually want to move to a Webkit-like approach, read the comments in
  JSCoreVariant::getJSDOMWindowForJSValue() for a discussion of what we'd need to do. Note also that the improvement of
  the invalidation behavior is not foolproof. Wrappers are associated with the "correct" Interpreter if the wrapper
  code is what crosses the Window object boundary. If a wrapper associated with Interpreter A calls a function that then
  navigates into a Window associated with Interpreter B and returns say a DOM object associated with that Window, it
  will NOT be associated with Interpreter B as you would like.
- Outermost Interpreter: instead of associating current Interpreter with JSCoreVariant, instead walk up frame tree
  (as long as same origin check succeeds) starting with current Interpreter's frame until at root or same origin check
  fails, then use that Interpreter for the JSCoreVariant. This approach has some benefits like lessening the need to 
  deadify objects by using frames (and Interpreters) higher in the hierarchy (presuming that they are destroyed less often), 
  and could aid in equivalency, but doesn't seem worth the complexity, and would make certain behaviors (URL fixup, and
  ramifications of global object usage bugs in Webkit) harder to understand/explain.
- Objects bound to Frames: the idea here is that we modify WebKit so that it DOES track which Frame a JSObject is 
  associated with. This could either be done directly (e.g. by modifying JSObject class and code that creates JSObjects
  to track this info) or indirectly with a wrapper approach (Stan Switzer's idea). Using a wrapper approach, anytime 
  a Window object was asked for a property lookup (or returned a result from a function call) by a different Window 
  (detected via dynamic Interpreter) it'd return a wrapper around the actual result. The wrapper would remember the 
  associated Window, and when it was asked for a lookup, it'd also return a wrapper when the Interpreters don't match. 
  Inside the wrapper code for get/set/invoke we'd perform operations using the wrapper's Interpreter, not the incoming
  dynamic Interpreter. Basically, this would mean that objects from a given Frame are only interacted with by that
  Frame's Interpreter. This would solve the issue mentioned above in the WebKit approach (where JS code can smuggle
  objects from a different frame out without noticing they belong to a different Interpreter) but there are problems.
  First, the wrappers defeat RTTI - e.g. when trying to add a DOM node as a child of another DOM node, you really need 
  a DOM object not a wrapper - we'd need to change the code in all such places to know how to deal with wrappers. Further
  issues include performance concerns (memory usage due to wrappers, as well as execution cost) and changes to relative
  URL fixup behavior when crossing frames.
- Find-an-Interpreter: if JSCoreVariant is going to become dead because its Interpreter is going away, instead of making
  it dead, find another Interpreter which passes the same origin check against the original Interpreter. Note that this
  could be done lazily as well (compute this when the JSCoreVariant is next used, not when it is made dead). This idea
  would lead to some very hard to understand behavior, and doesn't seem worth the complexity. Its value would be in 
  reducing deadness conditions..
- Interpreter for AS3: rather than binding JSCoreVariants to Interpreters, generate an Interpreter that corresponds
  to the calling AS3's "frame". This might be achieved by creating a ScriptInterpreter (subclass of Interpreter) and a phony
  Frame object, by finding a way for ScriptInterpreter not to need a Frame object, or by introducing another subclass
  of Interpreter (in addition to ScriptInterpreter) and making sure that code doesn't downcast to ScriptInterpreter. 
  Scriptbridging.txt contains some (somewhat dated) research on what Interpreter and ScriptInterpreter methods are used.
  This approach, while probably most difficult and requiring substantial research on how Interpreter/ScriptInterpeter
  methods are used, has the desirable properties of behaving like normal x-frame access, eliminating "deadness" entirely, 
  eliminating the equivalency problem, and causing relative URL fixup to behave as if the AS3 code was a calling HTML frame, 
  which seems nice. So why didn't we do it? It seemed too risky to introduce, and possibly fragile in the face of changes
  to WebKit. If we decide our current solution to be inadequate, I WOULD STRONGLY CONSIDER DOING THIS. Note that 
  JSCoreVariant::getExecState() is the chokepoint for where we get the ExecState in the existing code.

==> So what is the ideal solution to all of this mess? 
1. Implement the "Interpreter for AS3" approach described above, yielding the benefits described above.
2. Fix WebKit bugs mentioned above and detailed in scriptbridging.txt (this can and should be done independently of the 
   rest of this ideal solution even if we keep our existing approach). In fact, I would recommend removing globalObject() 
   access from Interpreter if possible, replacing anything that truly must be there (e.g. access to URL for same origin
   checks and relative URL fixup) with purpose-built virtual methods.
3. Set things up so that we don't switch Interpreters when JS calls into AS which then calls back into JS. In this case,
   as long as this was considered a continuation of the same call chain, we should be using the original Interpreter,
   not using a new one when we go from AS to JS. This could be tricky - we must know that the AS3 side is truly continuing
   the same thread of execution vs. starting a new one.
*/


/* DESIGN NOTE: Security considerations 

   Webkit security checks:
   Webkit performs same origin checks are by comparing a URL retrieved from the current (dynamic) Interpreter's 
   Frame and comparing it to the URL of the resource to be accessed (e.g. Window object, XHR, etc.). Thus, the 
   choice of Interpreter used for execution is important for security. The approach we've taken to Interpreter 
   selection (see above) works correctly for calls from AS3 into JS. Because we store the current Interpreter with 
   the JSCoreVariant when we create the JSCoreVariant, when we use that Interpreter for a later call from AS3
   into the JSCoreVariant it is as if we are resuming that thread of execution, so there are no new risks.

   AIR security checks:
   Access to privileged AIR APIs must be restricted such that only application content can call them. When calling
   across the bridge from JS to AS, we provide a token retrieved from ScriptInterpreter::platformInterpreterContext()
   which is an AIR-specific extension to ScriptInterpreter. This token is essentially associated with the root HTML
   in the HTMLLoader, and is used to establish the AS3 security context.

   The platformInterpreterContext maps (via the WebHost) to the HTMLLoader ActionScript object, but this detail could
   change in a future release.  In subframes, the platformInterpreterContext is null unless the subframe has scripting
   access (Window::isSafeScript)the top frame, in which case it also uses the top frame's platformInterpreterContext.

   On the ActionScript side, the platformInterpreterContext is used to determine which "code context" to use.
   If non-null, then it will use a code context corresponding to the page's (top frame's) URL.  If null, it will use
   a code context corresponding to the untrusted "about:blank" URL.
   
   Ideally, we'd have a per-frame platformInterpreterContext and it would map to an ActionScript CodeContext.  This would
   require us to maintain an AVM GC object reference (or equivalent) on the JavaScript side, and it would require
   us to have a per-frame place to store it.  Currently (10/23/07), the WebHostImplForFrames is shared among all
   subframes, and we'd probably want to maintain an instance per subframe.

    Least-privilege security model:
	We have NOT implemented a least-privilege security model, which would examine the call stack and grant 
	privilege only commeasurate with the least privileged call frame. In the end, we decided that in a dynamic
	environment like that of JavaScript, just checking the call stack at the time of a privileged operation
	is insufficient: unprivileged code could modify state (e.g. a variable containing a path to save to) that 
	is later used by privileged code without this being detected. We decided that having code of multiple privilege
	levels run through the same dynamic object model is inherently unsafe. A practical implementation problem with
	implementing least privilege is that we would literally need to be able to walk the JavaScriptCore call stack -
	we couldn't just push/pop frames at bridging boundaries, as it would not catch calls between JS frames, which
	are not chokepointed in any way. I'm not sure if the JavaScriptCore stack can be walked reliably enough to
	use it for security purposes - e.g. do calls that bounce through C++ code retain the stack? Should C++ code
	have its own frame on the stack for security purposes (would matter if unpriv C++ were somehow setup to be called
	directly from priv code and could do something nefarious)? Etc.
*/


/*
DESIGN NOTE - discussion about prototype bridging.

When doing first class bridging between JavaScript, ActionScript, or any similar scripting environments it is necessary to 
create the appearance of a single, unified prototype chain that spans scripting engines. In this scenario it is possible for 
a JavaScript object to have an ActionScript object as its prototype (by way of a JS proxy object) and vice versa. The "virtual" 
prototype chain for a given object can even span scripting engines multiple times - for example a JS object could have an AS 
object as its (virtual) prototype that in turn has a JS object as its (virtual) prototype. Note that the proxy objects (e.g. 
the JS object that stands in for an AS object and forwards to it) themselves are merely implementation details - if they have 
their own local-side prototypes, these should be completely invisible to a script developer - the effective prototype needs to 
be the prototype (if any) specified by the peer object living in the other script engine (the AS object in our example).

In order to implement a "virtual" prototype chain it is necessary to examine each point at which the involved scripting 
engines get or set object prototypes, and make sure that these operations work correctly when the object being operated upon 
is a local proxies for a peer object in another engine. In this case, it is typically the prototype of the peer object that 
needs to be operated upon, not the local-side proxy object's prototype (if any). It worth noting that in Webkit it is possible 
to change an object's prototype after construction (via the __proto__ extension) but this is not possible in ActionScript.


Current Webkit status (as of 9/3/2007 Apollo codebase)
------------------------------------------------------
Our proxy objects never have local-side prototypes. We don't give them prototypes upon construction, and __proto__ is not 
handled on the local side so they cannot get prototypes after the fact. JSObject::prototype(), access to JSObject::_proto, 
and JSObject::setPrototype() are not virtualized, so there are cases in which Webkit may get or set the prototype for a proxy 
object on the JS side that we do not catch. Note that most uses of these accessors/mutators in the codebase DO NOT operate 
upon a proxy object. Here are some notable cases (probably should be refeshed when we go to fix this) that can operate on a 
proxy object and are OK (and why):

  - JSObject::mark() - calls prototype() to get proto in order to mark it. OK cuz proxy objects don't have protos on the Webkit 
      side, and even if they did, marking is innocuous.

  - JSObject::put() - walks up prototype chain to see if there is an explicit setter for the property in question, and to call 
      it if there is one. Also calls setPrototype() to implement handling of setting the __proto__ meta-property. Both of these 
	  cases are OK because we override put() completely for our proxy objects. Note that __proto__ is not considered special on 
	  the AS3 side, so it merely would get set as a normal property, and not manipulate the proto chain.

  - JSObject::getPropertyNames() - looks up prototype so it can recursively call getPropertyNames() on prototype in order to add 
      prototype property names. This is irrelevant for our proxy objects because we override getPropertyNames() and forward the 
	  request to the associated AS object.

  - JSObject::getPropertySlot() [both versions of this func] - walks up prototype chain calling getOwnPropertySlot() until it 
      returns true. This is OK because when getOwnPropertySlot() is called on a proxy object it forwards the request to the 
	  associated AS object which is expected to continue the traversal of the prototype chain on itself (possibly forwarding it 
	  back to the webkit side if needed). If the property does not exist we eventually return false from getOwnPropertySlot(), 
	  and then getPropertySlot() tries to walk to the proxy object's (local) prototype, but since it doesn't have one, the 
	  property is correctly flagged as not found. We'd have a problem if there was a local prototype for the proxy object, as 
	  this could lead a property to be incorrectly found if it were in that prototype. 

  - JSObject::defaultValue() - looks at prototype for object so it can give special handling to date objects. This should not 
      be an issue for us - an AS3 date object will be represented by a proxy object, and we override defaultValue() for proxy 
	  objects, so if AS3 wants to give special handling for its date objects, it will have that chance.

And some cases that could pose a problem:

  - Interpreter::initGlobalObject() - walks global obj's proto chain to end, then links end to Object proto. PROBLEM: If a 
      proxy object were used as the global object or was in the global object's proto chain, this would not work correctly.
 
  - ObjectProtoFunc::callAsFunction() [LookupGetter, LookupSetter cases] - walks "this" object's proto chain looking for a 
      getter or setter. If "this" object is a proxy obj (possible using call or apply) or there is a proxy object in the proto 
	  chain, prototype walking will stop there rather than jumping to the AS side (which could later jump back). This means that 
	  setter/getter lookup could not find one when the correct answer per the virtual prototype chain would be to find one. 

  - ObjectProtoFunc::callAsFunction() [IsPrototypeOf case] - walks prototype chain of argument comparing each object to "this" 
      object. PROBLEM: if argument object is a proxy object or the (virtual) prototype chain contain any proxy objects, 
	  prototype walking will stop there rather than jumping to the AS side (which could later jump back). This means that 
	  isPrototypeOf could return false when the correct answer per the virtual prototype chain would have been true.

  - JSObject::hasInstance() - this method is used to implement the instanceof operator. hasInstance() is virtual, so we can 
      override it and have our version called when a proxy object is on the RHS of an instanceof operator. This doesn't do us 
	  much good - we really want to get involved when a proxy object is on the LHS of an instanceof operator, because this is 
	  when we need to walk its virtual proto chain. Worse, a proxy object could appear anywhere in the proto chain, so just 
	  catching it when a proxy object is on the LHS is not sufficient. PROBLEM: if the LHS of instanceof is a proxy object or 
	  contains one in its prototype chain, prototype walking will stop there rather than jumping to the AS side (which could 
	  later jump back). This means that instanceof could return false when the correct answer per the virtual prototype chain 
	  would have been true.


Current Tamarin status (as of 9/3/2007 Apollo codebase)
-------------------------------------------------------
Our proxy objects DO have local-side prototypes (they get Object by default). We don't give them prototypes upon construction, 
and __proto__ is not handled on the local side so they cannot change prototypes after construction. Prototype access is not 
virtualized, so as with webkit we must examine uses of prototypes (known internally as delegates). Here are the results of a 
sweep (probably should be refeshed when we go to fix this) where things are OK:

  - MethodEnv::findWithProperty - seems OK as long as a proxy object not on scope chain
  - MethodEnv::findproperty - seems OK as long as a proxy object is not the global object
  - ScriptObject::getAtomProperty - overridden by proxy and bridged - walks proto chain which is correct
  - ScriptObject::getAtomPropertyFromProtoChain - not called from anywhere where it will operate against one of our objects
  - ScriptObject::getUintProperty - overridden by proxy and bridged - walks proto chain which is correct
  - Toplevel::hasproperty - will not work correctly if run against one of our proxy objects, but never expect to run against 
      one of our proxy objects so OK [Chris wrote this function] 
  - Toplevel::getproperty - works correctly against one of our proxy objects

And some cases that could pose a problem:

  - MethodEnv::hasnext2 - this is mostly bridged, but at end of enumeration it will walk up delegate chain and hit local-side 
      Object, thus showing properties that perhaps should not be visible (though correct fix may be to get rid of local side 
	  Object prototype, or decide that the behavior is correct).
  - MethodEnv::in - this is bridged, but will walk up to local-side Object since it is the prototype. Correct fix may be to 
      get rid of local side Object prototype, or decide that the behavior is correct.
  - Toplevel::instanceof - walks proto chain on local side only, clearly will not work properly if we've got a virtual 
      prototype chain.
  - ObjectClass::objectIsPrototypeOf - walks proto chain on local side only, clearly will not work properly if we've got a 
      virtual prototype chain

For now, we've decided to live with the limitations described above (filed as bug 1607826). Here are some ideas for fixing if needed:

  - Virtualize setting and getting prototype from an object. This means that code that walks the prototype chain itself need 
    not change, and we merely override looking up the prototype and setting the prototype on proxy objects and make them walk 
	the virtual chain. Drawback is that some of this code may be in tight loops and introducing virtual function calls may be 
	a perf hit. When discussed with Geoff Garen from Apple, this was a major concern, specifically with respect to the property 
	lookup loop. A variant of this approach virtualizes the code that walks the prototype chains, but doesn't necessarily 
	virtualize getting a prototype from an object. For example, a traverser pattern might be used, doing a callback for each 
	prototype in the chain.

  - At construction-time, clone virtual prototype chain for an object so that the local-side prototype chain for the object 
    reflects the virtual prototype chain. For example, if a proxy is created on the Javascript side (references an AS3 object), 
	instead of having it have no prototype at all on the javascript side, we get the prototype of the AS3 object, put a 
	javascript wrapper on it (unless already a JS wrapper) and make it the prototype of the new proxy on the javascript side. 
	Recursively, this process creates a full cloned prototype chain. The one issue here is that in JavaScript, the prototype 
	for an object can be changed via the __proto__ property. This means that if the prototype for a JS object that was part of 
	a prototype chain for an AS3 proxy object was changed, we would need to update the peer AS3 objects so the proto chain on 
	the AS3 side was changed as appropriate. There are a few problems with this: 
	  1) we would need to be able to hook the prototype change for ANY JS object and notify its AS peer as needed, 
	  2) we would either need to be able to change the prototype of an AS3 object or reconstruct the AS3 proxy objects as needed 
	    (NOTE - reconstruction would be hard - need to find anybody who points at the object and update them too), 
      3) any code in our bridge that assumed that recursive proto chain walking occurs (e.g. getproperty) would need to be 
	    changed to only go one level deep, as we'd now be depending on the proto walking loops in the originating engine.
*/

/*
 olaru - 2011/05/03
 
 A copy of DynamicGlobalObjectScope, as originally defined in JSGlobalObject.h
 We use this implementation of the class as a convenient way to set and reset the
 globalData dynamicGlobalObject.
 
 We used to rely on the original JSC implementation, added in chageset 39377, but this was modified
 in changeset 50590 to only set the dynamicGlobalObject when the current one was null (upon
 first entry into JSC). However, proxy object code relied on this class setting the 
 dynamicGlobalObject regardless of the existing one, so this change broke the bridge behavior.
 
 In JSC, before this change, the client would check if a dynamicGlobalObject was already set, and 
 if so, pass in the same object. This means that the dynamicGlobalObject did not actually change.
 So from the perspective of JSC, the class serves the same purpose as before, but now incorporates
 the client null check. 
 
 The proxy object implementation needs to set the dynamicGlobalObject to provide correct context
 for cross frame calls using the sandbox bridge mechanism. If a function thax uses application
 sandbox APIs is exposed on the parentSandboxBridge, a call to that function from the child frame
 will need to set the context to parent frame to be allowed access to application sandbox APIs
 used in the function.
 This is done by setting the dynamicGlobalObject of the executing code, which started execution
 in the child frame, to the global object of the parent.
 
 See also
 http://trac.webkit.org/changeset/39377/trunk/JavaScriptCore/runtime/JSGlobalObject.h
 http://trac.webkit.org/changeset/50590/trunk/JavaScriptCore/runtime/JSGlobalObject.h
 */
namespace JSC {
    class DynamicGlobalObjectScopeApollo : public Noncopyable {
    public:
        DynamicGlobalObjectScopeApollo(CallFrame* callFrame, JSGlobalObject* dynamicGlobalObject) 
        : m_dynamicGlobalObjectSlot(callFrame->globalData().dynamicGlobalObject)
        , m_savedDynamicGlobalObject(m_dynamicGlobalObjectSlot)
        {
            m_dynamicGlobalObjectSlot = dynamicGlobalObject;
        }

        ~DynamicGlobalObjectScopeApollo()
        {
            m_dynamicGlobalObjectSlot = m_savedDynamicGlobalObject;
        }
        
    private:
        JSGlobalObject*& m_dynamicGlobalObjectSlot;
        JSGlobalObject* m_savedDynamicGlobalObject;
    };
}

namespace WebCore
{

namespace ApolloScriptBridging
{
    namespace VP
    {
        template <typename t_VariantPtr>
        class VariantPtr
        {
        public:
            ALWAYS_INLINE VariantPtr() throw() : m_ptr(0) {}
            
            ALWAYS_INLINE VariantPtr(t_VariantPtr ptr) throw()
                : m_ptr(ptr)
            {
                if (m_ptr)
                        m_ptr->m_pVTable->acquire(m_ptr);
            }

            template <typename t_VariantPtrOther>
            ALWAYS_INLINE VariantPtr(const VariantPtr<t_VariantPtrOther>& other) throw()
                : m_ptr(other.get())
            {
                if (m_ptr)
                    m_ptr->m_pVTable->acquire(m_ptr);     
            }

            ALWAYS_INLINE VariantPtr(const VariantPtr& other)
                : m_ptr(other.get())
            {
                if (m_ptr)
                    m_ptr->m_pVTable->acquire(m_ptr);
            }

            template <typename t_VariantPtrOther>
            ALWAYS_INLINE VariantPtr& operator=(const VariantPtr<t_VariantPtrOther>& other) throw()
            {
                if (m_ptr)
                    m_ptr->m_pVTable->release(m_ptr);
                m_ptr = other.get();
                if (m_ptr)
                    m_ptr->m_pVTable->acquire(m_ptr);
                return *this;
            }

            ALWAYS_INLINE VariantPtr& operator=(const VariantPtr& other) throw()
            {
                if (m_ptr)
                    m_ptr->m_pVTable->release(m_ptr);
                m_ptr = other.get();
                if (m_ptr)
                    m_ptr->m_pVTable->acquire(m_ptr);
                return *this;
            }

            static ALWAYS_INLINE VariantPtr adoptRef(t_VariantPtr ptr) throw()
            {
                ASSERT(ptr);
                return VariantPtr(ptr, true);
            }

            ALWAYS_INLINE t_VariantPtr release() throw()
            {
                ASSERT(m_ptr);
                t_VariantPtr const ptr = m_ptr;
                m_ptr = 0;
                return ptr;
            }

            ALWAYS_INLINE ~VariantPtr() throw()
            {
                if (m_ptr)
                    m_ptr->m_pVTable->release(m_ptr);
            }

            ALWAYS_INLINE t_VariantPtr get() const throw() { return m_ptr; }

            ALWAYS_INLINE bool operator!() const throw() { return !m_ptr; }
            
            ALWAYS_INLINE t_VariantPtr operator->() const throw() { return m_ptr; }

            // This conversion operator allows implicit conversion to bool but not to other integer types.
            typedef t_VariantPtr VariantPtr::*UnspecifiedBoolType;
            operator UnspecifiedBoolType() const { return m_ptr ? &VariantPtr::m_ptr : 0; }
        private:
            // adopting constructor
            ALWAYS_INLINE VariantPtr(t_VariantPtr ptr, bool) throw() : m_ptr(ptr) {}

            t_VariantPtr m_ptr;
        };
    }

    typedef VP::VariantPtr<WebScriptProxyVariant*> VariantPtr;
    typedef VP::VariantPtr<const WebScriptProxyVariant*> ConstVariantPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Wrapper obj that inits and releases contents of WebScriptProxyInvokeGetSetResult
class WebScriptProxyInvokeGetSetResultWrapper {
public:

    WebScriptProxyInvokeGetSetResultWrapper()
    {
        m_result.type = WebScriptProxyVariantInvokeGetSetResultSuccess;
        m_result.value = 0;
        m_result.exception = 0;
    }

    ~WebScriptProxyInvokeGetSetResultWrapper()
    {
        if (m_result.value)
            m_result.value->m_pVTable->release(m_result.value);
        if (m_result.exception)
            m_result.exception->m_pVTable->release(m_result.exception);
    }

    WebScriptProxyInvokeGetSetResult    m_result;

private:

    // intentionally private and unimplemented so they cannot be used
    WebScriptProxyInvokeGetSetResultWrapper& operator=(const WebScriptProxyInvokeGetSetResultWrapper&);    // operator=
    WebScriptProxyInvokeGetSetResultWrapper(const WebScriptProxyInvokeGetSetResultWrapper&);               // copy constructor
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* Proxies JavaScript access to another object system (e.g. ActionScript) by wrapping a strong referenced pointer
   to a variant and making it available to JavaScriptCore as a JSObject. */ 

/*
mihnea integrate: do we need to implement JSObject:: getPropertyDescriptor/getOwnPropertyDescriptor/getOwnPropertyNames
*/

class ProxyObjectImp : public JSC::JSObject {
public:

    const JSC::ClassInfo* classInfo() const;

    // from JSObject
    virtual bool getOwnPropertySlot(JSC::ExecState* exec, const JSC::Identifier& propertyName, JSC::PropertySlot& slot);
    virtual void put(JSC::ExecState* exec, const JSC::Identifier& propertyName, JSC::JSValue value, JSC::PutPropertySlot& slot);
    virtual void putWithAttributes(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::JSValue value, unsigned attributes);
    virtual bool deleteProperty(JSC::ExecState* exec, const JSC::Identifier& propertyName);
    virtual JSC::JSValue defaultValue(JSC::ExecState* exec, JSC::PreferredPrimitiveType hint) const;
    virtual void getPropertyNames(JSC::ExecState* exec, JSC::PropertyNameArray& propertyNames, JSC::EnumerationMode mode = JSC::ExcludeDontEnumProperties);

    // from JSCell (JSObject subclasses JSCell)
    virtual JSC::CallType getCallData(JSC::CallData&);
    virtual JSC::ConstructType getConstructData(JSC::ConstructData&);

    static JSC::JSValue JSC_HOST_CALL callAsFunction(JSC::ExecState* exec, JSC::JSObject* func, JSC::JSValue thisValue, const JSC::ArgList& args);
    
#if ENABLE(APOLLO_PROFILER)
	// profiler related functions, added in JSObject under PLATFORM(APOLLO)
    virtual void getMembersForProfiler(JSC::ProfilerMembersList*) { }
	virtual JSC::UString typeName();
#endif

    static JSC::JSObject* construct(JSC::ExecState* exec, JSC::JSObject* constructor, const JSC::ArgList& args);

    // Returns true if passed JSValue is a pointer to a JSProxyObject, else false.
    static bool isJSValueProxyObject(JSC::JSValue value);

    // Returns NULL on failure. Illegal to pass NULL or a local-side variant or a non-boject-reference variant
    static JSC::JSObject* GetJSObjectProxyForRemoteVariantCreateIfNeeded(JSC::ExecState* exec, const struct WebScriptProxyVariant* pVariant);

    // Returns pointer to variant wrapped by proxy obj
    inline VariantPtr getVariant() const
    {
        return m_pObjRefVariant;
    }

private:
    // Do not call ctor directly, call GetJSObjectProxyForRemoteVariantCreateIfNeeded() instead
    ProxyObjectImp(const VariantPtr& pObjRefVariant, PassRefPtr<JSC::Structure> structure);
    ~ProxyObjectImp();
    
    // Throws JSValue as exception in exec, and returns same error object. Error object is based on contents of pResult,
    // which must represent an error condition. If exception object present in pResult, that is thrown, else an error
    // object is concocted.
    // NULL for params not allowed.
    // Never fails (crashes internally instead)
    static JSC::JSValue throwJSExceptionForInvokeGetSetResultWrapper(JSC::ExecState* exec, const WebScriptProxyInvokeGetSetResult* pResult);

    // Throws "internal error" JSValue as exception in exec, and returns same error object
    // exec arg may not be NULL
    // Never fails (crashes internally instead)
    static JSC::JSObject* throwInternalErrorJSException(JSC::ExecState* exec);


    // Callback used to implement getOwnPropertySlot()
    static JSC::JSValue propGetter(JSC::ExecState* exec, JSC::JSValue slotBase, const JSC::Identifier& propertyName);

    

    // Map that goes from Variant -> ProxyObjectImp to ensure uniqueness of ProxyObjectImps
    // Map does NOT hold strong references on the ProxyObjectImps or variants it contains, nor is it part of the GC for JSValues it contains
    // ~ProxyObjectImp removes from map - this makes sure map doesn't hold any bogus ProxyObjectImps or variants
    typedef HashMap< const struct WebScriptProxyVariant*, ProxyObjectImp* > t_VariantToProxyJSObjectMap; 
    static t_VariantToProxyJSObjectMap* m_pVariantToProxyJSObjectMap;

    // Gets static map between Variant and ProxyObjectImp. Returns NULL on failure.
    static t_VariantToProxyJSObjectMap* getVariantToProxyJSObjectMap();

    // Removes "this" variant from the Variant -> ProxyObjectImp map
    void removeSelfFromVariantToProxyJSObjectMap() const;

    bool implementsConstruct();

    // ClassInfo used by classInfo() method
    static const JSC::ClassInfo  m_classInfo;
    static const JSC::ClassInfo  m_classInfoForArray;

    // Smart pointer to the variant we wrap (holds a ref on the variant)
    VariantPtr              m_pObjRefVariant;

    // the computed type name
    JSC::UString    m_typeName;

    ProxyObjectImp(); // prevent default construction
    ProxyObjectImp(const ProxyObjectImp& other, PassRefPtr<JSC::Structure> structure); // prevent copying
    ProxyObjectImp& operator=(const ProxyObjectImp& other); // ditto
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
    Implementation of variant class used in script bridging. This implementation wraps a JavaScriptCore JSValue. 
    Interface comments may be found in ApolloWebKitScriptProxy.h

    Note that this is NOT a garbage collected object. It is kept alive by its refcount.

    DESIGN NOTE:
       There are two obvious approaches to implementing the variant:
       1. Have the variant store a union that contains boolean, number, string, object reference types and convert
          into these representations here
       2. Have the variant simply store the JSValue and hold a strong reference on it, converting to the various
          types (e.g. boolean, number, etc.) upon request.

      I have chosen to implement #2 because it is simpler. From a performance standpoint it means any necessary type 
      conversions will be done on each request for a value (unless we put a cache in the variant). This is not likely
      to be a perf issue though. 


    DESIGN NOTE - dead variants
      The current code throws away the JSValue when the variant becomes dead. This allows it to be garbage
      collected, which is nice. For primitives (boolean, number, etc.) it is not strictly necessary because 
      they are self-contained. We could retain the JSValue and continue to operate upon it. However, this 
      would be more fragile as there is no promise that these types will always be self contained. Likewise,
      we currently cache the type of the JSValue so that we can continue to respond to getType() from a 
      dead variant. The type() method of a JSValue does not require an interpreter to be passed, so we could
      conceivably retain the JSValue and still call its type method. However, this is potentially fragile 
      and requires us to retain a reference on the JSValue that will be prevent it from being GCed. See also
      comments with isDeadVariant() method.
*/
class JSCoreVariant : public WebKitApollo::WebScriptProxyVariantHelper< JSCoreVariant > {
public:

    void acquire() const { m_refCount++; }
    void release() const { ASSERT(m_refCount > 0); m_refCount--; if (m_refCount == 0) delete this; }
    WebScriptProxyVariantType getType() const;
    bool getBool() const;
    double getNumber() const;
    unsigned long getStringUTF16Length() const;
    void getStringUTF16(uint16_t* pDest) const;
    bool hasProperty(const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength, void* interpreterContext) const;
    void getProperty(const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength, void* interpreterContext, WebScriptProxyInvokeGetSetResult* pPropertyGetResult) const;
    void setProperty(const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength, ::WebScriptProxyVariant* pValue, void* interpreterContext, WebScriptProxyInvokeGetSetResult* pPropertySetResult);
    bool deleteProperty(const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength, void* interpreterContext, ::WebScriptProxyVariant** ppDeleteException);
    bool canInvoke() const;
    void invoke(::WebScriptProxyVariant* pThisVariant, unsigned long numArguments, ::WebScriptProxyVariant* const * argumentArray, void* interpreterContext, struct WebScriptProxyInvokeGetSetResult* pInvokeResult) const;
    void enumProperties(   WebScriptProxyVariantEnumPropertiesProcFunction enumProc,
                            WebScriptProxyVariantPropertiesEnumData* pClientData,
							void* interpreterContext) const;
    bool canConstruct() const;
    void construct(unsigned long numArguments,
                    ::WebScriptProxyVariant * const * argumentArray,
					void* interpreterContext,
                    WebScriptProxyInvokeGetSetResult* pConstructResult) const;
    void defaultValue(WebScriptProxyInvokeGetSetResult* pDefaultValueResult,
                        WebScriptProxyVariantType typeHint,
						void* interpreterContext) const;
    WebScriptProxyVariantObjectType getObjectType() const;

    // Must be called when an Interpreter is destroyed. Will not call through the Interpreter pointer
    static void notifyGlobalObjectDead(JSC::JSGlobalObject* pJSGlobalObject);

    // Mark a variant as dead 
    static void markAsDeadIfNeeded(JSC::JSValue value, JSC::JSGlobalObject* pJSGlobalObject);

    // Returns whether the JSCoreVariant is "dead" meaning that it cannot be used anymore
    // We need the concept of a dead variant because we do not control the lifetime of "this" object - it is
    // controlled by the refcount. However, things the variant point at conceivably can go away before "this" object,
    // in which case we make the variant "dead" and it starts returning defaults or exceptions from all methods.
    // Also see DESIGN NOTE above JSCoreVariant class for more info.
    bool isDeadVariant() const { return (!m_pJSGlobalObject); }

    // Returns JSValue wrapped by variant. Do not call if isDeadVariant() returns false
    JSC::JSValue getValue() const { return m_pValue; }

    // Returns smartptr to JSCoreVariant, reusing existing one if possible else creating new one. 
    // Illegal to pass NULL for value.
    // Illegal to pass NULL for exec. exec will be used to exec calls etc. on the variant
    // Returns NULL if failure
    static VariantPtr GetJSCoreVariantCreateIfNeeded(const JSC::ExecState* exec, JSC::JSValue value);
    
    void getObjectTypeName(uint16_t *pUTF16TypeName, unsigned long* pTypeNameLength) const;


private:

    // Key used in mapping from JSValue/JSGlobalObject -> JSCoreVariant
    //typedef std::pair<JSC::EncodedJSValue, JSC::JSGlobalObject*> t_JSValueToVariantKey;
    typedef std::pair<JSC::EncodedJSValue, JSC::JSGlobalObject*> t_JSValueToVariantKey;

    // Hash struct used in mapping from JSValue/JSGlobalObject -> JSCoreVariant
    struct JSValueToVariantKeyHash {
        static const bool safeToCompareToEmptyOrDeleted = false;

        static unsigned hash(const t_JSValueToVariantKey& key) 
        { 
            //return (PtrHash<JSC::EncodedJSValue>::hash(key.first) +  PtrHash<JSC::JSGlobalObject*>::hash(key.second));   // Use + to combine hash values
            return (JSC::EncodedJSValueHash::hash(key.first) +  PtrHash<JSC::JSGlobalObject*>::hash(key.second));
        }

        static bool equal(const t_JSValueToVariantKey& a, const t_JSValueToVariantKey& b) 
        { 
            return (a == b); 
        }

    };

    // Map that goes from JSValue/Interpreter -> JSCoreVariant to ensure uniqueness of JSCoreVariants
    // Map must not include any dead variants
    // Map does NOT hold strong references on the variants it contains, nor is it part of the GC for JSValues it contains
    // Items are removed from the map by ~JSCoreVariant and when a variant goes dead
	// See comment in GetJSCoreVariantCreateIfNeeded() to understand why Interpeter is part of the key
    typedef HashMap< t_JSValueToVariantKey, JSCoreVariant*, JSValueToVariantKeyHash> t_JSValueToVariantMap; 

    // Don't call this directly if GetJSCoreVariantCreateIfNeeded() can do what you want
    // Forwards to JSCoreVariant ctor
    // Illegal to pass NULL for value.
    // Illegal to pass NULL for pJSGlobalObject. pJSGlobalObject will be used to exec calls etc. on the variant
    // Returns NULL if failure
    static inline VariantPtr Construct(JSC::JSValue value, JSC::JSGlobalObject* pJSGlobalObject)
    {
        JSCoreVariant* const pVariant(new JSCoreVariant(value, pJSGlobalObject));
    
        // Refcount starts at 1, so we immediately wrap it in a VariantPtr to control lifetime
        return VariantPtr::adoptRef(pVariant->getApolloScriptProxyVariant()); // new must be in sync with delete in JSCoreVariant::release()
    }

    // Do not call ctor directly - use Construct static method instead
    // Refcount starts life at 0!
    JSCoreVariant(JSC::JSValue value, JSC::JSGlobalObject* pJSGlobalObject);
    // Private dtor means only refcounting mechanism can destroy
    ~JSCoreVariant();

    // Returns ExecState to be used for executing against the JSValue in the variant (e.g. for get/put/invoke)
    // Must not be called on a dead variant. Returned ExecState is owned by the Interpreter in the variant,
    // not by the caller. Be sure to clear any exception in the ExecState when done with it.
    JSC::ExecState* getExecState() const;

    // Gets static map between JSValue and JSCoreVariant. Returns NULL on failure.
    static t_JSValueToVariantMap* getValueToVariantMap();

    // Removes "this" variant from the JSValue -> JSCoreVariant map
    void removeSelfFromJSValueToVariantMap() const;

    // Given ExecState and JSValue, returns Interpreter to use for executing against the JSValue.
    // Interpreter is NOT owned by caller, no refcount is increased, etc.
    static JSC::JSGlobalObject* getJSDOMWindowForJSValue(const JSC::ExecState* exec, JSC::JSValue value);

    /* Completely fills out WebScriptProxyInvokeGetSetResult object based on exception info in the passed ExecState.
       The passed ExecState MUST have an exception in it. Neither param may be NULL.
       This method assumes that pResult is a blank slate and in particular will not call release through any pointers
       it may contain. Upon return, any non-NULL refcounted pointers in pResult will be owned by the struct and must be
       released at some point */
    static void fillErrorInfoFromException(const JSC::ExecState* pExecState, WebScriptProxyInvokeGetSetResult* pResult);

    /* Completely fills out WebScriptProxyInvokeGetSetResult object based on passed code, which must be one of the
       codes that indicates a generic error for which no exception is available.
       This method assumes that pResult is a blank slate and in particular will not call release through any pointers
       it may contain.
    */
    static void fillConstantErrorInfo(WebScriptProxyInvokeGetSetResult* pResult, WebScriptProxyVariantInvokeGetSetResultType error);


    /* Helper function for implementing invoke and get. None of the args may be NULL. Completely fills out 
       WebScriptProxyInvokeGetSetResult object.
       This method assumes that pResult is a blank slate and in particular will not call release through any pointers
       it may contain.
       If pExecState contains an exception the error info in pResult will be filled out and pResultJSValue will be ignored.
       In this case the exception will be cleared as well.
    */
    static void handleGetOrCallResult(JSC::ExecState* pExecState, JSC::JSValue pResultJSValue, struct WebScriptProxyInvokeGetSetResult* pResult);
    
    /* Our refcount */
    mutable unsigned long                   m_refCount;

    /* We keep a strong reference on the JSValue so that it cannot be GCed. It is fine to use one of these
       pointers even if the JSValue is an immediate (e.g. boolean) which doesn't ever get GCed - in this
       case there is no perf hit or anything. */
    JSC::ProtectedJSValue         m_pValue;       

    /* Interpreter associated with the JSValue at the time we created the JSCoreVariant. We must go dead
	   when this Interpreter is destructed. */
    JSC::JSGlobalObject*                    m_pJSGlobalObject;

    /* Cache of type of variant. See DESIGN NOTE above JSCoreVariant class for details. */
    WebScriptProxyVariantType               m_CachedType;

    
    static t_JSValueToVariantMap*           m_pValueToVariantMap;


    // intentionally private and unimplemented so they cannot be used
    JSCoreVariant& operator=(const JSCoreVariant&);    // operator=
    JSCoreVariant(const JSCoreVariant&);               // copy constructor
};

//Forward declaration.
static VariantPtr convertJSValueToBridgingVariant(const JSC::ExecState* exec, JSC::JSValue value);

static inline JSC::JSType convertVariantTypeToJSType(const WebScriptProxyVariantType variantType)
{
    JSC::JSType retVal = JSC::UnspecifiedType;
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
    COMPILE_ASSERT((WebScriptProxyVariantTypeObjectRef + 1) == WebScriptProxyVariantTypeLast,
                        enum_changed_code_must_be_updated);
    COMPILE_ASSERT(6 == WebScriptProxyVariantTypeLast,
                        enum_changed_code_must_be_updated);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__
    switch (variantType) {
        case WebScriptProxyVariantTypeNumber:
            retVal = JSC::NumberType;
            break;
        case WebScriptProxyVariantTypeBoolean:
            retVal = JSC::BooleanType;
            break;
        case WebScriptProxyVariantTypeUndefined:
            retVal = JSC::UndefinedType;
            break;
        case WebScriptProxyVariantTypeNull:
            retVal = JSC::NullType;
            break;
        case WebScriptProxyVariantTypeString: 
            retVal = JSC::StringType;
            break;

        case WebScriptProxyVariantTypeObjectRef:
            retVal = JSC::ObjectType;
            break;
        default:
            ASSERT(false);
            retVal = JSC::UnspecifiedType;
            break;
    }
    return retVal;
}

static inline WebScriptProxyVariantType convertJSTypeToVariantType(JSC::PreferredPrimitiveType type, bool const unspecifiedTypeOk = false)
{
    WebScriptProxyVariantType retVal;

    //Following statement is needed to get rid of unused argument error in the release build.
    (void)unspecifiedTypeOk;

    switch (type) {
    
        case JSC::PreferNumber:  retVal = WebScriptProxyVariantTypeNumber;   break;
        case JSC::PreferString:  retVal = WebScriptProxyVariantTypeString;   break;

        case JSC::NoPreference:       // Used for specifying a desired type to methods such as defaultValue(), not a type you actually find a value to have 
            ASSERT(unspecifiedTypeOk);
            retVal = WebScriptProxyVariantTypeUndefined;
            break;
        default:
            ASSERT(false);    // uh oh! don't expect to get here...
            retVal = WebScriptProxyVariantTypeUndefined;
            break;
    }

    return retVal;
}
	
static inline WebScriptProxyVariantType convertJSTypeToVariantType(JSC::JSValue value, bool const unspecifiedTypeOk = false)
{
	ASSERT(value);	
	WebScriptProxyVariantType retVal = WebScriptProxyVariantTypeUndefined;
		
	//Following statement is needed to get rid of unused argument error in the release build.
	(void)unspecifiedTypeOk;
	
	if(value.isNumber())
		retVal = WebScriptProxyVariantTypeNumber;
	else if(value.isBoolean())
		retVal = WebScriptProxyVariantTypeBoolean;
	else if(value.isUndefined())
		retVal = WebScriptProxyVariantTypeUndefined;
	else if(value.isNull())
		retVal = WebScriptProxyVariantTypeNull;
	else if(value.isString())
		retVal = WebScriptProxyVariantTypeString;
	else if(value.isObject())
		retVal = WebScriptProxyVariantTypeObjectRef;
	else if(value.isGetterSetter()) {
		ASSERT(false);
		retVal = WebScriptProxyVariantTypeUndefined;
	}
	else {
		ASSERT(unspecifiedTypeOk);
	}
				
	return retVal;
}
	

// Creates new Identifier based on passed UTF16 data. pUTF16Data may not be NULL. OK for utf16DataLength to be 0.
static inline JSC::Identifier identifierFromUTF16Data(JSC::ExecState* exec, const uint16_t* pUTF16Data, unsigned long utf16DataLength)
{
    ASSERT(pUTF16Data);

    // Turn UTF16 string into Identifier
    // **PERF - this alloc/copy might be saved if hasProperty() could work with raw strings rather than requiring an Identifier
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
    COMPILE_ASSERT( (  (sizeof(UChar) == 2) &&
                            (sizeof(UChar) == sizeof(uint16_t))),
                        UChar_must_be_two_bytes_in_size); // Identifier needs to work in 16bit chars
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__

    // **PERF - each Identifier that is created becomes an item in a global table that is never freed. If tons of Identifiers
    // were generated this could be a perf issue or a denial of service issue. Note that this is not specific to 
    // bridging - appears to be a general WebKit issue.

    // This can almost certainly fail internally, but I believe it will crash inside if it happens  
    // Note that returning by value is a simple addref so cannot fail and is cheap
    return JSC::Identifier(exec, reinterpret_cast< const UChar* >(pUTF16Data), utf16DataLength);   
}

/*
    Given that this object proxies to a different object, is it appropriate to have a hardcoded
    ClassInfo object with a hardcoded classname? Should we be dynamically returning classinfo (and classname)
    corresponding to the object we're proxying for? This isn't really documented, but after an exhaustive
    search of usage and a look at precedent (RuntimeObjectImp does bridging for the plugin API) I have determined
    that hardcoding is correct. Here is what ClassInfo/ClassName are used for:

    1. RTTI in C++ code: downcasting from JSObject to a subclass based on comparing ClassInfo pointers. 
    2. Implementing toString (incorporating className) by asking for _own_ className
    3. ArayProtoFunc::callAsFunction() creates error string incorporating somebody else's className when
       there is no toString() method from which it can get a string from.
    4. JSObject implementation uses ClassInfo to find hashtables for non-dynamic properties.
    5. JSObject::inherits() uses ClassInfo to determine JSObject inheritance, generally used for validation
       or downcasting.

    For case #1, proxying a remote type makes no sense, as there is no local C++ class/object do downcast to
    For case #2, the code in question looks only at its own className so what we do is irrelevant
    For case #3, it would be nice to have the proxied className but hardly seems worthwhile given that it
      only occurs if toString() is not present.
    For case #4, we don't have any non-dynamic hashtables in our class, and all prop lookup is done on the
      remote side, so this isn't relevant
    Case #5 is very similar to case #1

	Note also that WebKit uses the same strategy we are using for its RuntimeObjectImp, which is analagous
	to ProxyObjectImp.
*/
const JSC::ClassInfo ProxyObjectImp::m_classInfo = { "ScriptBridgingProxyObject", 0, 0, 0 };
const JSC::ClassInfo ProxyObjectImp::m_classInfoForArray = { "ScriptBridgingArrayProxyObject", &JSC::ArrayPrototype::info, 0, 0 };

bool isJSValueProxyObject(JSC::JSValue value);
bool isJSValueProxyObject(JSC::JSValue value)
{
    return ProxyObjectImp::isJSValueProxyObject(value);
}

// Init static map ptr to NULL.
ProxyObjectImp::t_VariantToProxyJSObjectMap* ProxyObjectImp::m_pVariantToProxyJSObjectMap = 0;


ProxyObjectImp::ProxyObjectImp(const VariantPtr& pObjRefVariant, PassRefPtr<JSC::Structure> structure) : 
	JSC::JSObject(structure), m_pObjRefVariant(pObjRefVariant)
{
    ASSERT((pObjRefVariant) && (pObjRefVariant->m_pVTable->getType(pObjRefVariant.get()) == WebScriptProxyVariantTypeObjectRef));
    ASSERT(!JSCoreVariant::isSameImpl(pObjRefVariant.get()));
}


ProxyObjectImp::~ProxyObjectImp()
{
    // VariantPtr dtor takes care of releasing ref to variant

    // Pull out of map if we're in it
    removeSelfFromVariantToProxyJSObjectMap();
}

const JSC::ClassInfo* ProxyObjectImp::classInfo() const
{
    WebScriptProxyVariantObjectType objectType =
		m_pObjRefVariant->m_pVTable->getObjectType(m_pObjRefVariant.get());
    // So that arrays are recognized as such in, for instance, Array.concat parameters
    if (objectType == WebScriptProxyVariantObjectTypeArray)
        return &m_classInfoForArray;
    return &m_classInfo;
}


ProxyObjectImp::t_VariantToProxyJSObjectMap* ProxyObjectImp::getVariantToProxyJSObjectMap()
{
    // Create new map if we don't have one, stashing it in class-static variable
    if (!m_pVariantToProxyJSObjectMap)
        m_pVariantToProxyJSObjectMap = new t_VariantToProxyJSObjectMap;
    ASSERT(m_pVariantToProxyJSObjectMap);
    return m_pVariantToProxyJSObjectMap;
}


void ProxyObjectImp::removeSelfFromVariantToProxyJSObjectMap() const
{
    // Remove this object from the map  
    t_VariantToProxyJSObjectMap* pVariantToProxyJSObjectMap = getVariantToProxyJSObjectMap();   // rets NULL on failure
    ASSERT(pVariantToProxyJSObjectMap);   // should never happen cuz we shouldn't be called if we are not in the map
    if (pVariantToProxyJSObjectMap) { 
    
        // See if we're in the map. If this method has already been called, we won't be.
        t_VariantToProxyJSObjectMap::iterator iter = pVariantToProxyJSObjectMap->find(m_pObjRefVariant.get());
        if (iter != pVariantToProxyJSObjectMap->end()) 
            pVariantToProxyJSObjectMap->remove(iter);
    }
}

inline bool ProxyObjectImp::implementsConstruct()
{
    JSC::ConstructData cd;
    return getConstructData(cd) != JSC::ConstructTypeNone;
}

bool ProxyObjectImp::isJSValueProxyObject(JSC::JSValue value)
{
    ASSERT(value);
    return value.inherits(&m_classInfo) || value.inherits(&m_classInfoForArray) ;
}


JSC::JSValue ProxyObjectImp::propGetter(JSC::ExecState* exec, JSC::JSValue slotBase, const JSC::Identifier& propertyName)
{
    ASSERT(exec);
    
    // Note that 0-len propertyName is OK
	ProxyObjectImp* pThis = static_cast<ProxyObjectImp*>(JSC::asObject(slotBase));
    
    ASSERT(pThis);
    ASSERT((pThis->m_pObjRefVariant) &&
            (pThis->m_pObjRefVariant->m_pVTable->getType(pThis->m_pObjRefVariant.get()) == WebScriptProxyVariantTypeObjectRef));

    JSC::JSValue retJSValue;

    // Get string from propertyName. Note that we could likely get the same data directly out of propertyName
    // but I want to be sure we've got our own copy (current imp actually just does COW cheap copy) because
    // get could call back into us, and I want to make sure there's no risk of the bytes becoming invalid.
    // Identifier seems to be immutable now, but lets just be safe.
    //
    // We can pass the non-NULL terminated 16-bit string from UString::data() in as the prop name,
    // and its length is UString::size().
    JSC::UString propertyNameString(propertyName.ustring()); // this can fail but crashes internally if it does...
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
    COMPILE_ASSERT( (sizeof(UChar) == 2) &&
                        (sizeof(UChar) == sizeof(uint16_t)),
                        UChar_must_be_two_bytes_in_size);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__
    const uint16_t* const pUTF16PropName =
        reinterpret_cast< const uint16_t* >(propertyNameString.data());

    // Perform the get, then see if it succeeded.
    // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
    WebScriptProxyInvokeGetSetResultWrapper getResult;   // wrapper obj inits and releases contents of result obj
    retJSValue = JSC::jsUndefined();     // cannot fail
    pThis->m_pObjRefVariant->m_pVTable->getProperty(pThis->m_pObjRefVariant.get(),
                                                             pUTF16PropName,
                                                             propertyNameString.size(),
															 exec->dynamicGlobalObject()->platformInterpreterContext(),
                                                             &getResult.m_result);
    if (!webScriptProxyGetSetResultIsSuccess(getResult.m_result.type)) {

        // Error doing the get
        ASSERT(!getResult.m_result.value);

        // Per Geoff Garen (ggaren@apple.com) at Apple, even though no prop getters currently throw, there is no reason
        // that this is not allowed. As such, we'll throw the exception back to JavascriptCore.
        retJSValue = throwJSExceptionForInvokeGetSetResultWrapper(exec, &getResult.m_result);
    }
    else {

        // Successful get
        ASSERT(getResult.m_result.value);
        ASSERT(!getResult.m_result.exception);

        // Convert bridging variant we got from the get operation to a JSValue. Returns NULL on failure.
        retJSValue = jsValueFromBridgingVariant(exec, getResult.m_result.value);
        if (!retJSValue) {

            // As above, throw exception back to JavascriptCore
            retJSValue = throwInternalErrorJSException(exec);
        }
    }

    ASSERT(retJSValue);	
    return retJSValue; 
}


/* Called by property get mechanism and hasProperty mechanism */
bool ProxyObjectImp::getOwnPropertySlot(JSC::ExecState* exec, const JSC::Identifier& propertyName, JSC::PropertySlot& slot)
{
    ASSERT(exec);
    //The following line is needed to get rid of an unused parameter warning in the release build.
    //assert above uses parameter in debug build.
    (void)exec;
    // Note that 0-len propertyName is OK

    bool retVal = false;

    // Get string from propertyName. Note that we could likely get the same data directly out of propertyName
    // but I want to be sure we've got our own copy (current imp actually just does COW cheap copy) because
    // hasProperty could call back into us, and I want to make sure there's no risk of the bytes becoming invalid.
    // Identifier seems to be immutable now, but lets just be safe.
    //
    // We can pass the non-NULL terminated 16-bit string from UString::data() in as the prop name,
    // and its length is UString::size().
    JSC::UString propertyNameString(propertyName.ustring());       // this can fail but crashes internally if it does...
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
    COMPILE_ASSERT( (sizeof(UChar) == 2) &&
                        (sizeof(UChar) == sizeof(uint16_t)),
                        UChar_must_be_two_bytes_in_size);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__

    const uint16_t* const pUTF16PropName =
        reinterpret_cast< const uint16_t* >(propertyNameString.data());

    /* We need to ask if the remote object has the property so we can return false if it does not. This is
       necessary because JSObject::hasProperty() is implemented in terms of this method. */
    // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
    if (m_pObjRefVariant->m_pVTable->hasProperty(  m_pObjRefVariant.get(),
                                                                pUTF16PropName,
                                                                propertyNameString.size(),
																exec->dynamicGlobalObject()->platformInterpreterContext() )) 
    {

        /* Setup so that get mechanism will call our propGetter() method to actually retrieve the property */
        slot.setCustom(this, propGetter);

        retVal = true;
    }

    /* Note: if variant doesn't have property, we don't call the superclass or try to walk a proto chain
       or anything - that is all done on the remote side. Returning false does cause that to happen on the local
	   side (inside GetPropertySlot) however because our proxy objects have no prototype this doesn't cause any harm. */
	ASSERT( prototype() == JSC::jsNull() );

    // don't call superclass, because runtime objects can't have custom properties or a prototype
    return retVal;
}


void ProxyObjectImp::put(JSC::ExecState* exec, const JSC::Identifier& propertyName, JSC::JSValue value, JSC::PutPropertySlot& /*slot*/)
{
    ASSERT((exec) && (value));
    // Note that 0-len propertyName is OK

    

    /* DESIGN NOTE: why aren't we calling canPut() here and making this a NOP if it returns false? After
       all, this is what the default put() implementation does, and is correct per ECMA-262 section 8.6.2.2.
       The answer is that it is up to the remote side to do this if they want - we do not want to enforce 
       local-side rules in this matter. The local side can choose to call canPut() before calling put() or not,
       both are fine. The remote side will behave as per its own rules when put() is called without canPut()
       being called. */

    // Note that the default JSObject::put() implementation special-cases sets of __proto__ to change
    // the prototype (and has similar code in get()). It isn't appropriate to do such remapping here -
    // if the remote side wants to give special handling to __proto__ it is up to that code to do it.

    // Get bridging variant that corresponds to the passed JSValue. Returns NULL on failure.
    VariantPtr pVariantArg = convertJSValueToBridgingVariant(exec, value);
    if (!pVariantArg) {

        // Failure converting JSValue to variant. Throw JS exception.
        throwInternalErrorJSException(exec);
    }
    else {

        // Get string from propertyName. Note that we could likely get the same data directly out of propertyName
        // but I want to be sure we've got our own copy (current imp actually just does COW cheap copy) because
        // put could call back into us, and I want to make sure there's no risk of the bytes becoming invalid.
        // Identifier seems to be immutable now, but lets just be safe.
        //
        // We can pass the non-NULL terminated 16-bit string from UString::data() in as the prop name,
        // and its length is UString::size().
        JSC::UString propertyNameString(propertyName.ustring()); // this can fail but crashes internally if it does...
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
        COMPILE_ASSERT( (sizeof(UChar) == 2) &&
                            (sizeof(UChar) == sizeof(uint16_t)),
                            UChar_must_be_two_bytes_in_size);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__

        const uint16_t* const pUTF16PropName =
            reinterpret_cast< const uint16_t* >(propertyNameString.data());

        // Perform the put, then see if it succeeded.
        // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
        WebScriptProxyInvokeGetSetResultWrapper putResult;   // wrapper obj inits and releases contents of result obj
        m_pObjRefVariant->m_pVTable->setProperty(   
            m_pObjRefVariant.get(),
            pUTF16PropName, 
            propertyNameString.size(),
            pVariantArg.get(),
            exec->dynamicGlobalObject()->platformInterpreterContext(),
            &putResult.m_result);
        
        if (!webScriptProxyGetSetResultIsSuccess(putResult.m_result.type)) {

            // Error on put. Throw exception we got back into JS.
            throwJSExceptionForInvokeGetSetResultWrapper(exec, &putResult.m_result);
        }
        ASSERT(!putResult.m_result.value); // should always be NULL for a put
    }
}

void ProxyObjectImp::putWithAttributes(JSC::ExecState* exec, const JSC::Identifier& propertyName, JSC::JSValue propertyValue, unsigned)
{
	if (exec->hadException())
		// return early if a previous call has thrown an exception
		return;
	
	// This is used to add line numbers on errors thrown from AS3
	// check Interpreter::updateExceptionStackTraceIfPossible for details.
	// Sometimes the user might throw a sealed AS3 class. Because of that
	// the following put will fail with exception and the caller code is not
	// designed to catch exceptions from putWithAttributes.
	JSC::PutPropertySlot slot;
    put(exec, propertyName, propertyValue, slot);
	
	if (exec->hadException())
		// don't leak the exceptions to JS
		exec->clearException();
}

bool ProxyObjectImp::deleteProperty(JSC::ExecState* exec, const JSC::Identifier& propertyName)
{
    ASSERT(exec);
    //The following line is needed to get rid of an unused parameter warning in the release build.
    //assert above uses parameter in debug build.
    (void)exec;
    // Note that 0-len propertyName is OK

    // Get string from propertyName. Note that we could likely get the same data directly out of propertyName
    // but I want to be sure we've got our own copy (current imp actually just does COW cheap copy) just in case
    // deleteProperty calls back into us, in which case I want to make sure there's no risk of the bytes becoming invalid.
    // Identifier seems to be immutable now, but lets just be safe.
    //
    // We can pass the non-NULL terminated 16-bit string from UString::data() in as the prop name,
    // and its length is UString::size().
    JSC::UString propertyNameString(propertyName.ustring()); // this can fail but crashes internally if it does...
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
    COMPILE_ASSERT( (sizeof(UChar) == 2) &&
                        (sizeof(UChar) == sizeof(uint16_t)),
                        UChar_must_be_two_bytes_in_size);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__
    const uint16_t* const pUTF16PropName =
        reinterpret_cast< const uint16_t* >(propertyNameString.data());
    WebScriptProxyVariant* pExceptionVariant = 0;

    // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
    unsigned char const deleteResult = m_pObjRefVariant->m_pVTable->deleteProperty( m_pObjRefVariant.get(),
                                                                                                pUTF16PropName,
                                                                                                propertyNameString.size(),
																								exec->dynamicGlobalObject()->platformInterpreterContext(),
                                                                                                &pExceptionVariant);
    ASSERT((deleteResult == 0) || (deleteResult == 1));
    ASSERT((deleteResult == 0) || (!pExceptionVariant));
    
    if (pExceptionVariant) {

        ASSERT(pExceptionVariant);
        ASSERT(pExceptionVariant->m_pVTable);
        ASSERT(pExceptionVariant->m_pVTable->release);

        // Error on delete. Throw exception we got back into JS. Per Geoff Garen (ggaren@apple.com) at Apple, this is legal.
        JSC::JSValue const pError = jsValueFromBridgingVariant(exec, pExceptionVariant);
        pExceptionVariant->m_pVTable->release(pExceptionVariant);
        exec->setException(pError);               
    }
    
    bool const result = (deleteResult == 1);
    return result;
}


JSC::JSValue ProxyObjectImp::defaultValue(JSC::ExecState* exec, JSC::PreferredPrimitiveType typeHint) const
{
    ASSERT((exec));

    JSC::JSValue retJSValue;
    
    // Perform the defaultValue, then see if it succeeded.
    // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
    WebScriptProxyInvokeGetSetResultWrapper defaultValueResult;  // wrapper obj inits and releases contents of result obj
    retJSValue = JSC::jsUndefined();     // cannot fail
    WebScriptProxyVariantType const variantTypeHint = convertJSTypeToVariantType(typeHint, true);
    m_pObjRefVariant->m_pVTable->defaultValue(  m_pObjRefVariant.get(),
                                                &defaultValueResult.m_result,
                                                variantTypeHint,
                                                exec->dynamicGlobalObject()->platformInterpreterContext() 
                                             );
    if (!webScriptProxyGetSetResultIsSuccess(defaultValueResult.m_result.type)) {

        // Error doing the defaultValue
        ASSERT(!defaultValueResult.m_result.value);

        // Per Geoff Garen (ggaren@apple.com) at Apple, even though no defaultValue implementations currently throw, there is no reason
        // that this is not allowed. As such, we'll throw the exception back to JavascriptCore.
        retJSValue = throwJSExceptionForInvokeGetSetResultWrapper(exec, &defaultValueResult.m_result);
    }
    else {

        // Successful defaultValue
        ASSERT(defaultValueResult.m_result.value);
        ASSERT(!defaultValueResult.m_result.exception);

        // Convert bridging variant we got from the defaultValue operation to a JSValue. Returns NULL on failure.
        retJSValue = jsValueFromBridgingVariant(exec, defaultValueResult.m_result.value);
        if (!retJSValue) {

            // As above, throw exception back to JavascriptCore
            retJSValue = throwInternalErrorJSException(exec);
        }
    }

    ASSERT(retJSValue);
    /*
        chrisb: We decided to bridge defaultValue across the boundary.  We initially decided default value MAY return an object,
                including "this". Callers of defaultValue should guard against defaultValue returning "this" if that would cause
                bad things to happen in the calling side. NEW INFORMATION: PER ECMA 262 SECTION 8.6.2.6 defaultValue should never
                return any sort of object, much less this. Need to decide if we encode that in the variant interface or enforce it
                here. certainly should assert it here if nothing else. Note that per Geoff Garen at Apple, returning an object is OK
				when there is an exception.
    */
    ASSERT((! retJSValue.isObject()) || (exec->hadException()));
    return retJSValue; 
}

JSC::CallType ProxyObjectImp::getCallData(JSC::CallData& callData)
{
    // Forward request to variant. Note that per Geoff Garen at Apple, it is OK for getCallData() to return different values
	// for different instances of the same class (as seen in ObjcFallbackObjectImp::getCallData()) and perhaps even could return
	// different values for the same object instance at different times (this latter case is not well defined - Geoff says 
	// "Though I don't know of any classes that respond dynamically, I don't think it would be harmful if they did. The only 
	// requirement is that you must implement callAsFunction() if you return true")
    // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
    bool const canInvoke = m_pObjRefVariant->m_pVTable->canInvoke(m_pObjRefVariant.get()) != 0;
    if (canInvoke)
    {
        callData.native.function = callAsFunction;
    }
    return canInvoke ? JSC::CallTypeHost : JSC::CallTypeNone;
}


JSC::JSValue JSC_HOST_CALL ProxyObjectImp::callAsFunction(JSC::ExecState* exec, JSC::JSObject* func, JSC::JSValue thisValue, const JSC::ArgList& args)
{
    ASSERT(exec);

    ProxyObjectImp* pThis = static_cast<ProxyObjectImp* >(func);
    JSC::JSValue retJSValue;   

    // It is unclear whether thisObj can ever be NULL. Grepping the code shows some callAsFunction()
    // implementations checking for NULL and others not checking (and crashing if they get NULL). We'll 
	// be safe and check and throw if NULL. I considered using the global object from 
	// exec->dynamicGlobalObject()->globalObject() or the global object from m_pJSGlobalObject->globalObject()
	// as "this" if we get NULL, but it just doesn't seem worth it and it is hard to say which if either
	// of these cases would yield the "correct" global object.
	// Ecma 262 10.2.3 says the global object should be used if an object is not provided, but given
	// the lack of checking for the case in WebKit, I'm assuming that's handled upstream if necessary.   
    if (!thisValue) {

        // Error with "this". Throw error back to JS by setting it up in the exec AND returning it.
        // It seems weird to return it, but this is what WindowFunc::callAsFunction() does.
        retJSValue = throwInternalErrorJSException(exec);
    }
    else {

        // Get variant for thisObj. Returns NULL on failure.
        VariantPtr pThisVariant = convertJSValueToBridgingVariant(exec, thisValue);
        if (!pThisVariant) {

            // Error converting this to variant. Throw error back to JS by setting it up in the exec AND returning it.
            // It seems weird to return it, but this is what WindowFunc::callAsFunction() does.
            retJSValue = throwInternalErrorJSException(exec);
        }
        else {

            // Convert arg JSValues to variants. First we setup a vector of smartpointers to variants.
            //**PERF - could use alloca and save the alloc in the vector, but would then not be able to use smartptrs
            //  and would need to explicitly call release on all variants
            unsigned long const numArgs = args.size();
            
            WTF::Vector< VariantPtr > argsVariants(numArgs);

            JSC::ArgList::const_iterator argIter = args.begin();
            for (unsigned long argIndex = 0 ; argIndex < numArgs; ++argIndex)
            {
                // Convert this one. Returns NULL on failure.
                VariantPtr pArgVariant = convertJSValueToBridgingVariant(exec, (*argIter));
                if (!pArgVariant)
                    break;
                ++argIter;
                argsVariants[ argIndex ] = pArgVariant;
            }

            // Did arg conversion succeed?
            if (numArgs != argsVariants.size()) {

                // Error converting args to variant. Throw error back to JS by setting it up in the exec AND returning it.
                // It seems weird to return it, but this is what WindowFunc::callAsFunction() does.
                retJSValue = throwInternalErrorJSException(exec);
            }
            else {

                // Now we need to get a array of variant pointers. We don't want to take references here
                // since the vector of VariantPtrs takes care of that.
                std::auto_ptr<WebScriptProxyVariant*> rawArgsVariants;
                if (numArgs > 0) {

                    rawArgsVariants = std::auto_ptr< WebScriptProxyVariant* >(new WebScriptProxyVariant* [ numArgs ]);
                    ASSERT(rawArgsVariants.get());    // gonna crash if this goes off...
                    for (unsigned long i = 0 ; i < numArgs ; ++i) {

                        rawArgsVariants.get()[ i ] = argsVariants[ i ].get();
                    }
                }

                // Perform the invoke, then see if it succeeded.
                // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
                WebScriptProxyInvokeGetSetResultWrapper invokeResult;    // wrapper obj inits and releases contents of result obj
                pThis->getVariant()->m_pVTable->invoke(pThis->getVariant().get(),
                                                                pThisVariant.get(),
                                                                numArgs,
                                                                rawArgsVariants.get(),
																exec->dynamicGlobalObject()->platformInterpreterContext(),
                                                                &invokeResult.m_result);
                if (!webScriptProxyGetSetResultIsSuccess(invokeResult.m_result.type)) {

                    // Error doing the invoke
                    ASSERT(!invokeResult.m_result.value);

                    // Throw error back to JS
                    retJSValue = throwJSExceptionForInvokeGetSetResultWrapper(exec, &invokeResult.m_result);
                }
                else {

                    // Successful invoke
                    ASSERT(invokeResult.m_result.value);
                    ASSERT(!invokeResult.m_result.exception);

                    // Convert bridging variant we got from the invoke operation to a JSValue. Returns NULL on failure.
                    retJSValue = jsValueFromBridgingVariant(exec, invokeResult.m_result.value);
                    if (!retJSValue) {

                        // Error converting. Throw error back to JS by setting it up in the exec AND returning it.
                        // It seems weird to return it, but this is what WindowFunc::callAsFunction() does.
                        retJSValue = throwInternalErrorJSException(exec);
                    }
                }
            }
        }
    }

	ASSERT(retJSValue);	// never expected to return NULL, which would be illegal per Geoff Garen at Apple
    return retJSValue;
}

#if ENABLE(APOLLO_PROFILER)
JSC::UString ProxyObjectImp::typeName()
{   
    if (!m_typeName.isNull())
        return m_typeName;

    //if (!m_typeName.empty()) return m_typeName;
	WebCore::StringBuffer buffer(256);
	unsigned long bufferLength = buffer.length(); // length is counted as uint16_t, not as uint8_t
	m_pObjRefVariant->m_pVTable->getObjectTypeName(m_pObjRefVariant.get(), (uint16_t *)buffer.characters(), &bufferLength);
    if (bufferLength) {
	    buffer.resize(bufferLength);
        // save the computed name in order to be reused next time
        String typeName("<RuntimeObject>");
        typeName.append(String(buffer.characters(), buffer.length()));
        m_typeName = JSC::UString(stringToUString(typeName));
        return m_typeName;
    }

    // in the case where we were not able to retrieve the name, it is worth
    // not setting the type name so that next time we try again
    return JSC::UString("<RuntimeObject>");
}
#endif



namespace
{
    class PropertyEnumerator : public WebKitApollo::PropertyEnumHelper<PropertyEnumerator>
    {
        friend class WebKitApollo::PropertyEnumHelper<PropertyEnumerator>;
    public:
        PropertyEnumerator(JSC::ExecState* const exec,
                           JSC::PropertyNameArray& propertyNames);
        void enumProperties(const VariantPtr& pVariant, void* interpreterContext);
    private:
        void enumProc(const uint16_t* const pUTF16PropertyName, unsigned long utf16PropertyNameLength);
        JSC::ExecState* const m_pExecState;
        JSC::PropertyNameArray& m_propertyNames;
    };

    PropertyEnumerator::PropertyEnumerator(JSC::ExecState* const exec,
                                           JSC::PropertyNameArray& propertyNames)
                                            :   m_pExecState(exec),
                                                m_propertyNames(propertyNames)
    {
        ASSERT(m_pExecState);
    }

    void PropertyEnumerator::enumProperties(const VariantPtr& pVariant, void* interpreterContext)
    {
        WebKitApollo::PropertyEnumHelper<PropertyEnumerator>::enumProperties(pVariant.get(), interpreterContext);
    }

    void PropertyEnumerator::enumProc(const uint16_t* const pUTF16PropertyName,
                                      unsigned long utf16PropertyNameLength)
    {
        JSC::Identifier const propertyNameIdentifier(identifierFromUTF16Data(m_pExecState, pUTF16PropertyName,
                                                                        utf16PropertyNameLength));
        m_propertyNames.add(propertyNameIdentifier);
    }
}


void ProxyObjectImp::getPropertyNames(JSC::ExecState* exec, JSC::PropertyNameArray& propertyNames, JSC::EnumerationMode /*mode*/)
{
    ASSERT(exec);

    PropertyEnumerator propertyEnumerator(exec, propertyNames);
    // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
    propertyEnumerator.enumProperties(m_pObjRefVariant, exec->dynamicGlobalObject()->platformInterpreterContext());
}

JSC::ConstructType ProxyObjectImp::getConstructData(JSC::ConstructData& constructData)
{
    // Forward request to variant. Note that per Geoff Garen at Apple, it is OK for getConstructData() to return different 
	// values for different instances of the same class and perhaps even could return different values for the same object 
	// instance at different times (this latter case is not well defined - Geoff says "Though I don't know of any classes that 
	// respond dynamically, I don't think it would be harmful if they did. The only requirement is that you must implement 
	// construct() if you return true")
    // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
    bool const canConstruct = m_pObjRefVariant->m_pVTable->canConstruct(m_pObjRefVariant.get()) != 0;
    if (!canConstruct)
    {
        return  JSC::ConstructTypeNone;
    }
    // Set the constructor function
    constructData.native.function = construct;
    return JSC::ConstructTypeHost;
}


JSC::JSObject* ProxyObjectImp::construct(JSC::ExecState* exec, JSC::JSObject* constructor, const JSC::ArgList& args)
{
    ASSERT(exec);

    ASSERT(constructor);
    ProxyObjectImp* pThis = static_cast<ProxyObjectImp* >(constructor);
    ASSERT(pThis->implementsConstruct()); 

    JSC::JSObject* retJSObject = 0;
    
    // Convert arg JSValues to variants. First we setup a vector of smartpointers to variants.
    //**PERF - could use alloca and save the alloc in the vector, but would then not be able to use smartptrs
    //  and would need to explicitly call release on all variants
    unsigned long const numArgs = args.size();
    
    WTF::Vector<VariantPtr> argsVariants(numArgs);

    JSC::ArgList::const_iterator argIter = args.begin();
    for (unsigned long argIndex = 0 ; argIndex < numArgs; ++argIndex)
    {
        // Convert this one. Returns NULL on failure.
        VariantPtr pArgVariant = convertJSValueToBridgingVariant(exec, (*argIter));
        if (!pArgVariant)
            break;
        ++argIter;
        argsVariants[ argIndex ] = pArgVariant;
    }

    // Did arg conversion succeed?
    if (numArgs != argsVariants.size()) {

        // Error converting args to variant. Throw error back to JS by setting it up in the exec AND returning it.
        // It seems weird to return it, but this is what WindowFunc::callAsFunction() does.
        retJSObject = throwInternalErrorJSException(exec);
    }
    else {

        // Now we need to get a array of variant pointers. We don't want to take references here
        // since the vector of VariantPtrs takes care of that.
        std::auto_ptr<WebScriptProxyVariant*> rawArgsVariants;
        if (numArgs > 0) {

            rawArgsVariants = std::auto_ptr< WebScriptProxyVariant* >(new WebScriptProxyVariant* [ numArgs ]);
            ASSERT(rawArgsVariants.get());    // gonna crash if this goes off...
            for (unsigned long i = 0 ; i < numArgs ; ++i) {
                rawArgsVariants.get()[ i ] = argsVariants[i].get();
            }
        }
        
        // Perform the construct call, then see if it succeeded.
        // **RE-ENTRANCY: it possible that ProxyObjectImp (even "this" instance) may be re-entered from within this call
        WebScriptProxyInvokeGetSetResultWrapper constructResult;     // wrapper obj inits and releases contents of result obj
		pThis->getVariant()->m_pVTable->construct( pThis->getVariant().get(),
                                                            numArgs,
                                                            rawArgsVariants.get(),
															exec->dynamicGlobalObject()->platformInterpreterContext(),
                                                            //&constructResult.m_result);
												  &constructResult.m_result);
        if (!webScriptProxyGetSetResultIsSuccess(constructResult.m_result.type)) {

            // Error doing the construct call
            ASSERT(!constructResult.m_result.value);

            // Throw error back to JS
            JSC::JSValue const retJSValue =
                throwJSExceptionForInvokeGetSetResultWrapper(exec, &constructResult.m_result);
            retJSObject = retJSValue.toObject(exec);
        }
        else {

            // Successful construct call
            ASSERT(constructResult.m_result.value);
            ASSERT(!constructResult.m_result.exception);
            
            // Convert bridging variant we got from the construct operation to a JSObject. Returns NULL on failure.
            JSC::JSValue const retJSValue =
                jsValueFromBridgingVariant(exec, constructResult.m_result.value);
            if (!retJSValue) {

                // Error converting. Throw error back to JS by setting it up in the exec AND returning it.
                // It seems weird to return it, but this is what WindowFunc::callAsFunction() does.
                retJSObject = throwInternalErrorJSException(exec);
            }
            else {

                retJSObject = retJSValue.toObject(exec);
                ASSERT(retJSObject);
            }
        }
    }
    
	return retJSObject;
}


JSC::JSObject* ProxyObjectImp::GetJSObjectProxyForRemoteVariantCreateIfNeeded(JSC::ExecState* exec, const struct WebScriptProxyVariant* pVariant)
{
    ASSERT((pVariant) &&
            (pVariant->m_pVTable->getType(pVariant) == WebScriptProxyVariantTypeObjectRef));
    ASSERT(!JSCoreVariant::isSameImpl(pVariant));

    JSC::JSObject* pRetVal = 0;

    /* First we need to see if we already have a proxy object on our side that corresponds to the remote variant
       in question. This is not just a performance optimization, if we are given the same variant more than once
       we want to use the same proxy object so that object ref comparisons (between proxy objects) will be correct.
       Object ref comparisons are not virtualized or overrideable in any way - the engine just compares JSObject pointers.

       **PERF - rather than use a hashtable to do this mapping, we could allow for storing client data on the variant,
       and put the proxy object there. The trick here is that if there are ever more than two engines which could see the
       same variant, we can't use a simple clientdata scheme, but must have a way to store per-client clientdata on the
       variant.
    */
    t_VariantToProxyJSObjectMap* pVariantToProxyJSObjectMap = getVariantToProxyJSObjectMap();   // rets NULL on failure
    if (pVariantToProxyJSObjectMap) { 

        // Lookup proxy object
        ProxyObjectImp* pExistingProxyObj = pVariantToProxyJSObjectMap->get(pVariant);
        if (pExistingProxyObj) {
        
            // Yep, we've already got one!

            // Setup to give back our proxy object, which is GCed when it is no longer needed
            pRetVal = pExistingProxyObj;
        }
        else {
            RefPtr<JSC::Structure> structure = 
                JSC::Structure::create(JSC::jsNull(), 
                JSC::TypeInfo(JSC::ObjectType, JSC::OverridesGetOwnPropertySlot| JSC::OverridesGetPropertyNames | JSObject::StructureFlags), 
                    AnonymousSlotCount);
            // Create ProxyObjectImp to wrap the variant. It will be GCed by JavaScriptCore when nobody references it
            ProxyObjectImp* const pProxyObjImp = 
                new (exec) ProxyObjectImp(VariantPtr(const_cast< WebScriptProxyVariant* >(pVariant)), structure);
            pRetVal = pProxyObjImp;
            //xxx - I'm not that excited about the const_cast... why don't we just make pVariant non-const and un-const callers as needed...

            // Put ProxyObjectImp onto map - the map just has a weak ref. ~ProxyObjectImp will pull it out of the map
            // This map allows us to  re-use the ProxyObjectImp if we encounter the variant again (allows equivalency
            // on our side to work). Since the ProxyObjectImp keeps a strong ref on the variant, the variant
            // cannot be torn down before the ProxyObjectImp allows it.
            pVariantToProxyJSObjectMap->add(pVariant, pProxyObjImp);  // can fail internally, but crashes if it does
        }
    }

    return pRetVal;
}


JSC::JSValue ProxyObjectImp::throwJSExceptionForInvokeGetSetResultWrapper(JSC::ExecState* exec, const WebScriptProxyInvokeGetSetResult* pResult)
{
    ASSERT((exec) && (pResult));
    ASSERT(!webScriptProxyGetSetResultIsSuccess(pResult->type));

    JSC::JSValue pError;

#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
    COMPILE_ASSERT( (WebScriptProxyVariantInvokeGetSetResultFirst == WebScriptProxyVariantInvokeGetSetResultSuccess) &&
                        ((WebScriptProxyVariantInvokeGetSetResultObjectDead + 1) == WebScriptProxyVariantInvokeGetSetResultLast),
                        enum_has_changed_must_update_code_below);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__
    switch (pResult->type) {

        case WebScriptProxyVariantInvokeGetSetResultSuccess:
            ASSERT(false);    // should never be called in this case
            ASSERT(!pError);   // code below will setup a generic error
            break;

        case WebScriptProxyVariantInvokeGetSetResultException:
            // Attempt to convert variant for exception it to a JSValue. If conversion fails,
            // we'll wind up using the generic error down below
            ASSERT(pResult->exception);
            pError = jsValueFromBridgingVariant(exec, pResult->exception);  
            break;

        case WebScriptProxyVariantInvokeGetSetResultFailed:
            ASSERT(!pError);   // code below will setup a generic error
            break;

        case WebScriptProxyVariantInvokeGetSetResultObjectDead:
            break;

        default:
            ASSERT(false);    // should never get here
            ASSERT(!pError);   // code below will setup a generic error
            break;
    }

    // Do we have an error JSValue so far?
    if (pError) {
        // "Throw" the exception by setting it into the ExecState
        exec->setException(pError);   // cannot fail
    }
    else {
        // No JSValue so far, go with internal error exception
        pError = throwInternalErrorJSException(exec); // cannot fail
    }

    return pError;
}


JSC::JSObject* ProxyObjectImp::throwInternalErrorJSException(JSC::ExecState* exec)
{
    ASSERT(exec);

    // Create "internal error" exception
    JSC::JSObject* retVal = JSC::Error::create(exec, JSC::GeneralError, "Internal Error");       // crashes internally on failure
    ASSERT(retVal);   // I'm not super thrilled about just asserting this, but Error::create() can't fail - it'll crash first. sigh.

    // "Throw" the exception by setting it into the ExecState
    exec->setException(retVal);   // cannot fail

    return retVal;
}


// Init static map ptr to NULL.
JSCoreVariant::t_JSValueToVariantMap* JSCoreVariant::m_pValueToVariantMap = 0;

// Used when we need to fetch the string value of a dead JSCoreVariant
static const uint16_t kDeadString[] = { 'd', 'e', 'a', 'd' };


JSCoreVariant::JSCoreVariant(JSC::JSValue value, JSC::JSGlobalObject* pJSGlobalObject)
    : m_refCount(1), m_pValue(value), m_pJSGlobalObject(pJSGlobalObject)
{
    ASSERT((value) && (pJSGlobalObject));

    JSC::JSLock lock(false);

    // Cache the type of the value (needed if we go dead - see DESIGN NOTE above JSCoreVariant class)
    m_CachedType = convertJSTypeToVariantType(m_pValue);	// Do not expect JSCoreVariant code to be re-entered from here
    ASSERT((m_CachedType >= WebScriptProxyVariantTypeFirst) && (m_CachedType < WebScriptProxyVariantTypeLast));
}


JSCoreVariant::~JSCoreVariant()
{
    // Pull out of map if we're in it
    removeSelfFromJSValueToVariantMap();
}


void JSCoreVariant::removeSelfFromJSValueToVariantMap() const
{
    ASSERT(((!m_pValue) && (!m_pJSGlobalObject)) || (m_pValue && m_pJSGlobalObject));
    if (m_pValue) {
        // Get the map
        t_JSValueToVariantMap* pJSValueToVariantMap = getValueToVariantMap();
        ASSERT(pJSValueToVariantMap); 
        if (pJSValueToVariantMap) { 
        
            // See if we're in the map. If this method has already been called, or we're already dead, we won't be.
            t_JSValueToVariantMap::iterator iter = pJSValueToVariantMap->find(t_JSValueToVariantKey(JSC::JSValue::encode(m_pValue.get()), m_pJSGlobalObject));
            if (iter != pJSValueToVariantMap->end())
                pJSValueToVariantMap->remove(iter);   // pull ourselves out of the map
        }
    }
}

// Mark the proxy of this object dead.
void markObjectProxyDead(JSC::JSValue value, JSC::JSGlobalObject* pJSGlobalObject)
{
    JSCoreVariant::markAsDeadIfNeeded(value, pJSGlobalObject);
}

void JSCoreVariant::markAsDeadIfNeeded(JSC::JSValue value, JSC::JSGlobalObject* pJSGlobalObject)
{
    // Lookup JSValue
    t_JSValueToVariantKey valueKey(JSC::JSValue::encode(value), pJSGlobalObject);
    JSCoreVariant* const coreVariant = getValueToVariantMap()->get(valueKey);
    if (coreVariant) {
        // Marks this variant dead.
        // Cannot be used below since it removes the item from the map
        // that is being iterated.
        ASSERT(coreVariant->m_pJSGlobalObject);
        coreVariant->removeSelfFromJSValueToVariantMap();
        coreVariant->m_pJSGlobalObject = 0;
        coreVariant->m_pValue = JSC::JSValue();
    }
}

void JSCoreVariant::notifyGlobalObjectDead(JSC::JSGlobalObject* jsGlobalObject)
{
    ASSERT(jsGlobalObject);

    // **PERF - the code below does a full traversal of all JSCoreVariants in the world looking for ones
    // that match the interpreter being destroyed. We could improve perf by using a two-stage mapping approach
    // where the first map is keyed on interpreter, and then for each interpreter we have a JSValue -> JSCoreVariant map

    t_JSValueToVariantMap* valueToVariantMap = getValueToVariantMap();
    if (valueToVariantMap) { 

        WTF::Vector< t_JSValueToVariantKey > keysToNuke;

        // Visit each JSCoreVariant in the map and if it matches the Interpreter, put it on a list of things to kill
        t_JSValueToVariantMap::iterator iter = valueToVariantMap->begin();
        t_JSValueToVariantMap::iterator endIter = valueToVariantMap->end();
        while (iter != endIter) {

            // Does current variant's Interpreter match the passed one?
            JSCoreVariant* pCurJSCoreVariant = iter->second;
            ASSERT(pCurJSCoreVariant);
            ASSERT(!pCurJSCoreVariant->isDeadVariant());  // should not have dead variants in map
            if (pCurJSCoreVariant->m_pJSGlobalObject == jsGlobalObject) {
        
                // Make the variant dead by clearing its interpreter and releasing its JSValue to allow it to be GCed
                pCurJSCoreVariant->m_pJSGlobalObject = 0;
                pCurJSCoreVariant->m_pValue = JSC::JSValue();

                // Now we'd like to yank the item out of the map, but the problem is that removing from a hashmap 
                // invalidates all iterators which would screw up our traversal!! We can't reasonably just grab the 
                // key for the next item and do a find and resume from there - rehash on remove could change the order.
                // For now at leas, we just make a list of all the keys we want to remove. May need to go to the 
                // multilevel map idea discussed in **PERF above if perf is a problem.
                keysToNuke.append(iter->first);   // can fail, but crashes internally if it does
                ASSERT(pCurJSCoreVariant->isDeadVariant());   // should now be dead
            }
            ++iter;
        }

        // Remove all the hash table entries that we identified above.
        {
            WTF::Vector< t_JSValueToVariantKey >::const_iterator pCurKeyToNuke = keysToNuke.begin();
            WTF::Vector< t_JSValueToVariantKey >::const_iterator const pEndOfKeysToNuke = keysToNuke.end();
            while (pCurKeyToNuke != pEndOfKeysToNuke) {

                valueToVariantMap->remove(*pCurKeyToNuke);
                ++pCurKeyToNuke;
            }
        }
    }
}


JSC::JSGlobalObject* JSCoreVariant::getJSDOMWindowForJSValue(const JSC::ExecState* exec, JSC::JSValue value)
{
    ASSERT((exec) && (value));
    //The following line is needed to get rid of an unused parameter warning in the release build.
    //assert above uses parameter in debug build.
    (void)value;

    /* We'll just return the ExecState's dynamic interpreter. See the big comment at the top of this
       file "Discussion of Interpreters, ExecState, Context" to understand how this works.

      DESIGN NOTE: Unlike the system WebKit uses for associating Interpreters with wrapper objects,
      we are NOT adding special handling for window objects. If we want to do this in the future
      we'd need to do the following:
       - Detect if the JSValue is a window object, and if so return its Interepreter (see code in convertValueToNPVariant()) here.
       - Store the current dynamic interpreter with the JSCoreVariant (a la "origin Interpreter" in WebKit's wrappers) such that
         a given JSCoreVariant could reference two Interpreters.
       - Invalidate/deadify JSCoreVariant if either Interpreter it references is destroyed
       - When a JSCoreVariant references both Interpreters, make same origin checks between the URLs of each Interpreter on any
         get/set/invoke/etc coming into the JSCoreVariant. These are needed because we are going to perform the operations in the
	     context of the Window object's Interpreter, so the Window object's built-in same origin checks will always succeed. Thus,
    	 we must add our own (as Webkit does) to prevent have a security hole.
       - Consider whether both interpreters must be part of key for looking up JSCoreVariant
    */
    return (const_cast<JSC::ExecState*>(exec))->dynamicGlobalObject();
}


VariantPtr JSCoreVariant::GetJSCoreVariantCreateIfNeeded(const JSC::ExecState* exec, JSC::JSValue value)
{
    ASSERT((exec) && (value));

    VariantPtr pRetVal;

    /* First we need to see if we already have a variant on our side that corresponds to the JSValue in question.
       This is not just a performance optimization, we need to give back the same variant for the same JSValue
       in order to allow the other side to correctly perform equivalency tests (they are not routed across
       the bridging layer). For JSValues that are not in fact object references (JSObjects) one might assume
       that it is not strictly necessary to ensure variant uniqueness because comparisons of these types
       are by value not by reference. However, this is assuming that the same comparison rules are used on
       both sides, which may not be true. Thus, we want to ensure variant uniqueness.

	   Note that for security reasons, it is important that the key includes the Interpreter. It is a bit unfortunate
	   that equivalency (discussed above) is not preserved if we encounter the same JSValue when working with different
	   Interpreters (call chains starting in different frames) but just because we can reach an object from two
	   Interpreters doesn't mean we should treat them (and their origins) as interchangeable. Even if a world where
	   we don't have cross-domain scripting in HTML, an object can be passed between domains via AS3 and you can 
	   get into trouble if Interpreters were shared.

       **PERF - for JSObjects rather than use a special hashtable we could put a special property on
          them that references the variant. Note, however, that this is just a lookup in a different hashtable.
    */
    t_JSValueToVariantMap* pJSValueToVariantMap = getValueToVariantMap();   // returns NULL on failure
    if (pJSValueToVariantMap) { 

        // Get Interpreter that corresponds to JSValue
        JSC::JSGlobalObject* pJSGlobalObject = getJSDOMWindowForJSValue(exec, value);
        ASSERT(pJSGlobalObject);

        // Unbox a few special cases
        if (value.inherits(&JSC::BooleanObject::info) 
         || value.inherits(&JSC::NumberObject::info)
         || value.inherits(&JSC::StringObject::info)) {
            // Oddly, a string prototype is an instance of a string!
            if (!value.inherits(&JSC::StringPrototype::info)) {
                JSC::JSWrapperObject *wrapper = static_cast<JSC::JSWrapperObject *>(value.getObject());
                value = wrapper->internalValue();
            }
        }

        // Lookup JSValue
        t_JSValueToVariantKey valueKey(JSC::JSValue::encode(value), pJSGlobalObject);
        JSCoreVariant* pExistingVariant = pJSValueToVariantMap->get(valueKey);
        if (pExistingVariant) {
        
            // Yep, we've already got one!
            ASSERT(!pExistingVariant->isDeadVariant());

            // Setup to give back our variant. VariantPtr takes a ref
            pRetVal = pExistingVariant->getApolloScriptProxyVariant();
        }
        else {

            // Make sure it is a JSValue type that we understand/expect
			if(value.isNumber() ||
			   value.isBoolean() ||
			   value.isUndefinedOrNull() ||
			   value.isNull() ||
			   value.isString() ||
			   value.isObject()) {
				// Create JSCoreVariant for the JSValue
				pRetVal = JSCoreVariant::Construct(value, pJSGlobalObject);  
				ASSERT(JSCoreVariant::isSameImpl(pRetVal.get()));
				// Put JSCoreVariant onto map - the map just has a weak ref. ~JSCoreVariant will pull it out of the map
				// This map allows us to re-use the variant if we encounter the JSValue again (allows equivalency
				// on remote side to work), as well as to make the variant dead if its interpreter goes away.
				pJSValueToVariantMap->add(valueKey, JSCoreVariant::downCast(pRetVal.get()));   // can fail internally, but crashes if it does				
			}
			else {
				// Should not get here
				ASSERT(false);
				pRetVal = 0;
			}			
        }
    }

    return pRetVal;
}


JSCoreVariant::t_JSValueToVariantMap* JSCoreVariant::getValueToVariantMap()
{
    // Create new map if we don't have one, stashing it in class-static variable
    if (!m_pValueToVariantMap)
        m_pValueToVariantMap = new t_JSValueToVariantMap;
    
    return m_pValueToVariantMap;
}

WebScriptProxyVariantType JSCoreVariant::getType() const
{
    ASSERT((m_CachedType >= WebScriptProxyVariantTypeFirst) && (m_CachedType < WebScriptProxyVariantTypeLast));

	/* Note that whether or not JSValue::type() is immutable for a particular JSValue instance is not super well-defined.
	   Per Geoff Garen at Apple: "I know of one class -- ObjcFallbackObjectImp -- that has code to return a different type() 
	   in different circumstances. Two comments about that: (1) I think that behavior is silly; (2) I think that, in actuality, 
	   it's dead code, so you can safely ignore it." Note, however, that as of now (10/2007) a comment in JSValue.h with
	   the type() method specifically says you shouldn't cache the result... we'll keep doing it per Geoff's email at least
	   until there's a problem. */

    // Note that we don't care if the variant is dead or not - the cached value lets us keep returning the type
    return m_CachedType;
}


bool JSCoreVariant::getBool() const
{
    bool retVal;

    ASSERT(getType() == WebScriptProxyVariantTypeBoolean);

    JSC::JSLock lock(false);

    // If dead variant, we return false. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        retVal = false; 
    else 
        retVal = m_pValue.get().getBoolean();	// Do not expect JSCoreVariant code to be re-entered from here

    return retVal;
}


double JSCoreVariant::getNumber() const
{
    double retVal;

    ASSERT(getType() == WebScriptProxyVariantTypeNumber);

    JSC::JSLock lock(false);

    // If dead variant, we return NaN. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        retVal = JSC::NaN;   
    else 
        m_pValue.get().getNumber(retVal);		// Do not expect JSCoreVariant code to be re-entered from here

    return retVal;
}


unsigned long JSCoreVariant::getStringUTF16Length() const
{
    unsigned long retVal;

    ASSERT(getType() == WebScriptProxyVariantTypeString);

    JSC::JSLock lock(false);

    // If dead variant, we return length of dead string. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        retVal = (sizeof(kDeadString) / sizeof(kDeadString[ 0 ]));    
    else {

#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
        COMPILE_ASSERT( (  (sizeof(UChar) == 2) &&
                                (sizeof(UChar) == sizeof(uint16_t))),
                            UChar_must_be_two_bytes_in_size); // UString needs to work in 16bit chars
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__

        // Get ExecState to use 
        JSC::ExecState* pExecState = getExecState();
        ASSERT(pExecState);
        ASSERT(!pExecState->hadException());

        JSC::UString stringValue = m_pValue.get().getString(pExecState);    // this is just an addref and cannot fail, and do not expect JSCoreVariant code to be re-entered from here
        retVal = stringValue.size();    // UString::size() does not include NULL terminators
    }

    return retVal;
}


void JSCoreVariant::getStringUTF16(uint16_t* pDest) const
{
    ASSERT(pDest);
    ASSERT(getType() == WebScriptProxyVariantTypeString);

    JSC::JSLock lock(false);

    // If dead variant, we give dead string. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        memcpy(pDest, kDeadString, (sizeof(kDeadString) / sizeof(kDeadString[ 0 ])));
    else {

#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
        COMPILE_ASSERT( (  (sizeof(UChar) == 2) &&
                                (sizeof(UChar) == sizeof(uint16_t))),
                            UChar_must_be_two_bytes_in_size); // UString needs to work in 16bit chars
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__

        // Get ExecState to use 
        JSC::ExecState* pExecState = getExecState();
        ASSERT(pExecState);
        ASSERT(!pExecState->hadException());

        JSC::UString stringValue = m_pValue.get().getString(pExecState);    // this is just an addref and cannot fail, and do not expect JSCoreVariant code to be re-entered from here
        if (stringValue.size() > 0)   // in theory test is not needed, but I'm not sure it is legit to call data() on empty string so lets be safe
            memcpy(pDest, stringValue.data(), (stringValue.size() * sizeof(UChar)));
    }
}


bool JSCoreVariant::hasProperty(const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength, void*) const
{
    ASSERT(pUTF16PropertyName);
    // NOTE: 0-len property name (empty string) IS legal

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);

    bool retVal;
    
    JSC::JSLock lock(false);
        
	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we treat property as not present. See DESIGN NOTE above JSCoreVariant class for more info. 
    if (isDeadVariant())
        retVal = false;
    else {
        
        // Downcast from JSValue to JSObject so we can forward the request
        JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        // Get ExecState to use 
        JSC::ExecState* pExecState = getExecState();
        ASSERT(pExecState);
        ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here

        //the dynamic global object should be set to the global object of the runtime object context
        JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());         
        
        // Forward call to JSObject::hasProperty
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
        retVal = pJSObject->hasProperty(pExecState, identifierFromUTF16Data(pExecState, pUTF16PropertyName, utf16PropertyNameLength));
        ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) hasProperty cannot throw exceptions
    }

    return retVal;
}

void JSCoreVariant::getProperty(const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength, void*, WebScriptProxyInvokeGetSetResult* pPropertyGetResult) const
{
    ASSERT((pUTF16PropertyName) && (pPropertyGetResult));

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);
    
    JSC::JSLock lock(false);

	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we give exception. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        fillConstantErrorInfo( pPropertyGetResult,
                                WebScriptProxyVariantInvokeGetSetResultObjectDead);
    else {

        // Get ExecState to use 
        JSC::ExecState* pExecState = getExecState();
        ASSERT(pExecState);
        ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here

        // Downcast from JSValue to JSObject so we can forward the request
        JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        //the dynamic global object should be set to the global object of the runtime object context
        JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());         
        
        // Do the get on the JSObject. Note that we do not call hasProperty on it before doing so. This is because we are fine
        // with the behavior of get() for missing properties (it gives back undefined JSValue)
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
        JSC::JSValue pResultJSValue = pJSObject->get(pExecState, identifierFromUTF16Data(pExecState, pUTF16PropertyName, utf16PropertyNameLength));
        ASSERT(pResultJSValue);   // Per Geoff Garen at Apple (ggaren@apple.com), get() should never return NULL

        // Setup the result struct based on the get, including handling reporting errors, clearing exception in exec as needed
        // Note that clearing any exception in the ExecState is important per Geoff Garen at Apple
        handleGetOrCallResult(pExecState, pResultJSValue, pPropertyGetResult);
    }
}


void JSCoreVariant::fillErrorInfoFromException(const JSC::ExecState* pExecState, WebScriptProxyInvokeGetSetResult* pResult) 
{
    ASSERT((pExecState) && (pExecState->hadException()) && (pResult));

    // Try to convert exception object into variant we can hand back. Note that while the ExecState is passed in,
    // it is OK that it already contains an exception.
    VariantPtr pVariant = convertJSValueToBridgingVariant(pExecState, pExecState->exception());
    if (!pVariant) {

        // Couldn't convert exception - use generic error code
        pResult->type                   = WebScriptProxyVariantInvokeGetSetResultFailed;
        pResult->exception  = 0;
    }
    else {

        // Got exception - setup to give it back - note that we need to bump the refcount on the exception variant
        // because the struct owns a ref.
        pResult->type = WebScriptProxyVariantInvokeGetSetResultException;
        pResult->exception = pVariant.release();
    }

    // No value in exception cases
    pResult->value = 0;
}


void JSCoreVariant::fillConstantErrorInfo(WebScriptProxyInvokeGetSetResult* pResult, WebScriptProxyVariantInvokeGetSetResultType error)
{
    ASSERT((pResult) && ((error == WebScriptProxyVariantInvokeGetSetResultFailed) || (error == WebScriptProxyVariantInvokeGetSetResultObjectDead)));

    pResult->type       = error;
    pResult->value      = 0;
    pResult->exception  = 0;
}


void JSCoreVariant::setProperty(const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength, ::WebScriptProxyVariant* pValue, void*, WebScriptProxyInvokeGetSetResult* pPropertySetResult)
{
    ASSERT((pUTF16PropertyName) && (pValue) && (pPropertySetResult));

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);
    
    JSC::JSLock lock(false);

	VariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we give dead object exception. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        fillConstantErrorInfo(pPropertySetResult, WebScriptProxyVariantInvokeGetSetResultObjectDead);
    else {
        // Get ExecState to use 

		// Get ExecState to use 
        JSC::ExecState* pExecState = getExecState();
        ASSERT(pExecState);
        ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here


        // Convert passed variant to JSValue. This will take a reference on pValue if needed. JSValues are GCed as needed so we need not free.
        JSC::JSValue pJSValue = jsValueFromBridgingVariant(pExecState, pValue);   // NOTE: not clear that in perfect world we should use same ExecState for value as for the put...
        if (!pJSValue)
            fillConstantErrorInfo(pPropertySetResult, WebScriptProxyVariantInvokeGetSetResultFailed); // give generic failure exception
        else {
            // Downcast our JSValue from JSValue to JSObject so we can forward the request
            JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
            ASSERT(pJSObject);

            /* DESIGN NOTE: should we be calling canPut() before calling put? NO. Not our job to enforce this.
               If caller wants to do this, they can. Also, per Ecma-262 and the default JSObject::put() implementation,
               canPut() is called within put() so no need to do it here. */

            //the dynamic global object should be set to the global object of the runtime object context
            JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());             
            
            // Do the put on the JSObject. Note that we do not call canPut on it before doing so. This is because we are fine
            // with the behavior of put() without a prior call to canPut(). It appears to be standard practice in the JavaScriptCore
            // codebase NOT to call canPut() before calling put()           
            // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
			JSC::PutPropertySlot slot;
            pJSObject->put(pExecState, identifierFromUTF16Data(pExecState, pUTF16PropertyName, utf16PropertyNameLength), pJSValue, slot);
            if (pExecState->hadException()) {

                // Failure. Record exception info into result object, includes setting of pPropertySetResult->value to NULL
                fillErrorInfoFromException(pExecState, pPropertySetResult);

                // Clear out exception. Note that clearing any exception in the ExecState is important per Geoff Garen at Apple
                pExecState->clearException();
            }
            else {
            
                // Success!
                pPropertySetResult->type      = WebScriptProxyVariantInvokeGetSetResultSuccess;
                pPropertySetResult->value     = 0;
                pPropertySetResult->exception = 0;
            }

            // This should always be NULL for set case
            ASSERT(!pPropertySetResult->value);
        }
    }
}


bool JSCoreVariant::deleteProperty(const uint16_t* pUTF16PropertyName,
                                    unsigned long utf16PropertyNameLength,
									void*,
                                    ::WebScriptProxyVariant** ppDeleteException)
{
    ASSERT(pUTF16PropertyName);

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);

    bool retVal;
    
    JSC::JSLock lock(false);

	VariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we give a false result. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        retVal = false; // because the property did not exist and could not have been deleted if it did exist
    else {

        // Downcast our JSValue from JSValue to JSObject so we can forward the request
        JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        // Get ExecState to use 
        JSC::ExecState* const pExecState = getExecState();

        //the dynamic global object should be set to the global object of the runtime object context
        JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());         
        
        // Simply forward the delete request. Its return result maps correctly to what the variant interface expects.
		// Per Geoff Garen at Apple, deleteProperty() CAN produce exceptions.
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
        retVal = pJSObject->deleteProperty(pExecState, identifierFromUTF16Data(pExecState, pUTF16PropertyName, utf16PropertyNameLength));      
        if (pExecState->hadException()) {

            // Try to convert exception object into variant we can hand back. Note that while the ExecState is passed in,
            // it is OK that it already contains an exception.
            VariantPtr pVariant = convertJSValueToBridgingVariant(pExecState, pExecState->exception());
            if (pVariant) {

                *ppDeleteException = pVariant.release();
            }
            retVal = false;
    
            // Clear out exception. Note that clearing any exception in the ExecState is important per Geoff Garen at Apple
            pExecState->clearException();
        }
    }

    return retVal;
}


bool JSCoreVariant::canInvoke() const
{
    bool retVal;

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);

    JSC::JSLock lock(false);

	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we return false. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        retVal = false;
    else {

        // Downcast our JSValue from JSValue to JSObject so we can forward the request
        JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        // Forward the request
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call

        JSC::CallData callData;
        JSC::CallType callType = pJSObject->getCallData(callData);
        retVal = callType != JSC::CallTypeNone;
    }

    return retVal;
}


void JSCoreVariant::invoke(::WebScriptProxyVariant* pThisVariant, unsigned long numArguments, ::WebScriptProxyVariant * const * argumentArray, void*, struct WebScriptProxyInvokeGetSetResult* pInvokeResult) const
{
    ASSERT((pThisVariant) && (((numArguments == 0) && (!argumentArray)) || ((numArguments > 0) && (argumentArray))) && (pInvokeResult));
    
    
    WebScriptProxyVariantType const thisVariantType = pThisVariant->m_pVTable->getType(pThisVariant);
    bool const thisIsNullVariant = thisVariantType == WebScriptProxyVariantTypeNull;
    
    // Determine if the thisVariant, is the null variant.  If it is we will use the global object of the
    // dynamic interpreter as the "this" object.
    // In future I think this should be handled by having the code calling this function ( on the other side of the DLL boundary ),
    // translate invoke's with null this objects to calls of the call method of the function object with null passed in as the "this" object.
    ASSERT((thisVariantType == WebScriptProxyVariantTypeObjectRef) || (thisIsNullVariant));

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);
    
    JSC::JSLock lock(false);

	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we give exception. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        fillConstantErrorInfo(pInvokeResult, WebScriptProxyVariantInvokeGetSetResultObjectDead);
    else {

        // If variant does not support invoke, throw back a generic exception
        if (!canInvoke()) 
            fillConstantErrorInfo(pInvokeResult, WebScriptProxyVariantInvokeGetSetResultFailed);
        else {

            // Get ExecState to use 
            JSC::ExecState* const pExecState = getExecState();
            ASSERT(pExecState);
            ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here
            ASSERT(pExecState->dynamicGlobalObject());
            
            // Convert passed "this" variant to JSValue. This will take a reference on pThisVariant if needed. JSValues are GCed as needed so we need not free.
            JSC::JSValue pThisJSValue = thisIsNullVariant ? pExecState->globalThisValue() : jsValueFromBridgingVariant(pExecState, pThisVariant); 
            if (!pThisJSValue)
                fillConstantErrorInfo(pInvokeResult, WebScriptProxyVariantInvokeGetSetResultFailed);  // give generic failure exception
            else {
                
                // Convert args to List of JSValues. References will be taken on variants as needed. JSValues are GCed as needed so we need not free.
                // The List object itself serves as a GC root so the JSValues in it will not be GCed while they are in the List
                bool hadError = false;
                JSC::MarkedArgumentBuffer argsList;

                for (unsigned long i = 0; i < numArguments; i++) {
                
                    ASSERT(argumentArray[ i ]);
                    JSC::JSValue pJSValueForArg = jsValueFromBridgingVariant(pExecState, argumentArray[ i ]);
                    if (!pJSValueForArg) {

                        hadError = true;
                        break;
                    }

                    argsList.append(pJSValueForArg);  // This can clearly fail internally, but current imp crashes internally if it does
                }

                // Was there a failure converting args to JSValues?
                if (hadError) 
                    fillConstantErrorInfo(pInvokeResult, WebScriptProxyVariantInvokeGetSetResultFailed);  // give generic failure exception
                else {
                    // get pointer to this object from JS value for this.
                    JSC::JSObject* const pJSThisObject = pThisJSValue.getObject();
                    ASSERT(pJSThisObject);

                    // Downcast our JSValue from JSValue to JSObject so we can forward the request
                    JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
                    ASSERT(pJSObject);

                    // Make the call on the JSObject
                    // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
                    JSC::CallData callData;
                    JSC::CallType callType = pJSObject->getCallData(callData);
                    ASSERT(callType != JSC::CallTypeNone);
                    
                    //the dynamic global object should be set to the global object of the runtime object context
                    JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());                    
                    
                    JSC::JSValue pResultJSValue = JSC::call(pExecState, pJSObject, callType, callData, pJSThisObject, argsList);
                    ASSERT(pResultJSValue);   // Per Geoff Garen at Apple (ggaren@apple.com) call() should never return NULL

                    // Setup the result struct based on the call, including handling reporting errors, clearing exception in exec as needed
                    // Note that clearing any exception in the ExecState is important per Geoff Garen at Apple
                    handleGetOrCallResult(pExecState, pResultJSValue, pInvokeResult);
                }
            }
        }
    }
}


void JSCoreVariant::enumProperties(WebScriptProxyVariantEnumPropertiesProcFunction enumProc,
                                    WebScriptProxyVariantPropertiesEnumData* pClientData,
									void*) const 
{
    ASSERT(enumProc);
    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);

    // If dead variant, we do nothing. See DESIGN NOTE above JSCoreVariant class for more info.
    if (!isDeadVariant()) {

        JSC::JSLock lock(false);

		ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

        // Downcast our JSValue from JSValue to JSObject so we can forward the request
        JSC::JSObject* const pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        // Get ExecState to use 
        JSC::ExecState* pExecState = getExecState();
        ASSERT(pExecState);
        ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here

        //the dynamic global object should be set to the global object of the runtime object context
        JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());         
        
        // Ask JSObject to cough up list of property names
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
        JSC::PropertyNameArray propertyNames(pExecState);
        pJSObject->getPropertyNames(pExecState, propertyNames);
// FIX PR#1608426 HERE! - getPropertyNames can throw an exception - RuntimeObjectImp::getPropertyNames does it. 
//		Need to clear the exception before we from the exec before we return and propagate the exception back to our caller.

        // Call enum proc with each property name
        JSC::PropertyNameArray::const_iterator const pEnd = propertyNames.end();
        JSC::PropertyNameArray::const_iterator pCurrPropertyName = propertyNames.begin();
        while (pEnd != pCurrPropertyName) {

            const JSC::Identifier& currPropertyNameIdentifier(*pCurrPropertyName);
            ++pCurrPropertyName;
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
            COMPILE_ASSERT( (  (sizeof(UChar) == 2) &&
                                    (sizeof(UChar) == sizeof(uint16_t))),
                                UChar_must_be_two_bytes_in_size); // Identifier needs to work in 16bit chars
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__
            const uint16_t* const pPropertyName = reinterpret_cast< const uint16_t* >(currPropertyNameIdentifier.data());
	        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
            enumProc(  pClientData,
                        pPropertyName,
                        currPropertyNameIdentifier.size() );
        }
    }
}


bool JSCoreVariant::canConstruct() const
{
    bool retVal;

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);

    JSC::JSLock lock(false);
	

	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we return false. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        retVal = false;
    else {

        // Downcast our JSValue from JSValue to JSObject so we can forward the request
        JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        // Forward the request
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
        JSC::ConstructData constructData;
        retVal = (pJSObject->getConstructData(constructData) != JSC::ConstructTypeNone);
    }

    return retVal;
}


void JSCoreVariant::construct( unsigned long numArguments,
                                ::WebScriptProxyVariant * const * argumentArray,
								void*,
                                WebScriptProxyInvokeGetSetResult* pConstructResult) const
{
    ASSERT((((numArguments == 0) && (!argumentArray)) || ((numArguments > 0) && (argumentArray))) && (pConstructResult));
    
    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);
    
    JSC::JSLock lock(false);

	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we give exception. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        fillConstantErrorInfo(pConstructResult, WebScriptProxyVariantInvokeGetSetResultObjectDead);
    else {

        // If variant does not support construct, throw back a generic exception
        if (!canConstruct()) 
            fillConstantErrorInfo(pConstructResult, WebScriptProxyVariantInvokeGetSetResultFailed);
        else {

            // Convert args to List of JSValues. References will be taken on variants as needed. JSValues are GCed as needed so we need not free.
            // The List object itself serves as a GC root so the JSValues in it will not be GCed while they are in the List
            bool hadError = false;
            JSC::MarkedArgumentBuffer argsList;

            // Get ExecState to use 
            JSC::ExecState* const pExecState = getExecState();
            ASSERT(pExecState);
            ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here

            for (unsigned long i = 0; i < numArguments; i++) {
            
                ASSERT(argumentArray[ i ]);
                JSC::JSValue pJSValueForArg = jsValueFromBridgingVariant(pExecState, argumentArray[ i ]);
                if (!pJSValueForArg) {

                    hadError = true;
                    break;
                }

                argsList.append(pJSValueForArg);  // This can clearly fail internally, but current imp crashes internally if it does
            }

            // Was there a failure converting args to JSValues?
            if (hadError) 
                fillConstantErrorInfo(pConstructResult, WebScriptProxyVariantInvokeGetSetResultFailed);   // give generic failure exception
            else {

                // Downcast our JSValue from JSValue to JSObject so we can forward the request
                JSC::JSObject* const pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
                ASSERT(pJSObject);

                //the dynamic global object should be set to the global object of the runtime object context
                JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());                 
                
                // Do the construct on the JSObject
                // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
                JSC::ConstructData constructData;
                JSC::ConstructType constructType = pJSObject->getConstructData(constructData);
                ASSERT(constructType != JSC::ConstructTypeNone);
                JSC::JSObject* const pResultJSObject = JSC::construct(pExecState, pJSObject, constructType, constructData, argsList);
                ASSERT(pResultJSObject);	// Per Geoff Garen at Apple (ggaren@apple.com) construct cannot return NULL

                // Setup the result struct based on the construct, including handling reporting errors, clearing exception in exec as needed
                // Note that clearing any exception in the ExecState is important per Geoff Garen at Apple
                handleGetOrCallResult(pExecState, pResultJSObject, pConstructResult);
            }
        }
    }
}


void JSCoreVariant::defaultValue(  WebScriptProxyInvokeGetSetResult* pDefaultValueResult,
                                    WebScriptProxyVariantType typeHint,
									void*) const
{
    ASSERT((pDefaultValueResult));

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);

    JSC::JSLock lock(false);

	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we give exception. See DESIGN NOTE above JSCoreVariant class for more info.
    if (isDeadVariant())
        fillConstantErrorInfo( pDefaultValueResult,
                                WebScriptProxyVariantInvokeGetSetResultObjectDead);
    else {
               
        // Get ExecState to use 
        JSC::ExecState* pExecState = getExecState();
        ASSERT(pExecState);
        ASSERT(!pExecState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here

        // Downcast from JSValue to JSObject so we can forward the request
        JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        JSC::JSType const jsTypeHint = convertVariantTypeToJSType(typeHint);

        //the dynamic global object should be set to the global object of the runtime object context
        JSC::DynamicGlobalObjectScopeApollo globalObjectScope(pExecState, pExecState->dynamicGlobalObject());         
        
        // Do the defaultValue call on the JSObject. 
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within this call
		JSC::PreferredPrimitiveType jsPrefPrimitiveType = JSC::NoPreference;
		if(JSC::NumberType == jsTypeHint)
			jsPrefPrimitiveType = JSC::PreferNumber;
		else if(JSC::StringType == jsTypeHint)
			jsPrefPrimitiveType = JSC::PreferString;
		
        JSC::JSValue pResultJSValue = pJSObject->defaultValue(pExecState, jsPrefPrimitiveType);
        ASSERT(pResultJSValue);   // Per Geoff Garen at Apple (ggaren@apple.com) defaultValue() should never return NULL

        // Setup the result struct based on the defaultValue call, including handling reporting errors, clearing exception in exec as needed
        // Note that clearing any exception in the ExecState is important per Geoff Garen at Apple
        handleGetOrCallResult(pExecState, pResultJSValue, pDefaultValueResult);
    }
}


namespace {
    static const char applyPropertyName[] = "apply";
    static const char callPropertyName[] = "call";
}

WebScriptProxyVariantObjectType JSCoreVariant::getObjectType() const
{
    WebScriptProxyVariantObjectType retVal;

    ASSERT(getType() == WebScriptProxyVariantTypeObjectRef);

    JSC::JSLock lock(false);

	ConstVariantPtr pThis(getApolloScriptProxyVariant());	// Add a reference on ourselves in case the other side releases us when we call across

    // If dead variant, we return unknown object type.
    if (isDeadVariant())
        retVal = WebScriptProxyVariantObjectTypeUnknown;
    else {

        JSC::JSObject* pJSObject = m_pValue.get().getObject();	// Do not expect JSCoreVariant code to be re-entered from here
        ASSERT(pJSObject);

        JSC::ExecState* const execState = getExecState();
        ASSERT(execState);
        ASSERT(!execState->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here

        //We attempt to determine what type of object we have.
        //For now we have three types: function, array, and unkown.
        //We check specifically for JSNodeList and JSHTMLCollection because they return true
        //for implementsCall and they do a significant amount of work in hasProperty.  Their
        //has property calls end up changing the state, which is not something we want to cause from here.
        // **RE-ENTRANCY: it possible that JSCoreVariant (even "this" instance) may be re-entered from within these calls
        JSC::CallData callData;
        JSC::CallType callType = pJSObject->getCallData(callData);
        bool canCall = callType != JSC::CallTypeNone;

        if ((canCall) &&
            (!(pJSObject->inherits(&WebCore::JSNodeList::s_info))) &&
            (!(pJSObject->inherits(&WebCore::JSHTMLCollection::s_info))) &&
            (pJSObject->hasProperty(execState, JSC::Identifier(execState, applyPropertyName))) &&
            (pJSObject->hasProperty(execState,  JSC::Identifier(execState, callPropertyName)))) {

            //If the object implements call we'll say it is a function:
            retVal = WebScriptProxyVariantObjectTypeFunction;
        }
        else if (pJSObject->inherits(&JSC::JSArray::info)) {		// Do not expect JSCoreVariant code to be re-entered from here

            //If the object inherits from ArrayInstance, then it is an array.
            retVal = WebScriptProxyVariantObjectTypeArray;
        }
        else {

            //Don't know what the object is.
            retVal = WebScriptProxyVariantObjectTypeUnknown;
        }

		ASSERT(!execState->hadException());  // Should not have an exception on exit. If we did, we'd need to clear it
	}
    return retVal;
}
    
void JSCoreVariant::getObjectTypeName(uint16_t *pUTF16TypeName, unsigned long* pTypeNameLength) const
{
    (void)pUTF16TypeName;
    (void)pTypeNameLength;
}


void JSCoreVariant::handleGetOrCallResult(JSC::ExecState* pExecState, JSC::JSValue pResultJSValue, struct WebScriptProxyInvokeGetSetResult* pResult)
{
    ASSERT((pExecState) && (pResultJSValue) && (pResult));

    // Was there an exception?
    if (!pExecState->hadException()) {
    
        // Success! Now we need to try to convert the result of the call to a variant we can give back
        VariantPtr pVariantResult = convertJSValueToBridgingVariant(pExecState, pResultJSValue);
        if (pVariantResult) {
        
            // All is well. Note that we need to bump up the refcount on the result because the result struct needs to own a ref 
            pResult->type                   = WebScriptProxyVariantInvokeGetSetResultSuccess;
            pResult->exception              = 0;
            pResult->value                  = pVariantResult.release();
        }
        else {
        
            // Failed to get variant for result. Setup generic error.
            pResult->type                   = WebScriptProxyVariantInvokeGetSetResultFailed;
            pResult->exception              = 0;     
            pResult->value                  = 0;
        }
    }
    else {

        // Failure. Record exception info into result object, also sets pInvokeResult->value to NULL
        fillErrorInfoFromException(pExecState, pResult);
        ASSERT(!pResult->value);

        // Clear out exception
        pExecState->clearException();
    }
}


JSC::ExecState* JSCoreVariant::getExecState() const
{
    ASSERT(m_pJSGlobalObject);
    ASSERT(!isDeadVariant());

    JSC::ExecState* retVal = m_pJSGlobalObject->globalExec();
    ASSERT(!retVal->hadException());  // Per Geoff Garen at Apple (ggaren@apple.com) should not already have exception in here
    return retVal;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void notifyGlobalObjectDead(JSDOMWindow* const jsDOMWindow)
{
    ASSERT(jsDOMWindow);

    // Simply forward this
    JSCoreVariant::notifyGlobalObjectDead(jsDOMWindow);
}


/* Gets a variant for the passed JSValue. Will handle the case where the passed JSValue is actually
   a proxy for a remote variant (simply yields remote variant).
   Never sets exception in exec or changes exception in exec (if any).
   Returns NULL on failure */
static VariantPtr convertJSValueToBridgingVariant(const JSC::ExecState* exec, JSC::JSValue value)
{
    ASSERT((exec) && (value));

    VariantPtr pRetVal;

    /* Before we do anything more costly, see if this JSValue is actually one of our proxy (wrapper) objects.
       If it is, we simply need to pull out the variant that it references. We certainly don't want to wrap
       it in yet another layer. */
    if (ProxyObjectImp::isJSValueProxyObject(value)) {

        // Pull Variant out of the proxy object
        ProxyObjectImp* pProxyObj = static_cast< ProxyObjectImp* >(value.getObject()); 
        ASSERT(pProxyObj);
        pRetVal = pProxyObj->getVariant();
        ASSERT(pRetVal);  // no concept of dead objects for proxies, so should always have this
    }
    else {

        // Get JSCoreVariant for the JSValue, creating if needed. Returns NULL if failure.
        pRetVal = JSCoreVariant::GetJSCoreVariantCreateIfNeeded(exec, value);
    }

    return pRetVal;
}


/* Gets a JSValue for the passed variant. Will handle the case where the passed variant is actually
   a local-side variant (simply yields JSValue from variant).
   Returned JSValue will be garbage collected. If you need to keep it alive, make sure to keep
   it on the stack or root it via a ProtectedPtr.
   Returns NULL on failure */
JSC::JSValue jsValueFromBridgingVariant(JSC::ExecState* exec, const struct WebScriptProxyVariant* pVariant)
{
    ASSERT(pVariant);

    JSC::JSValue retVal;

    // Is the variant one of our JSCoreVariant objects (which just wraps a JSValue)?
    if (JSCoreVariant::isSameImpl(pVariant)) {

        // Downcast to JSCoreVariant
        const JSCoreVariant* pJSCoreVariant = JSCoreVariant::downCast(pVariant);

        /* Setup to return the JSValue stored in the JSCoreVariant, or NULL if the variant is dead.
         
           DESIGN NOTE: do we need to compare the current interpreter to the interpreter(s) stored in the
           variant in order to enforce security? NO. We aren't giving the caller access to the interpreter(s)
           in the variant or allowing execution based on these interpreters. Any execution against the returned
           JSValue will happen on behalf of whatever interpreter is active, and any needed security checks are
           made against that interpreter. 
        */
        if (pJSCoreVariant->isDeadVariant())
            retVal = JSC::JSValue();
        else {

            retVal = pJSCoreVariant->getValue();    // just yank JSValue from variant
            ASSERT(retVal);
        }
    }
    else {

        // Variant belongs to other side. Need to convert it to one we can use.
        // We convert primitive types, and wrap object references
        COMPILE_ASSERT( (WebScriptProxyVariantTypeObjectRef + 1) == WebScriptProxyVariantTypeLast,
                            enum_changed_code_must_be_updated);
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
        COMPILE_ASSERT( 6 == WebScriptProxyVariantTypeLast,
                            enum_changed_code_must_be_updated);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__

        switch (pVariant->m_pVTable->getType(pVariant)) {

            case WebScriptProxyVariantTypeNumber:
                {
                    double const variantNumberValue = pVariant->m_pVTable->getNumber(pVariant);
                    retVal = JSC::jsNumber(exec, variantNumberValue);    // getNumber() cannot fail but jsNumber() can - however seems to crash internally if it does
                }
                break;

            case WebScriptProxyVariantTypeBoolean:
                retVal = JSC::jsBoolean(pVariant->m_pVTable->getBool(pVariant) != 0); // cannot fail
                break;

            case WebScriptProxyVariantTypeUndefined:
                retVal = JSC::jsUndefined();     // cannot fail
                break;

            case WebScriptProxyVariantTypeNull:
                retVal = JSC::jsNull();      // cannot fail
                break;

            case WebScriptProxyVariantTypeString: 
            {
                //**PERF - we are making a deep copy here. If we wanted to save copies we could conceivably virtualize
                // the primitive string types on both sides of the boundary to save copies. This would potentially have
                // its own perf issues given the virtualization. Also, really want local-side comparison rules etc.
                // to be in effect.

                // Get length of string (may be 0) in 16bit values 
                unsigned long stringLen = pVariant->m_pVTable->getStringUTF16Length(pVariant);

                // Special case len=0 because I'm wary of calling UString ctor with 0 len 
                if (stringLen == 0) {

                    retVal = JSC::jsString(exec, "");    // returns NULL if it fails
                }
                else {

                    // **PERF - it is unfortunate to have an extra alloc of a temporary buffer here. We could speed
                    // this up by using a stack-based buffer for strings below a certain size.
        
                    // Allocate buffer to hold string and put it into OwnArrayPtr autoptr. Note that we must use array 
                    // new here to match with OwnArrayPtr class (which uses array delete)
                    OwnArrayPtr< uint16_t > pBuf(new uint16_t[ stringLen ]);
                    if (!pBuf) 
                        retVal = JSC::JSValue();  // alloc failed
                    else {

                        // Ask variant to fill in our allocated buffer with the actual string values
                        pVariant->m_pVTable->getStringUTF16(pVariant, pBuf.get());

                        // Instantiate UString that copies the string from the variant. 0-len UStrings are OK.
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
                        COMPILE_ASSERT( (  (sizeof(UChar) == 2) &&
                                                (sizeof(UChar) == sizeof(uint16_t))),
                                            UChar_must_be_two_bytes_in_size); // UString needs to work in 16bit chars
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__
                        JSC::UString copiedString(reinterpret_cast< UChar* >(pBuf.get()),
                                                  stringLen);    // this can fail but crashes internally if it does...

                        // Create JSString around the UString (it makes a copy, but is cheap because of refcounting)
                        retVal = JSC::jsString(exec, copiedString);  // returns NULL if it fails
                    }
                }

                break;
            }

            case WebScriptProxyVariantTypeObjectRef:
                // Get local JSObject subclass that wraps the remote object ref. This function
                // will re-use an existing local JSObject if we already have one for this
                // variant pointer (and thus local equivalency comparisons will work correctly)
                retVal = ProxyObjectImp::GetJSObjectProxyForRemoteVariantCreateIfNeeded(exec, pVariant);    // returns NULL on failure
                break;

            default:
                ASSERT(false);
                retVal = JSC::jsUndefined();     // cannot fail
                break;
        }
    }

    return retVal;
}


WebScriptProxyVariant* getGlobalObject(WebCore::Frame* const frame)
{

    JSC::JSLock lock(false);
    WebScriptProxyVariant* result = 0;
    WebCore::ScriptController* const scriptProxy = frame->script();
    if (scriptProxy) {
        WebCore::JSDOMWindowShell* const jsDOMWindowShell = scriptProxy->windowShell(mainThreadNormalWorld());
        WebCore::JSDOMWindow* const jsDOMWindow = scriptProxy->globalObject(mainThreadNormalWorld());
        if ((jsDOMWindowShell) && (jsDOMWindow)) {
            // ensure a rootObject is associated with the interpreter
            frame->script()->bindingRootObject();
            VariantPtr globalObjVariant(convertJSValueToBridgingVariant(jsDOMWindow->globalExec(), jsDOMWindowShell));
            result = globalObjVariant.release();
        }
    }
    return result;
}


struct WebScriptProxyVariant* getApolloVariantForJSValue(const JSC::ExecState* const exec, JSC::JSValue const value)
{
    VariantPtr pVariantForJSValue(convertJSValueToBridgingVariant(exec, value));
    return pVariantForJSValue.release();
}



} // namespace ApolloScriptBridging

}   // namespace JSC
