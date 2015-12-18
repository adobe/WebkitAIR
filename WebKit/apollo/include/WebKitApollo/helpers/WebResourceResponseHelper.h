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
 
#ifndef WebResourceResponseHelper_h
#define WebResourceResponseHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo {
	template <class ImplClass>
	class WebResourceResponseHelper : private WebResourceResponse {
	public:
		inline WebResourceResponse* getWebResourceResponse() { return this; }
        
    protected:
        WebResourceResponseHelper();
		virtual ~WebResourceResponseHelper();

	private:
		static WebResourceResponseVTable const s_VTable;
		static ImplClass* getThis( WebResourceResponse* const pResponse ) { return static_cast<ImplClass*>(pResponse); }
		static const ImplClass* getThis( const WebResourceResponse* const pResponse ) { return static_cast<const ImplClass*>(pResponse); }
		
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAcquire(struct WebResourceResponse* pResponse);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sRelease(struct WebResourceResponse* pResponse);
        static WEBKIT_APOLLO_PROTO1 const unsigned char* WEBKIT_APOLLO_PROTO2 sGetURL(const struct WebResourceResponse* pResponse, unsigned long* numURLBytes);
        static WEBKIT_APOLLO_PROTO1 long WEBKIT_APOLLO_PROTO2 sGetStatusCode(const struct WebResourceResponse* pResponse);
        static WEBKIT_APOLLO_PROTO1 const WebResourceResponseHeader* WEBKIT_APOLLO_PROTO2 sGetHeaders(const struct WebResourceResponse*, unsigned long* numHeaders);
    };

	template <class ImplClass>
	WebResourceResponseVTable const WebResourceResponseHelper<ImplClass>::s_VTable = {
		sizeof(WebResourceResponseVTable),
		WebResourceResponseHelper<ImplClass>::sAcquire,
		WebResourceResponseHelper<ImplClass>::sRelease,
		WebResourceResponseHelper<ImplClass>::sGetURL,
		WebResourceResponseHelper<ImplClass>::sGetStatusCode,
		WebResourceResponseHelper<ImplClass>::sGetHeaders
	};
    
    template<class ImplClass>
    WebResourceResponseHelper<ImplClass>::WebResourceResponseHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
	WebResourceResponseHelper<ImplClass>::~WebResourceResponseHelper()
	{
        m_pVTable = 0;
	}
    
    template <class ImplClass>
    void WebResourceResponseHelper<ImplClass>::sAcquire(struct WebResourceResponse* pResponse)
    {
        ImplClass* const pThis = getThis(pResponse);
        pThis->acquire();
    }
    
    template <class ImplClass>
	void WebResourceResponseHelper<ImplClass>::sRelease(struct WebResourceResponse* pResponse)
    {
        ImplClass* const pThis = getThis(pResponse);
        pThis->release();
    }
    
    template <class ImplClass>
	const unsigned char* WebResourceResponseHelper<ImplClass>::sGetURL(const struct WebResourceResponse* pResponse, unsigned long* numURLBytes)
    {
        const ImplClass* const pThis = getThis(pResponse);
        return pThis->getURL(numURLBytes);
    }
    
    template <class ImplClass>
	long WebResourceResponseHelper<ImplClass>::sGetStatusCode(const struct WebResourceResponse* pResponse)
    {
        const ImplClass* const pThis = getThis(pResponse);
        return pThis->getStatusCode();
    }
    
    template <class ImplClass>
	const WebResourceResponseHeader* WebResourceResponseHelper<ImplClass>::sGetHeaders(const struct WebResourceResponse* pResponse, unsigned long* numHeaders)
    {
        const ImplClass* const pThis = getThis(pResponse);
        return pThis->getHeaders(numHeaders);
    }
    
}
#endif

