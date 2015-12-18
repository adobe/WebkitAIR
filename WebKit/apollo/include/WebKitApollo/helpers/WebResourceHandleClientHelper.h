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

#ifndef WebResourceHandleClientHelper_h
#define WebResourceHandleClientHelper_h

#include <WebKitApollo/WebKit.h>

struct WebResourceRequest;

namespace WebKitApollo
{
	template <class ImplClass>
	class WebResourceHandleClientHelper : private WebResourceHandleClient
	{
	public:
		inline WebResourceHandleClient* getWebResourceHandleClient() { return this; }
	protected:
		WebResourceHandleClientHelper();
		virtual ~WebResourceHandleClientHelper();
	private:
		static WebResourceHandleClientVTable const s_VTable;
		static ImplClass* getThis(WebResourceHandleClient* const pResourceLoaderClient) { return static_cast<ImplClass*>(pResourceLoaderClient); }
		
		static WEBKIT_APOLLO_PROTO1 WebResourceRequest* WEBKIT_APOLLO_PROTO2 sReceivedRedirect(struct WebResourceHandleClient* resourceHandleClient, struct WebResourceRequest* proposedNewRequest, struct WebResourceResponse* redirectResponse);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sReceivedResponse(struct WebResourceHandleClient* resourceHandleClient, struct WebResourceResponse* response);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sReceivedData(struct WebResourceHandleClient* resourceHandleClient, const unsigned char* pData, unsigned long numDataBytes);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sReceivedRawDataURLBuffer(struct WebResourceHandleClient* resourceHandleClient, const unsigned char* pData, unsigned long numDataBytes, bool isBase64Encoded);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sReceivedAllData(struct WebResourceHandleClient* resourceHandleClient);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDidFail(struct WebResourceHandleClient* resourceHandleClient
                                                                      , const unsigned char* failingURLBytes
                                                                      , unsigned long numFailingURLBytes);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sRelease(struct WebResourceHandleClient* pResourceLoaderClient);
	};
	
	template <class ImplClass>
	WebResourceHandleClientVTable const WebResourceHandleClientHelper<ImplClass>::s_VTable =
	{
		sizeof( WebResourceHandleClientVTable ),
		WebResourceHandleClientHelper<ImplClass>::sReceivedRedirect,
		WebResourceHandleClientHelper<ImplClass>::sReceivedResponse,
		WebResourceHandleClientHelper<ImplClass>::sReceivedData,
        WebResourceHandleClientHelper<ImplClass>::sReceivedRawDataURLBuffer,
		WebResourceHandleClientHelper<ImplClass>::sReceivedAllData,
		WebResourceHandleClientHelper<ImplClass>::sDidFail,
		WebResourceHandleClientHelper<ImplClass>::sRelease
	};
	
	template <class ImplClass>
	WebResourceHandleClientHelper<ImplClass>::~WebResourceHandleClientHelper()
	{
		m_pVTable = 0;
	}
	
	template <class ImplClass>
	WebResourceHandleClientHelper<ImplClass>::WebResourceHandleClientHelper()
	{
		m_pVTable = &s_VTable;
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
    WebResourceRequest* WEBKIT_APOLLO_PROTO2 WebResourceHandleClientHelper<ImplClass>::sReceivedRedirect(struct WebResourceHandleClient* resourceHandleClient, struct WebResourceRequest* proposedNewRequest, struct WebResourceResponse* redirectResponse)
	{
		ImplClass* const pThis = getThis(resourceHandleClient);
		WebResourceRequest* const pNewRequest = pThis->receivedRedirect(proposedNewRequest, redirectResponse);
        return pNewRequest;
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleClientHelper<ImplClass>::sReceivedResponse(struct WebResourceHandleClient* resourceHandleClient,struct WebResourceResponse* response)
	{
		ImplClass* const pThis = getThis(resourceHandleClient);
		pThis->receivedResponse(response);
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleClientHelper<ImplClass>::sReceivedData(struct WebResourceHandleClient* resourceHandleClient, const unsigned char* pData, unsigned long numDataBytes)
	{
		ImplClass* const pThis = getThis(resourceHandleClient);
		pThis->receivedData(pData, numDataBytes);
	}

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebResourceHandleClientHelper<ImplClass>::sReceivedRawDataURLBuffer(struct WebResourceHandleClient* resourceHandleClient, const unsigned char* pData, unsigned long numDataBytes, bool isBase64Encoded)
    {
        ImplClass* const pThis = getThis(resourceHandleClient);
        pThis->receivedRawDataURLBuffer(pData, numDataBytes, isBase64Encoded);
    }
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleClientHelper<ImplClass>::sReceivedAllData(struct WebResourceHandleClient* resourceHandleClient)
	{
		ImplClass* const pThis = getThis(resourceHandleClient);
		pThis->receivedAllData();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleClientHelper<ImplClass>::sDidFail(struct WebResourceHandleClient* resourceHandleClient
                                                                                , const unsigned char* failingURLBytes
                                                                                , unsigned long numFailingURLBytes)
	{
		ImplClass* const pThis = getThis(resourceHandleClient);
		pThis->didFail(failingURLBytes, numFailingURLBytes);
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleClientHelper<ImplClass>::sRelease(struct WebResourceHandleClient* resourceHandleClient)
	{
		ImplClass* const pThis = getThis(resourceHandleClient);
		pThis->release();
	}
	
}

#endif
