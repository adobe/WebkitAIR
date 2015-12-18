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
#ifndef WebRequestHelper_h
#define WebRequestHelper_h

#include <WebKitApollo/WebKit.h>

#include <stddef.h>

namespace WebKitApollo
{
	template <class ImplClass>
	class WebResourceRequestHelper : private WebResourceRequest {

	public:
	
		inline WebResourceRequest* getWebResourceRequest() { return this; }
		
    protected:
        WebResourceRequestHelper();
        virtual ~WebResourceRequestHelper();
        
	private:
		static WebResourceRequestVTable const s_VTable;
		static ImplClass* getThis(WebResourceRequest* const pResourceRequest) { return static_cast<ImplClass*>(pResourceRequest); }
		static const ImplClass* getThis(const WebResourceRequest* const pResourceRequest) { return static_cast<const ImplClass*>(pResourceRequest); }
        
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAcquire(struct WebResourceRequest* pURLRequest);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sRelease(struct WebResourceRequest* pURLRequest);
		static WEBKIT_APOLLO_PROTO1 const unsigned char* WEBKIT_APOLLO_PROTO2 sGetURL(const struct WebResourceRequest* pURLRequest, unsigned long* pURLLength);
		static WEBKIT_APOLLO_PROTO1 const unsigned char* WEBKIT_APOLLO_PROTO2 sGetMethod(const struct WebResourceRequest* pURLRequest, unsigned long* pMethodLength);
		static WEBKIT_APOLLO_PROTO1 const void* WEBKIT_APOLLO_PROTO2 sGetPostData(const struct WebResourceRequest* pURLRequest, unsigned long* pPostDataLength);
		static WEBKIT_APOLLO_PROTO1 const WebResourceRequestHeader* WEBKIT_APOLLO_PROTO2 sGetHeaders(const struct WebResourceRequest* pURLRequest, unsigned long* pNumHeaders);
		static WEBKIT_APOLLO_PROTO1 const unsigned char* WEBKIT_APOLLO_PROTO2 sGetUserAgent(const struct WebResourceRequest* pURLRequest, unsigned long* pUserAgentLength);
        static WEBKIT_APOLLO_PROTO1 const uint16_t* sGetUserName(const struct WebResourceRequest*, unsigned long* pNumUserNameCodeUnits);
        static WEBKIT_APOLLO_PROTO1 const uint16_t* sGetCredential(const struct WebResourceRequest*, unsigned long* pNumCredentialCodeUnits);
        static WEBKIT_APOLLO_PROTO1 bool sGetCanLoadDataURL(const struct WebResourceRequest*);
	};

	template <class ImplClass>
	WebResourceRequestVTable const WebResourceRequestHelper<ImplClass>::s_VTable =
	{
		sizeof(WebResourceRequestVTable),
		WebResourceRequestHelper<ImplClass>::sAcquire,
		WebResourceRequestHelper<ImplClass>::sRelease,
		WebResourceRequestHelper<ImplClass>::sGetURL,
		WebResourceRequestHelper<ImplClass>::sGetMethod,
		WebResourceRequestHelper<ImplClass>::sGetPostData,
		WebResourceRequestHelper<ImplClass>::sGetHeaders,
		WebResourceRequestHelper<ImplClass>::sGetUserAgent,
		WebResourceRequestHelper<ImplClass>::sGetUserName,
		WebResourceRequestHelper<ImplClass>::sGetCredential,
        WebResourceRequestHelper<ImplClass>::sGetCanLoadDataURL
	};

    template<class ImplClass>
    WebResourceRequestHelper<ImplClass>::WebResourceRequestHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
	WebResourceRequestHelper<ImplClass>::~WebResourceRequestHelper()
	{
        m_pVTable = 0;
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sAcquire(struct WebResourceRequest* pURLRequest)
	{
		ImplClass* const pThis = getThis(pURLRequest);
        pThis->acquire();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sRelease(struct WebResourceRequest* pURLRequest)
	{
		ImplClass* const pThis = getThis(pURLRequest);
		pThis->release();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const unsigned char* WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetURL(const struct WebResourceRequest* pURLRequest, unsigned long* pURLLength)
	{
		const ImplClass* const pThis = getThis(pURLRequest);
		return pThis->getURL(pURLLength);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const unsigned char* WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetMethod(const struct WebResourceRequest* pURLRequest, unsigned long* pMethodLength)
	{
		const ImplClass* const pThis = getThis(pURLRequest);
		return pThis->getMethod(pMethodLength);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const void* WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetPostData(const struct WebResourceRequest* pURLRequest, unsigned long* pPostDataLength)
	{
		const ImplClass* const pThis = getThis(pURLRequest);
		return pThis->getPostData(pPostDataLength);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const WebResourceRequestHeader* WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetHeaders(const struct WebResourceRequest* pURLRequest, unsigned long* pNumHeaders)
	{
		const ImplClass* const pThis = getThis(pURLRequest);
		return pThis->getHeaders(pNumHeaders);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const unsigned char* WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetUserAgent(const struct WebResourceRequest* pURLRequest, unsigned long* pUserAgentLength)
	{
		const ImplClass* const pThis = getThis(pURLRequest);
		return pThis->getUserAgent(pUserAgentLength);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const uint16_t* WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetUserName(const struct WebResourceRequest* pURLRequest, unsigned long* pNumUserNameCodeUnits)
	{
		const ImplClass* const pThis = getThis(pURLRequest);
		return pThis->getUserName(pNumUserNameCodeUnits);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const uint16_t* WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetCredential(const struct WebResourceRequest* pURLRequest, unsigned long* pNumCredentialCodeUnits)
	{
		const ImplClass* const pThis = getThis(pURLRequest);
		return pThis->getCredential(pNumCredentialCodeUnits);
	}

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    bool WEBKIT_APOLLO_PROTO2 WebResourceRequestHelper<ImplClass>::sGetCanLoadDataURL(const struct WebResourceRequest* pURLRequest)
    {
        const ImplClass* const pThis = getThis(pURLRequest);
        return pThis->getCanLoadDataURL();
    }

}
#endif
