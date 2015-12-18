/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
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
#include "ApolloHelper.h"

#if PLATFORM(APOLLO)

#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "JSDOMWindowCustom.h"
#include "Page.h"
#include "Chrome.h"

using namespace JSC;

namespace WebCore {

// check if loading the script is restricted
    bool loadScriptRestricted(Frame* frame, KURL& scriptURL)
    {
        if (frame) {
            FrameLoaderClient* frameLoaderClient = frame->loader()->client();
            if(!frameLoaderClient->canLoadScript(scriptURL))
                return true;
        }

        return false;
    }

    bool settingStringTimerRestricted(const JSC::JSValue& value, Frame* frame)
    {

        if (value.isString() && frame) {
            FrameLoaderClient *frameLoaderClient = frame->loader()->client();
            if (!frameLoaderClient->canSetStringTimer())
                return true;
        }

        return false;
    }

    bool isLocationChangeSafe(JSC::ExecState* exec, const String& url, const JSDOMWindow* window)
    {
        if (window) {
            if (protocolIsJavaScript(url)) {
                // JavaScript URLs are OK as long as the caller has scripting rights to this window
                return window->allowsAccessFrom(exec);
            }

            // Don't allow non-app content to replace app content.
            Frame* const targetFrame = window->impl()->frame();
            if (targetFrame) {
                if (targetFrame->loader()->client()->hasApplicationRestrictions()) {
                    Frame* const scriptFrame = asJSDOMWindow(exec->dynamicGlobalObject())->impl()->frame();
                    if (!scriptFrame->loader()->client()->hasApplicationPrivileges())
                        return false;
                }
            }
        }
        // otherwise, OK
        return true;
    }

    void createWindowSetOpener(Frame* openerFrame, Frame* newFrame)
    {
        // One can rewrite HTMLHost::CreateWindow such that it reuses the top frame. 
	    // We treat that as an anomaly and we do not set the opener property.
	    // Hence, the loaded content will not inherit any properties (like documentRoot) from its loader.
	    // For more details see bug 2537639.
	    if( openerFrame->tree()->top() == newFrame )
		    newFrame->loader()->setOpener(0);
	    else
		    newFrame->loader()->setOpener(openerFrame);
    }

    void reportApolloException(JSC::ExecState* exec, JSC::JSValue value, Frame* frame)
    {
        ASSERT(exec);
        ASSERT(frame);

        reportException(exec, value);
        if (Page* page = frame->page()) {
            page->chrome()->uncaughtJavaScriptException(exec, value);
        }
    }

    void reportApolloException(JSDOMGlobalObject* globalObject)
    {
        ASSERT(globalObject);

        ExecState* exec = globalObject->globalExec();
        ASSERT(exec);

        ASSERT(exec->hadException());

        JSValue exceptionValue = exec->exception();
        exec->clearException();

        JSDOMWindow* window = asJSDOMWindow(globalObject);
        Frame* frame = window->impl()->frame();

        reportApolloException(exec, exceptionValue, frame);
    }

    void reportApolloException(ExecState* exec)
    {
        JSValue exceptionValue = exec->exception();
        exec->clearException();

        JSDOMWindow *window = asJSDOMWindow(exec->lexicalGlobalObject());
        ASSERT(window);
        Frame* frame = window->impl()->frame();

        reportApolloException(exec, exceptionValue, frame);
    }

    bool needsForbiddenDocOpen(Document* doc)
    {
        KURL url = doc->url();
        Document* parent = doc->parentDocument();
        if( parent && (url.isEmpty() || url == blankURL()) )
            url = parent->url();
            
        return !doc->tokenizer() && (url.protocol().lower() == "app");
    }

    bool elementCanUseSimpleDefaultStyle(Element*);
    bool elementCanUseSimpleDefaultStyle(Element*)
    {
        // Latest version of webkit optimizes simple HTML loading by loading just 
        // a small CSS (simpleUserAgentStyleSheet) and replace it later when the HTML 
        // get more complex (contains other tags than the ones enumerated below)
        // This actually should be a bug, because the <HTML> tag is parsed
        // using the simple stylesheet while the full CSS file contains a 
        // font-family definition for <html> (Verdana, Helvetica, Arial)
        // which is ignored.
        // The fix disables the simple css loader.
        return false;
    }

}

#endif

