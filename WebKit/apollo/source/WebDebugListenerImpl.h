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

#ifndef WebDebugListenerImpl_h
#define WebDebugListenerImpl_h

#include <wtf/Platform.h>
#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif

#include <Debugger.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassRefPtr.h>

#include "SourceProvider.h"
#include "UString.h"
#include "Parser.h"

namespace WebCore {
	class JavaScriptCallFrame;
}

struct WebDebugListener;

namespace WebKitApollo {
	
	using namespace JSC;
	
	class WebDebugStateImpl;

	class WebDebugListenerImpl: public JSC::Debugger
	{

	private:	
		WebDebugListenerImpl (WebDebugListener* listener);
		void removedLastRef();

	public:
		virtual ~WebDebugListenerImpl ();

		static WTF::PassRefPtr<WebDebugListenerImpl> getInstance();

		JSC::Debugger * getDebugger();

		virtual void sourceParsed(ExecState* execState, const SourceCode&, int errorLine, const UString& errorMsg);
	
		virtual void sourceParsedFunctions(ExecState* execState, intptr_t sourceId, JSC::ParserArenaData<DeclarationStacks::FunctionStack>* funcStack);
    
		virtual void exception(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno, bool isCaughtException);

		virtual void atStatement(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno);

		virtual void callEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno);        
		virtual void returnEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno, bool fromReturnCommand);
        
		virtual void willExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno);
		virtual void didExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno);
		virtual void didReachBreakpoint(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceId, int lineno);

		WebCore::JavaScriptCallFrame *currentCallFrame(){
			return m_currentCallFrame.get();
		}

		void ref();

		void deref();
		
	private:
		int m_refCount;

#ifndef NDEBUG
        bool m_deletionHasBegun;
        bool m_inRemovedLastRefFunction;
#endif

		WebDebugListener *m_listener;
		
		//prevents recursivly debugging debugger code
		bool m_inDebug;	

		OwnPtr<WebDebugStateImpl> m_debugState;
		RefPtr<WebCore::JavaScriptCallFrame> m_currentCallFrame;
	};
}

#endif //WebDebugListenerImpl_h
