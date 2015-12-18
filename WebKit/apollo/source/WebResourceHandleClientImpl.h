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
#ifndef WebResourceHandleClientImpl_h
#define WebResourceHandleClientImpl_h

#include <WebKitApollo/WebKit.h>
#include <WebKitApollo/helpers/WebResourceHandleClientHelper.h>
#include <wtf/Platform.h>
#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif
#include <ResourceRequest.h>

#include <wtf/Vector.h>

namespace WebCore {
class ResourceHandle;
class ResourceHandleClient;
class ResourceError;
class ResourceResponse;

};

namespace WebKitApollo {

	class WebResourceHandleClientImpl : public WebResourceHandleClientHelper<WebResourceHandleClientImpl>
	{
		friend class WebResourceHandleClientHelper< WebResourceHandleClientImpl >;
	public:
		WebResourceHandleClientImpl(WebCore::ResourceHandle* pResourceHandle, WebCore::ResourceHandleClient* client);
		virtual ~WebResourceHandleClientImpl( );
		void loadStarted();
	private:
		WebResourceRequest* receivedRedirect(struct WebResourceRequest* proposedNewRequest, struct WebResourceResponse* redirectResponse);
		void receivedResponse(struct WebResourceResponse* response);
        void receivedRawDataURLBuffer(const unsigned char* pData, unsigned long numDataBytes, bool isBase64Encoded);
		void receivedData(const unsigned char* pData, unsigned long numDataBytes);
		void receivedAllData();
        void didFail(const unsigned char* failingURLBytes, unsigned long numFailingURLBytes);
		void release();

		WebResourceHandleClientImpl( const WebResourceHandleClientImpl& other );
		WebResourceHandleClientImpl& operator=( const WebResourceHandleClientImpl& other );
	
		WebCore::ResourceHandle* m_resourceHandle;
		WebCore::ResourceHandleClient* m_client;
		bool m_gotAllData;
	};

	class WebResourceLoaderSynchronousClientImpl : public WebResourceHandleClientHelper< WebResourceLoaderSynchronousClientImpl >
	{
		friend class WebResourceHandleClientHelper< WebResourceLoaderSynchronousClientImpl >;
    public:
        static void loadResourceSynchronously(WebHost* const webHost, const WebCore::ResourceRequest&, WebCore::ResourceError&, WebCore::ResourceResponse&, WTF::Vector< char >& bytes);
	protected:
        virtual ~WebResourceLoaderSynchronousClientImpl( );

	private:
        WebResourceLoaderSynchronousClientImpl(WebCore::ResourceResponse* const response, WebCore::ResourceError* const error, WTF::Vector< char >* bytes, bool* const gotRedirect, WebCore::ResourceRequest* const redirectRequest);
		
		WebResourceRequest* receivedRedirect(struct WebResourceRequest* proposedNewRequest, struct WebResourceResponse* redirectResponse);
        void receivedRawDataURLBuffer(const unsigned char* pData, unsigned long numDataBytes, bool isBase64Encoded);
		void receivedResponse(struct WebResourceResponse* response);
		void receivedData(const unsigned char* pData, unsigned long numDataBytes);
		void receivedAllData();
        void didFail(const unsigned char* failingURLBytes, unsigned long numFailingURLBytes);
		void release();

		WebResourceLoaderSynchronousClientImpl( const WebResourceLoaderSynchronousClientImpl& other );
		WebResourceLoaderSynchronousClientImpl& operator=( const WebResourceLoaderSynchronousClientImpl& other );

        WebCore::ResourceResponse* const m_response;
		WebCore::ResourceError* const m_error;
		WTF::Vector< char >* const m_bytes;
        bool* const m_gotRedirect;
        WebCore::ResourceRequest* const m_redirectRequest;
		
	};
}
#endif
