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
#ifndef KJS_PROXY_OBJ_H
#define KJS_PROXY_OBJ_H

struct WebScriptProxyVariant;

namespace JSC
{

class Interpreter;
class ExecState;
class JSValue;
class JSGlobalObject;

}

namespace WebCore {

class JSDOMWindowShell;
class JSDOMWindow;
class Frame;

namespace ApolloScriptBridging
{

/**
	Must be called when an Interpreter in the system is destroyed

	@param Interpreter that has been destroyed. This function will not call through the Interpreter or
		deference it in any way.
*/
void notifyGlobalObjectDead(JSDOMWindow* const jsDOMWindow);

struct WebScriptProxyVariant* getGlobalObject(WebCore::Frame* const frame);
struct WebScriptProxyVariant* getApolloVariantForJSValue(const JSC::ExecState* const exec, JSC::JSValue const value);

JSC::JSValue jsValueFromBridgingVariant(JSC::ExecState* exec, const struct WebScriptProxyVariant* const variant);

void markObjectProxyDead(JSC::JSValue const value, JSC::JSGlobalObject* const global);

} // namespace ApolloScriptBridging

}	// namespace WebCore



#endif	/* KJS_PROXY_OBJ_H */
