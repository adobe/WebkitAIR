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

#include <config.h>
#include <ResourceRequest.h>
#include <WebKitApollo/WebKit.h>

namespace WebCore {

ResourceRequest::ResourceRequest(const WebResourceRequest* const pWebResourceRequest)
{
    ASSERT(pWebResourceRequest);
    ASSERT(pWebResourceRequest->m_pVTable);
    ASSERT(pWebResourceRequest->m_pVTable->acquire);
    ASSERT(pWebResourceRequest->m_pVTable->release);
    ASSERT(pWebResourceRequest->m_pVTable->getURL);
    ASSERT(pWebResourceRequest->m_pVTable->getMethod);
    ASSERT(pWebResourceRequest->m_pVTable->getPostData);
    ASSERT(pWebResourceRequest->m_pVTable->getHeaders);
    ASSERT(pWebResourceRequest->m_pVTable->getUserAgent);
    ASSERT(pWebResourceRequest->m_pVTable->getUserName);
    ASSERT(pWebResourceRequest->m_pVTable->getCredential);
    ASSERT(pWebResourceRequest->m_pVTable->getCanLoadDataURL);
    
    {
        unsigned long numURLBytes = 0;
        const unsigned char* const pURLBytes = pWebResourceRequest->m_pVTable->getURL(pWebResourceRequest, &numURLBytes);
        ASSERT(pURLBytes);
        ASSERT(numURLBytes > 0);
    
        String const urlStr(String::fromUTF8(reinterpret_cast<const char*>(pURLBytes), numURLBytes));
        // KURL's constructor does some evil things if the first character is a '/'.
        // We should never see relative URLs here anyway.
        ASSERT(urlStr[0] != '/');
        m_url = KURL(KURL(), urlStr);
        ASSERT(m_url.isValid());
    }
    
    {
        unsigned long numMethodBytes = 0;
        const unsigned char* const pMethodBytes = pWebResourceRequest->m_pVTable->getMethod(pWebResourceRequest, &numMethodBytes);
        ASSERT(pMethodBytes);
        ASSERT(numMethodBytes > 0);
        
        m_httpMethod = String(reinterpret_cast<const char*>(pMethodBytes), numMethodBytes);
    }
    
    {
        unsigned long numPostBytes = 0;
        const void* const pPostBytes = pWebResourceRequest->m_pVTable->getPostData(pWebResourceRequest, &numPostBytes);
        if (numPostBytes > 0) {
            ASSERT(pPostBytes);
            m_httpBody = FormData::create(pPostBytes, numPostBytes);
        }
    }
    
    {
        unsigned long numRequestHeaders = 0;
        const WebResourceRequestHeader* const pRequestHeaders = pWebResourceRequest->m_pVTable->getHeaders(pWebResourceRequest, &numRequestHeaders);
        if (numRequestHeaders) {
            ASSERT(pRequestHeaders);
            for (unsigned int i = 0; i < numRequestHeaders; ++i) {
                const WebResourceRequestHeader& currRequestHeader = *(pRequestHeaders + i);
                ASSERT(currRequestHeader.m_pUTF16HeaderName);
                ASSERT(currRequestHeader.m_pUTF16HeaderValue);
                ASSERT(sizeof(uint16_t) == sizeof(UChar));
                
                String const headerName(reinterpret_cast<const UChar*>(currRequestHeader.m_pUTF16HeaderName), currRequestHeader.m_numHeaderNameCodeUnits);
                String const headerValue(reinterpret_cast<const UChar*>(currRequestHeader.m_pUTF16HeaderValue), currRequestHeader.m_numHeaderValueCodeUnits);
                
                m_httpHeaderFields.set(headerName, headerValue);
            }
        }
    }
    
    {
        unsigned long numUserAgentBytes = 0;
        const unsigned char* const pUserAgentBytes = pWebResourceRequest->m_pVTable->getUserAgent(pWebResourceRequest, &numUserAgentBytes);
        if (numUserAgentBytes > 0) {
            ASSERT(pUserAgentBytes);
            ASSERT(numUserAgentBytes > 0);
            m_httpHeaderFields.set("User-Agent", String(reinterpret_cast<const char*>(pUserAgentBytes), numUserAgentBytes));
        }
    }
    
    {
        unsigned long numUserNameCodeUnits = 0;
        const uint16_t* const userNameUTF16 = pWebResourceRequest->m_pVTable->getUserName(pWebResourceRequest, &numUserNameCodeUnits);
        if(numUserNameCodeUnits) {
            ASSERT(userNameUTF16);
            ASSERT(sizeof(UChar) == sizeof(uint16_t));
            String const userName(reinterpret_cast<const UChar*>(userNameUTF16), numUserNameCodeUnits);
            m_url.setUser(userName);
        }
    }
    
    {
        unsigned long numCredentialCodeUnits = 0;
        const uint16_t* const credentialUTF16 = pWebResourceRequest->m_pVTable->getCredential(pWebResourceRequest, &numCredentialCodeUnits);
        if(numCredentialCodeUnits) {
            ASSERT(credentialUTF16);
            ASSERT(sizeof(UChar) == sizeof(uint16_t));
            String const credential(reinterpret_cast<const UChar*>(credentialUTF16), numCredentialCodeUnits);
            m_url.setPass(credential);
        }
    }

    m_canLoadDataURL = pWebResourceRequest->m_pVTable->getCanLoadDataURL(pWebResourceRequest);
}

}

