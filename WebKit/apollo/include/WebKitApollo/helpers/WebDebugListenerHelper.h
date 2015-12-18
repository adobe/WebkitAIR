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

#ifndef WebDebugListenerHelper_h
#define WebDebugListenerHelper_h

#include <WebKitApollo/WebKit.h>


namespace WebKitApollo
{
    template <class ImplClass>
    class WebDebugListenerHelper : private WebDebugListener {

    public:
    
        inline WebDebugListener* getWebDebugListener() { return this; }

    protected:
        WebDebugListenerHelper();
        virtual ~WebDebugListenerHelper();

    private:
        static WebDebugListenerVTable const s_VTable;
        static ImplClass* getThis( WebDebugListener* const pDebugListener ) { return static_cast<ImplClass*>( pDebugListener ); }
	
	public:
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSourceParsed ( struct WebDebugListener* pWebDebugListener, intptr_t sourceId, WebString *sourceURL, WebString *source, int startingLineNumber, int errorLine, WebString *errorMsg );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sException ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno, bool isCaughtExeption );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAtStatement ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sCallEvent ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sReturnEvent ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno, bool fromReturnCommand );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sWillExecuteProgram ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDidExecuteProgram ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSourceParsedFunctions ( struct WebDebugListener* pWebDebugListener, intptr_t sourceId, WebScriptProxyVariant * funcList );
    };

    template <class ImplClass>
    WebDebugListenerVTable const WebDebugListenerHelper<ImplClass>::s_VTable =
    {
        sizeof( WebDebugListenerVTable ),
		WebDebugListenerHelper<ImplClass>::sSourceParsed,	
		WebDebugListenerHelper<ImplClass>::sException,
		WebDebugListenerHelper<ImplClass>::sAtStatement,
		WebDebugListenerHelper<ImplClass>::sCallEvent,
		WebDebugListenerHelper<ImplClass>::sReturnEvent,
		WebDebugListenerHelper<ImplClass>::sWillExecuteProgram,
		WebDebugListenerHelper<ImplClass>::sDidExecuteProgram, 
		WebDebugListenerHelper<ImplClass>::sSourceParsedFunctions, 
    };
    
    template <class ImplClass>
    WebDebugListenerHelper<ImplClass>::WebDebugListenerHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebDebugListenerHelper<ImplClass>::~WebDebugListenerHelper()
    {
        m_pVTable = 0;
		(* reinterpret_cast<int*>( this )) = NULL; 
    }

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sSourceParsed ( struct WebDebugListener* pWebDebugListener, intptr_t sourceId, WebString *sourceURL, WebString *source,  int startingLineNumber, int errorLine, WebString *errorMsg  ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->sourceParsed( sourceId, sourceURL, source,
			startingLineNumber, errorLine, errorMsg );
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sException ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno, bool isCaughtExeption ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->exception(pWebDebugState, sourceId, lineno, isCaughtExeption);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sAtStatement ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->atStatement(pWebDebugState, sourceId, lineno);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sCallEvent ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->callEvent(pWebDebugState, sourceId, lineno);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sReturnEvent ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno, bool fromReturnCommand ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->returnEvent(pWebDebugState, sourceId, lineno, fromReturnCommand);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sWillExecuteProgram ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->willExecuteProgram(pWebDebugState, sourceId, lineno);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sDidExecuteProgram ( struct WebDebugListener* pWebDebugListener, struct WebDebugState *pWebDebugState, intptr_t sourceId, int lineno ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->didExecuteProgram(pWebDebugState, sourceId, lineno);
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	void WEBKIT_APOLLO_PROTO2 WebDebugListenerHelper<ImplClass>::sSourceParsedFunctions ( struct WebDebugListener* pWebDebugListener, intptr_t sourceId, WebScriptProxyVariant * funcList ){
		ImplClass* const pThis = getThis(pWebDebugListener);
		if (pThis->isUnusable())
			return;
		pThis->sourceParsedFunctions( sourceId, funcList );
	}
	
}
#endif
