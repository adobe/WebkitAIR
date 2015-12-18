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

#ifndef WebWebDebugStateHelper_h
#define WebWebDebugStateHelper_h

#include <WebKitApollo/WebKit.h>


namespace WebKitApollo
{
    template <class ImplClass>
    class WebDebugStateHelper : private WebDebugState {

    public:

        inline WebDebugState* getWebDebugState() { return this; }

    protected:
        WebDebugStateHelper();
        virtual ~WebDebugStateHelper();

    private:
        static WebDebugStateVTable const s_VTable;
        static ImplClass* getThis( WebDebugState* const pDebugListener ) { return static_cast<ImplClass*>( pDebugListener ); }
	
	public:

		static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant * WEBKIT_APOLLO_PROTO2 sGetCurrentCallFrame ( struct WebDebugState* pDebugState );

    };

    template <class ImplClass>
    WebDebugStateVTable const WebDebugStateHelper<ImplClass>::s_VTable =
    {
        sizeof( WebDebugStateVTable ),
		WebDebugStateHelper<ImplClass>::sGetCurrentCallFrame
		
    };
    
    template <class ImplClass>
    WebDebugStateHelper<ImplClass>::WebDebugStateHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebDebugStateHelper<ImplClass>::~WebDebugStateHelper()
    {
        m_pVTable = 0;
    }

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1 
	WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebDebugStateHelper<ImplClass>::sGetCurrentCallFrame ( struct WebDebugState* pDebugState ){
		return getThis(pDebugState)->getCurrentCallFrame();
	}

	
	
}
#endif
