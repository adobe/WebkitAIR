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
#include "WebResourceHandleClientImpl.h"
#include "WebResourceRequestImpl.h"
#include <ResourceHandleClient.h>
#include <ResourceHandle.h>
#include <ResourceResponse.h>
#include <ResourceRequest.h>
#include <wtf/RefPtr.h>
#include <wtf/HashSet.h>

#include <wtf/Vector.h>
#include <Base64.h>

namespace WebKitApollo {

WebResourceHandleClientImpl::WebResourceHandleClientImpl(WebCore::ResourceHandle* pResourceHandle, WebCore::ResourceHandleClient* client)
    : m_resourceHandle(pResourceHandle)
    , m_client(client)
    , m_gotAllData(false)
{
    ASSERT(m_resourceHandle);
    ASSERT(m_client);
}

WebResourceHandleClientImpl::~WebResourceHandleClientImpl()
{
}

WebResourceRequest* WebResourceHandleClientImpl::receivedRedirect(struct WebResourceRequest* proposedNewWebRequest, struct WebResourceResponse* redirectWebResponse)
{
    WebCore::ResourceRequest newRequest(proposedNewWebRequest);
    WebCore::ResourceResponse redirectResponse(redirectWebResponse);
    ASSERT(m_client);
    m_client->willSendRequest(m_resourceHandle, newRequest, redirectResponse);
    WTF::RefPtr<WebResourceRequestImpl> newWebRequest(WebResourceRequestImpl::construct(newRequest));
    return newWebRequest.release().releaseRef()->getWebResourceRequest();
}

void WebResourceHandleClientImpl::receivedResponse(struct WebResourceResponse* webResponse)
{
    WebCore::ResourceResponse response(webResponse);
    ASSERT(m_client);
    ASSERT(m_resourceHandle);
    // The fragment part of the URL (foo.html#fragment) is dropped in WebResourceRequestImpl constructor.
    // In some cases, like loading a HTML document, this is not a problem because the loader uses the original request url and not the one from the response
    // but in other cases, like loading a PDF file using something like http://example.com/test.pdf#toolbar=0, the url is taken from the response
    // and because of the missing fragment, it is impossible to configure the PDF reader.
    // This also applies for redirects (the behavior is consistent with FireFox, Chrome and IE, but not with Safari (for redirects only, 
    // but there is bug for it https://bugs.webkit.org/show_bug.cgi?id=24175)
    // Add the fragment only if the original request had one and the response request doesn't have one
    if (!m_resourceHandle->request().url().fragmentIdentifier().isEmpty() && response.url().fragmentIdentifier().isEmpty()) {
        WebCore::KURL responseURL = response.url();
        responseURL.setFragmentIdentifier(m_resourceHandle->request().url().fragmentIdentifier());
        response.setURL(responseURL);
    }
    m_client->didReceiveResponse(m_resourceHandle, response);
}

// imported from KURL.cpp, with the only change that we do not use the encoding and use char type instead of UChar

static inline char hexDigitValue(char c)
{
    ASSERT(isASCIIHexDigit(c));
    if (c < 'A')
        return c - '0';
    return (c - 'A' + 10) & 0xF; // handle both upper and lower case without a branch
}

void decodeURLEscapeSequencesVector(const char* pData, size_t length, WTF::Vector<char>& result);
void decodeURLEscapeSequencesVector(const char* pData, size_t length, WTF::Vector<char>& result)
{
    if (pData == NULL || length == 0)
        // nothing to do here
        return;
    
    WTF::Vector<char> buffer;

    const char* decodedPosition = pData;
    const char* bufferEnd = pData + length;
    const char* searchPosition = pData;
    const char* encodedRunPosition = NULL;

    // search the first '%' in the buffer
    while (searchPosition < bufferEnd // if we didn't reach the end of the buffer yet
            && (encodedRunPosition = (const char*)memchr((const void*)searchPosition, '%', bufferEnd - searchPosition)) != NULL) {
        // Find the sequence of %-escape codes.
        ASSERT(encodedRunPosition < bufferEnd);
        
        const char* encodedRunEnd = encodedRunPosition;
        // Check that we have enough space to hold '%' and two hex digits
        // try to match all the occurrences here
        while ((bufferEnd - encodedRunEnd) >= 3
            && encodedRunEnd[0] == '%'
            && isASCIIHexDigit(encodedRunEnd[1])
            && isASCIIHexDigit(encodedRunEnd[2]))
            encodedRunEnd += 3;
        
        ASSERT(encodedRunEnd <= bufferEnd);

        if (encodedRunEnd == encodedRunPosition) {
            // if we didn't see any '%' + two hex digits
            // just increment the search position and try again
            ++searchPosition;
            continue;
        }
        searchPosition = encodedRunEnd;

        // Build up the string with what we just skipped and what we just decoded.
        result.append(decodedPosition, encodedRunPosition - decodedPosition);

        // Decode the %-escapes into bytes.
        size_t runLength = (encodedRunEnd - encodedRunPosition) / 3;
        buffer.resize(runLength);
        char* p = buffer.data();
        const char* q = encodedRunPosition;
        for (size_t i = 0; i < runLength; ++i) {
            // hexDigitValue will always return a value in (0x0 - 0xF) range.
            *p++ = (hexDigitValue(q[1]) << 4) | hexDigitValue(q[2]);
            q += 3;
        }

        result.append(buffer.data(), p - buffer.data());

        // move the reading head after the %-escapes
        decodedPosition = encodedRunEnd;
    }
    
    // if we have anything left
    // append it now
    if (decodedPosition < bufferEnd)
        result.append(decodedPosition, bufferEnd - decodedPosition);
}

void WebResourceHandleClientImpl::receivedRawDataURLBuffer(const unsigned char* pData, unsigned long numDataBytes, bool isBase64Encoded)
{
    ASSERT(m_client);
    // pData should always be allocated if the numDataBytes != 0
    ASSERT(pData);
    
    if (pData == NULL || numDataBytes == 0)
        return;

    // decode the URI encoded buffer into a vector of chars
    WTF::Vector<char> urlDecodedBuffer;
    decodeURLEscapeSequencesVector((const char*)pData, numDataBytes, urlDecodedBuffer);
    if (!urlDecodedBuffer.size())
        return;

    if (isBase64Encoded) {
        Vector<char> base64DecodedBuffer;
        if (!WebCore::base64Decode(urlDecodedBuffer, base64DecodedBuffer))
            return;

        m_client->didReceiveData(m_resourceHandle, base64DecodedBuffer.data(), base64DecodedBuffer.size(), 
                                        base64DecodedBuffer.size());
    
    } else {
        m_client->didReceiveData(m_resourceHandle, urlDecodedBuffer.data(), urlDecodedBuffer.size(), urlDecodedBuffer.size());
    }
}

void WebResourceHandleClientImpl::receivedData(const unsigned char* pData, unsigned long numDataBytes)
{
    ASSERT(m_client);
    m_client->didReceiveData(m_resourceHandle, reinterpret_cast<const char *>(pData), numDataBytes, numDataBytes);
}

void WebResourceHandleClientImpl::receivedAllData()
{
    ASSERT(!m_gotAllData);
    m_gotAllData = true;
    m_client->didFinishLoading(m_resourceHandle);
}

void WebResourceHandleClientImpl::didFail(const unsigned char* failingURLBytes, unsigned long numFailingURLBytes)
{
    WebCore::String const failingURLStr(reinterpret_cast<const char*>(failingURLBytes), numFailingURLBytes);
    WebCore::String const emptryStr;
    WebCore::ResourceError const resourceError(emptryStr, 0, failingURLStr, emptryStr);
    m_client->didFail(m_resourceHandle, resourceError);
}

void WebResourceHandleClientImpl::release()
{
    delete this;
}


void WebResourceLoaderSynchronousClientImpl::loadResourceSynchronously(WebHost* const webHost, const WebCore::ResourceRequest& request, WebCore::ResourceError& error, WebCore::ResourceResponse& response, WTF::Vector< char >& bytes)
{
    WebCore::ResourceRequest currRequest(request);
    
    
    WTF::HashSet<WebCore::String> redirectSet;
    static const unsigned int maxRedirects = 12;
    bool done = false;
    
    unsigned int redirectCount = 0;
    while ((!done) && (redirectCount < maxRedirects) && (!redirectSet.contains(currRequest.url().string()))) {
        WTF::RefPtr<WebResourceRequestImpl> const pWebResourceRequestImpl(WebResourceRequestImpl::construct(currRequest));
        WebResourceRequest* const pWebResourceRequest = pWebResourceRequestImpl->getWebResourceRequest();
        
        bool gotRedirect = false;
        WebCore::ResourceRequest redirectRequest;
        WebResourceLoaderSynchronousClientImpl* const pWebResourceHandleClientImpl = new WebResourceLoaderSynchronousClientImpl(&response, &error, &bytes, &gotRedirect, &redirectRequest);
        ::WebResourceHandleClient* const pWebResourceHandleClient = pWebResourceHandleClientImpl->getWebResourceHandleClient();
        webHost->m_pVTable->loadResourceSynchronously(webHost, pWebResourceRequest, pWebResourceHandleClient);
        
        if (gotRedirect) {
            ++redirectCount;
            redirectSet.add(currRequest.url().string());
            currRequest = redirectRequest;
            error = WebCore::ResourceError();
        }
        else {
            done = true;
        }
    }
    if (!done) {
        ASSERT((redirectCount == maxRedirects) || (redirectSet.contains(currRequest.url().string())));
        WebCore::String const emptryStr;
        error = WebCore::ResourceError(emptryStr, 0, request.url().string(), emptryStr);
    }
    else {
        ASSERT((redirectCount < maxRedirects) && (!redirectSet.contains(currRequest.url().string())));
    }
}

WebResourceLoaderSynchronousClientImpl::WebResourceLoaderSynchronousClientImpl(WebCore::ResourceResponse* const response, WebCore::ResourceError* const error, WTF::Vector< char >* bytes, bool* const gotRedirect, WebCore::ResourceRequest* const redirectRequest)
    : m_response(response)
    , m_error(error)
    , m_bytes(bytes)
    , m_gotRedirect(gotRedirect)
    , m_redirectRequest(redirectRequest)
{
}

WebResourceLoaderSynchronousClientImpl::~WebResourceLoaderSynchronousClientImpl()
{
}

void WebResourceLoaderSynchronousClientImpl::receivedRawDataURLBuffer(const unsigned char* pData, unsigned long numDataBytes, bool isBase64Encoded)
{
    ASSERT(false);
    // we should never be here because we can't 
    // load sync resources with data urls
}

struct WebResourceRequest* WebResourceLoaderSynchronousClientImpl::receivedRedirect(struct WebResourceRequest* proposedNewWebRequest, struct WebResourceResponse*)
{
   ASSERT(proposedNewWebRequest);
   ASSERT(proposedNewWebRequest->m_pVTable);
   ASSERT(proposedNewWebRequest->m_pVTable->acquire);
   *m_redirectRequest = WebCore::ResourceRequest(proposedNewWebRequest);
   *m_gotRedirect = true;
   return 0;
}

void WebResourceLoaderSynchronousClientImpl::receivedResponse(struct WebResourceResponse* webResponse)
{
    m_response->set(webResponse);
}

void WebResourceLoaderSynchronousClientImpl::receivedData( const unsigned char* pData, unsigned long numDataBytes )
{
    if ( numDataBytes > 0 )
    {
        size_t const currBufferSize = m_bytes->size();
        size_t const newBufferSize = currBufferSize + numDataBytes;
        ASSERT( currBufferSize < newBufferSize );
        m_bytes->resize( newBufferSize );
        char* const pBuffer = m_bytes->data();
        char* const pDest = pBuffer + currBufferSize;
        memcpy(pDest, pData, numDataBytes);
    }
}

void WebResourceLoaderSynchronousClientImpl::receivedAllData()
{
}

void WebResourceLoaderSynchronousClientImpl::didFail(const unsigned char* failingURLBytes, unsigned long numFailingURLBytes)
{
    WebCore::String const failingURLStr(reinterpret_cast<const char*>(failingURLBytes), numFailingURLBytes);
    WebCore::String const emptryStr;
    *m_error = WebCore::ResourceError(emptryStr, 0, failingURLStr, emptryStr);
}

void WebResourceLoaderSynchronousClientImpl::release()
{
    delete this;
}
}
