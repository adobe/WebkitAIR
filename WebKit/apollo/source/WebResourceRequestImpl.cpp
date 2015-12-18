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
#include "WebResourceRequestImpl.h"

#include <ResourceRequest.h>
#include <apollo/FormDataSerializer.h>

namespace WebKitApollo {

WTF::PassRefPtr<WebResourceRequestImpl> WebResourceRequestImpl::construct(const WebCore::ResourceRequest& resourceRequest)
{
    return adoptRef(new WebResourceRequestImpl(resourceRequest));
}

WebResourceRequestImpl::WebResourceRequestImpl(const WebCore::ResourceRequest& resourceRequest)
    : m_method(resourceRequest.httpMethod().latin1())
    , m_memoryForHeaders(0)
    , m_pHeaders(0)
    , m_numHeaders(0)
    , m_userAgent(resourceRequest.httpUserAgent().latin1())
    , m_userName(resourceRequest.url().user())
    , m_credential(resourceRequest.url().pass())
    , m_canLoadDataURL(resourceRequest.canLoadDataURL())
{
    WebCore::KURL requestURL(resourceRequest.url());
    requestURL.setPass(WebCore::String());
    requestURL.setUser(WebCore::String());
    
    // Should we be removing the fragment section of the URL? The windows implementation does.
    //
    requestURL.removeFragmentIdentifier();
    
    WebCore::String urlStr(requestURL.string());

    // Should we be using ascii or latin1? The windows code uses 
    // one in one case and the other in another case.
    //
    m_urlStr = urlStr.latin1();
    WebCore::FormData* const pFormPostData = resourceRequest.httpBody();
    if (pFormPostData)
        WebCore::FormDataSerializer(*pFormPostData).serialize(m_bytesToPost);
    
    const WebCore::HTTPHeaderMap& headersMap = resourceRequest.httpHeaderFields();
    m_numHeaders = headersMap.size();
    if (m_numHeaders > 0) {
        
        size_t numBytesNeeded = sizeof(WebResourceRequestHeader) * m_numHeaders;
        
        // WebKitApollo API uses uint16_t for UTF-16 code units, WebCore uses UChar so we have to ASSERT
        // that they are really the same size.
        ASSERT(sizeof(uint16_t) == sizeof(UChar));
        WebCore::HTTPHeaderMap::const_iterator const pEndHeaderEntry(headersMap.end());
        WebCore::HTTPHeaderMap::const_iterator pCurrHeaderEntry(headersMap.begin());
        while (pCurrHeaderEntry != pEndHeaderEntry) {
            numBytesNeeded += pCurrHeaderEntry->first.length() * sizeof(UChar);
            numBytesNeeded += pCurrHeaderEntry->second.length() * sizeof(UChar);
            ++pCurrHeaderEntry;
        }
        m_memoryForHeaders = new unsigned char[numBytesNeeded];
        m_pHeaders = reinterpret_cast<WebResourceRequestHeader*>(m_memoryForHeaders);
        uint16_t* pCurrStringDest = reinterpret_cast<uint16_t*>(m_memoryForHeaders + sizeof(WebResourceRequestHeader) * m_numHeaders);
        
        //Only used in ASSERTs.
        uint16_t* const pEndOfBuffer = reinterpret_cast<uint16_t*>(m_memoryForHeaders + numBytesNeeded);
        //Need the following line to get rid of unused variable warnings in release builds.
        (void)pEndOfBuffer;

        pCurrHeaderEntry = headersMap.begin();
        for (int i = 0; i < headersMap.size(); ++i) {
            ASSERT(pCurrHeaderEntry != pEndHeaderEntry);
            const WebCore::String& headerName = pCurrHeaderEntry->first;
            m_pHeaders[i].m_numHeaderNameCodeUnits = headerName.length();
            uint16_t* const pHeaderNameDest = pCurrStringDest;
            m_pHeaders[i].m_pUTF16HeaderName = pHeaderNameDest;
            
            pCurrStringDest += m_pHeaders[i].m_numHeaderNameCodeUnits;
            
            ASSERT(pCurrStringDest <= pEndOfBuffer);
            memcpy(pHeaderNameDest, headerName.characters(), m_pHeaders[i].m_numHeaderNameCodeUnits * sizeof(uint16_t));
            
            const WebCore::String& headerValue = pCurrHeaderEntry->second;
            m_pHeaders[i].m_numHeaderValueCodeUnits = headerValue.length();
            uint16_t* const pHeaderValueDest = pCurrStringDest;
            m_pHeaders[i].m_pUTF16HeaderValue = pHeaderValueDest;

            pCurrStringDest += m_pHeaders[i].m_numHeaderValueCodeUnits;

            ASSERT(pCurrStringDest <= pEndOfBuffer);
            memcpy(pHeaderValueDest, headerValue.characters(), m_pHeaders[i].m_numHeaderValueCodeUnits * sizeof(uint16_t));
            ++pCurrHeaderEntry;
        }
    }
    
    //If we have a credential, we must have a user name.
    ASSERT((!m_credential.length()) || (m_userName.length()));
}

WebResourceRequestImpl::~WebResourceRequestImpl()
{
    if (m_memoryForHeaders)
        delete [] m_memoryForHeaders;
}

void WebResourceRequestImpl::acquire()
{
    ref();
}

void WebResourceRequestImpl::release()
{
    deref();
}

const unsigned char* WebResourceRequestImpl::getURL(unsigned long* pURLLength) const
{
    *pURLLength = m_urlStr.length();
    return reinterpret_cast<const unsigned char *>(m_urlStr.data());
}

const unsigned char* WebResourceRequestImpl::getMethod(unsigned long* pMethodLength) const
{
    *pMethodLength = m_method.length();
    return reinterpret_cast<const unsigned char *>(m_method.data());
}

const void* WebResourceRequestImpl::getPostData(unsigned long* pPostDataLength) const
{
    ASSERT(pPostDataLength);
    *pPostDataLength = static_cast<unsigned long>(m_bytesToPost.size());
    return m_bytesToPost.data();
}

const WebResourceRequestHeader* WebResourceRequestImpl::getHeaders(unsigned long* pNumHeaders) const
{
    ASSERT(pNumHeaders);
    *pNumHeaders = m_numHeaders;
    return m_pHeaders;
}

const unsigned char* WebResourceRequestImpl::getUserAgent(unsigned long* pUserAgentLength) const
{
    (*pUserAgentLength) = m_userAgent.length();
    return reinterpret_cast<const unsigned char *>(m_userAgent.data());
}


const uint16_t* WebResourceRequestImpl::getUserName(unsigned long* pNumUserNameCodeUnits) const
{
    ASSERT(sizeof(uint16_t) == sizeof(UChar));
    ASSERT(pNumUserNameCodeUnits);
    //If we have a credential, we must have a user name.
    ASSERT((!m_credential.length()) || (m_userName.length()));
    const uint16_t* userName = 0;
    *pNumUserNameCodeUnits = m_userName.length();
    if (*pNumUserNameCodeUnits)
        userName = reinterpret_cast<const uint16_t*>(m_userName.characters());
    return userName;
}

const uint16_t* WebResourceRequestImpl::getCredential(unsigned long* pNumCredentialCodeUnits) const
{
    ASSERT(sizeof(uint16_t) == sizeof(UChar));
    ASSERT(pNumCredentialCodeUnits);
    //If we have a credential, we must have a user name.
    ASSERT((!m_credential.length()) || (m_userName.length()));
    const uint16_t* credential = 0;
    *pNumCredentialCodeUnits = m_credential.length();
    if (*pNumCredentialCodeUnits)
        credential = reinterpret_cast<const uint16_t*>(m_credential.characters());
    return credential;
}

}
