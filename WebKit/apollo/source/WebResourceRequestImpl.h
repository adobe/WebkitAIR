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
#ifndef WebResourceRequestImpl_h
#define WebResourceRequestImpl_h

#include <WebKitApollo/WebKit.h>
#include <WebKitApollo/helpers/WebResourceRequestHelper.h>

#include <wtf/Platform.h>
#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif

#include <wtf/Vector.h>
#include <RefCounted.h>
#include <wtf/RefPtr.h>
#include <PlatformString.h>
#include <wtf/text/CString.h>

namespace WebCore {
    class ResourceRequest;
}

namespace WebKitApollo {
	class WebResourceRequestImpl : public WebResourceRequestHelper<WebResourceRequestImpl>, public WTF::RefCounted<WebResourceRequestImpl>
	{
		friend class WebResourceRequestHelper< WebResourceRequestImpl >;
        friend class WTF::RefCounted<WebResourceRequestImpl>;
	public:
        static WTF::PassRefPtr<WebResourceRequestImpl> construct(const WebCore::ResourceRequest& resourceRequest);
	private:
        WebResourceRequestImpl(const WebCore::ResourceRequest& resourceRequest);
		virtual ~WebResourceRequestImpl( );
        
        void acquire();
        void release();
		const unsigned char* getURL(unsigned long* pURLLength) const;
		const unsigned char* getMethod(unsigned long* pMethodLength) const;
		const void* getPostData(unsigned long* pPostDataLength) const;
		const WebResourceRequestHeader* getHeaders(unsigned long* pNumHeaders) const;
		const unsigned char* getUserAgent(unsigned long* pUserAgentLength) const;
        const uint16_t* getUserName(unsigned long* pNumUserNameCodeUnits) const;
        const uint16_t* getCredential(unsigned long* pNumCredentialCodeUnits) const;
        bool getCanLoadDataURL() const { return m_canLoadDataURL; }
        
		WebResourceRequestImpl(const WebResourceRequestImpl& other);
		WebResourceRequestImpl& operator=(const WebResourceRequestImpl& other);
	
		WTF::CString const m_method;
		WTF::CString m_urlStr;
		WTF::Vector<char> m_bytesToPost;

		unsigned char* m_memoryForHeaders;
		WebResourceRequestHeader* m_pHeaders;
		unsigned long m_numHeaders;

		WTF::CString const m_userAgent;
        
        WebCore::String const m_userName;
        WebCore::String const m_credential;
        bool m_canLoadDataURL;
	};
}


#endif
