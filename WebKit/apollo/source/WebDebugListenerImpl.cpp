/*
 * Copyright (C) 2005, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Adobe Systems Incorporated.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config.h>
#include "WebDebugListenerImpl.h"

#include <apollo/proxy_obj.h>
#include "WebDebugStateImpl.h"
#include <JavaScriptCallFrame.h>
#include <Nodes.h>
#include <Parser.h>
#include "SourceProvider.h"
#include "WebKitApollo/WebKit.h"
#include "WebViewImpl.h"
#include <wtf/Vector.h>
#include <Parser.h>
#include <PlatformString.h>
#include <FrameLoaderClientApollo.h>
#include <KURL.h>
#include <JSDOMWindow.h>

#include "JSArray.h"
#include "KURL.h"

namespace WebKitApollo {

extern WebKitAPIHostFunctions* g_HostFunctions;

WebDebugListenerImpl::WebDebugListenerImpl(WebDebugListener* listener)
	: m_refCount(0)
	, m_listener(listener)
	, m_inDebug(false)
	, m_debugState(new WebDebugStateImpl(this))
{
#ifndef NDEBUG
        m_deletionHasBegun = false;
        m_inRemovedLastRefFunction = false;
#endif
}

WebDebugListenerImpl::~WebDebugListenerImpl()
{
	ASSERT(m_deletionHasBegun);
	ASSERT(!m_inDebug);
	m_listener = 0;
}

void WebDebugListenerImpl::ref()
{
	ASSERT(!m_deletionHasBegun);
	ASSERT(!m_inRemovedLastRefFunction);
	++m_refCount;
}

void WebDebugListenerImpl::deref()
{
	ASSERT(!m_deletionHasBegun);
	ASSERT(!m_inRemovedLastRefFunction);
	if (--m_refCount <= 0) {
#ifndef NDEBUG
		m_inRemovedLastRefFunction = true;
#endif
		removedLastRef();
	}
}

void WebDebugListenerImpl::removedLastRef()
{
#ifndef NDEBUG
    m_deletionHasBegun = true;
#endif
    delete this;
}

void WebDebugListenerImpl::sourceParsed(ExecState* execState, const SourceCode& source, int errorLine, const UString& errorMsg)
{
		if (m_listener && !m_inDebug) {
			m_inDebug = true;
            
            WebCore::String coreSourceURL = WebCore::ustringToString(source.provider()->url());
            WebCore::Frame *frame = static_cast<WebCore::JSDOMWindow*>(execState->lexicalGlobalObject())->impl()->frame();
            if (!(coreSourceURL.isNull() || coreSourceURL.isEmpty())) {
                WebCore::KURL sourceURL(WebCore::ParsedURLString, coreSourceURL);
                WebCore::KURL mappedURL;
                if(WebCore::FrameLoaderClientApollo::mapFrameUrl(frame, sourceURL, &mappedURL)){
                    coreSourceURL = mappedURL.string();
                }
            }

			WebCore::String coreErrorMsg = WebCore::ustringToString(errorMsg);
			
            WebCore::String coreSource(source.data(), source.length());

			m_listener->m_pVTable->sourceParsed(m_listener, source.provider()->asID(), coreSourceURL.webString(), coreSource.webString(), source.firstLine(), errorLine, coreErrorMsg.webString());
			
			m_inDebug = false;
		}
}

void WebDebugListenerImpl::exception(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno, bool isCaughtException)
{
	if (m_listener && !m_inDebug) {
		m_inDebug = true;

		ASSERT(m_currentCallFrame);
		m_currentCallFrame->update(debuggerCallFrame, sourceId, lineno);
		
		m_listener->m_pVTable->exception(m_listener, m_debugState->getWebDebugState(), sourceId, lineno, isCaughtException);
		m_inDebug = false;
	}
}

void WebDebugListenerImpl::atStatement(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno)
{
	if (m_listener && !m_inDebug) {
		m_inDebug = true;
		
		ASSERT(m_currentCallFrame);
		m_currentCallFrame->update(debuggerCallFrame, sourceId, lineno);
		
		m_listener->m_pVTable->atStatement(m_listener, m_debugState->getWebDebugState(), sourceId, lineno);
		
		m_inDebug = false;
	}
}

void WebDebugListenerImpl::callEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno)
{
	if (m_listener && !m_inDebug) {
		m_inDebug = true;
		
		m_currentCallFrame = WebCore::JavaScriptCallFrame::create(debuggerCallFrame, m_currentCallFrame, sourceId, lineno);				
		
		m_listener->m_pVTable->callEvent(m_listener, m_debugState->getWebDebugState(), sourceId, lineno);
		
		m_inDebug = false;
	}
}
    
void WebDebugListenerImpl::returnEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno, bool fromReturnCommand)
{
    if (m_listener && !m_inDebug) {
        m_inDebug = true;				
        
        ASSERT(m_currentCallFrame);
        m_currentCallFrame->update(debuggerCallFrame, sourceId, lineno);
        
        m_listener->m_pVTable->returnEvent(m_listener, m_debugState->getWebDebugState(), sourceId, lineno, fromReturnCommand);
        
        m_currentCallFrame = m_currentCallFrame->caller();
        
        m_inDebug = false;
    }
}

void WebDebugListenerImpl::willExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno)
{
	if (m_listener && !m_inDebug) {
		m_inDebug = true;
		
		m_currentCallFrame = WebCore::JavaScriptCallFrame::create(debuggerCallFrame, m_currentCallFrame, sourceId, lineno);
		
		m_listener->m_pVTable->willExecuteProgram(m_listener, m_debugState->getWebDebugState(), sourceId, lineno);

		m_inDebug = false;
	}
}

void WebDebugListenerImpl::didExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno)
{
	if (m_listener && !m_inDebug) {
		m_inDebug = true;
		
		ASSERT(m_currentCallFrame);
		m_currentCallFrame->update(debuggerCallFrame, sourceId, lineno);
		
		m_listener->m_pVTable->didExecuteProgram(m_listener, m_debugState->getWebDebugState(), sourceId, lineno);
		
		m_currentCallFrame = m_currentCallFrame->caller();

		m_inDebug = false;
	}

}

void WebDebugListenerImpl::sourceParsedFunctions(ExecState* execState, intptr_t sourceId, JSC::ParserArenaData<DeclarationStacks::FunctionStack>* funcStack)
{
	if (m_listener && !m_inDebug) {
		m_inDebug = true;
		
		MarkedArgumentBuffer funcList;
		ArgList empty;
		
		Identifier functionName (execState, "functionName");
		Identifier firstLine (execState, "firstLine");
		Identifier lastLine (execState, "lastLine");	
		
		for(size_t i=0, l=funcStack->data.size(); i<l; i++) {
            FunctionBodyNode *funcBody = funcStack->data[i];
			
			ASSERT(funcBody);
			if (!funcBody) continue;
			
			JSObject* funcObj = JSC::constructEmptyObject(execState);
			
			JSC::PutPropertySlot slot;
			funcObj->put(execState, functionName, jsString( execState, funcBody->ident().ustring() ), slot );
			
			funcObj->put(execState, firstLine, jsNumber ( execState, funcBody->firstLine() ), slot );
			funcObj->put(execState, lastLine,  jsNumber ( execState, funcBody->lastLine() ), slot);		
			
			funcList.append(funcObj);
			
		}
		
		JSObject* functions = JSC::constructArray(execState, funcList);
		
		WebScriptProxyVariant* functionsVariant = WebCore::ApolloScriptBridging::getApolloVariantForJSValue(execState, functions);
		
		m_listener->m_pVTable->sourceParsedFunctions(m_listener, sourceId, functionsVariant);
		
		m_inDebug = false;
	}
}

JSC::Debugger * WebDebugListenerImpl::getDebugger()
{
	return this;
}

WTF::PassRefPtr<WebDebugListenerImpl> WebDebugListenerImpl::getInstance()
{
	static bool preventSecondTry = false;
	static WTF::RefPtr<WebDebugListenerImpl> htmlDebugListener;

	if (!preventSecondTry && !htmlDebugListener.get()) {
		preventSecondTry = true;

		ASSERT(WebKitApollo::g_HostFunctions->getWebDebugListener);
		WebDebugListener * debugger = WebKitApollo::g_HostFunctions->getWebDebugListener();
		if (debugger) {
			// debugger might be 0 when it's not activated;
			htmlDebugListener = new WebDebugListenerImpl( debugger );
		}
	}		

	return htmlDebugListener.get();
}

void WebDebugListenerImpl::didReachBreakpoint(const DebuggerCallFrame&, intptr_t sourceID, int lineno)
{
    // not used in APOLLO
}

}
