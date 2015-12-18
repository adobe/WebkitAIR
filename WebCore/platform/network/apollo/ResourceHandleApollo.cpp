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
#include <ResourceHandle.h>
#include <ResourceHandleClient.h>
#include <ResourceHandleInternal.h>
#include <FrameLoaderClientApollo.h>
#include <WebKitApollo/WebResourceHandle.h>

#include <Frame.h>
#include <FrameLoader.h>
#include <Document.h>

#include <SharedBuffer.h>

namespace WebCore {

ResourceHandleInternal::~ResourceHandleInternal()
{
    if (m_requestPending) {
        ASSERT(m_pWebResourceHandle->m_pVTable != NULL);
        ASSERT(m_pWebResourceHandle->m_pVTable->kill != NULL);
        ASSERT(m_pWebResourceHandle->m_pVTable->release != NULL);
        m_pClientWrapper->loaderDead();
        m_pWebResourceHandle->m_pVTable->kill(m_pWebResourceHandle);
    }
    if(m_pWebResourceHandle)
    {
        ASSERT(m_pWebResourceHandle->m_pVTable != NULL);
        ASSERT(m_pWebResourceHandle->m_pVTable->release != NULL);
        m_pWebResourceHandle->m_pVTable->release(m_pWebResourceHandle);
        m_pWebResourceHandle = 0;
    }
}


ResourceHandleClientWrapper::ResourceHandleClientWrapper(ResourceHandleInternal* pInternal, ResourceHandle* pResourceHandle, ResourceHandleClient* pRealClient)
    : m_pInternal( pInternal ),
      m_pResourceHandle( pResourceHandle ),
      m_pRealClient( pRealClient ),
      m_realClientNuked( false ),
      m_loaderIsDead( false )
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pInternal);
}

ResourceHandleClientWrapper::~ResourceHandleClientWrapper()
{
}

void ResourceHandleClientWrapper::willSendRequest(ResourceHandle* pResourceHandle, ResourceRequest& resourceRequestToSend, const ResourceResponse& redirectResponse)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->willSendRequest(pResourceHandle, resourceRequestToSend, redirectResponse);
}

void ResourceHandleClientWrapper::didReceiveResponse(ResourceHandle* pResourceHandle, const ResourceResponse& response)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    // If this was a mapped request, fix up the URL to the original value
    if (!m_preMappedURL.isEmpty()) {
        ResourceResponse newResp(response, m_preMappedURL);
        m_pRealClient->didReceiveResponse(pResourceHandle, newResp);
    }
    else {
        m_pRealClient->didReceiveResponse(pResourceHandle, response);
    }
}

void ResourceHandleClientWrapper::didReceiveData(ResourceHandle* pResourceHandle, const char* data, int dataSize, int lengthReceived)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->didReceiveData(pResourceHandle, data, dataSize, lengthReceived);
}

void ResourceHandleClientWrapper::didFinishLoading(ResourceHandle* pResourceHandle)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->didFinishLoading(pResourceHandle);
}

void ResourceHandleClientWrapper::didFail(ResourceHandle* pResourceHandle, const ResourceError& resouceError)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->didFail(pResourceHandle, resouceError);
}

void ResourceHandleClientWrapper::wasBlocked(ResourceHandle* pResourceHandle)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->wasBlocked(pResourceHandle);
}

void ResourceHandleClientWrapper::willCacheResponse(ResourceHandle* pResourceHandle, CacheStoragePolicy& cachePolicy)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->willCacheResponse(pResourceHandle, cachePolicy);
}

void ResourceHandleClientWrapper::didReceiveAuthenticationChallenge(ResourceHandle* pResourceHandle, const AuthenticationChallenge& authChallenge)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->didReceiveAuthenticationChallenge(pResourceHandle, authChallenge);
}

void ResourceHandleClientWrapper::didCancelAuthenticationChallenge(ResourceHandle* pResourceHandle, const AuthenticationChallenge& authChallenge)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->didCancelAuthenticationChallenge(pResourceHandle, authChallenge);
}

#ifndef NDEBUG    
void ResourceHandleClientWrapper::receivedCredential(ResourceHandle* pResourceHandle, const AuthenticationChallenge&, const Credential&)
#else
void ResourceHandleClientWrapper::receivedCredential(ResourceHandle*, const AuthenticationChallenge&, const Credential&)
#endif
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
// rhu implement ?
//    m_pRealClient->receivedCredential(pResourceHandle, authChallenge, credential);
}
    
#ifndef NDEBUG    
void ResourceHandleClientWrapper::receivedRequestToContinueWithoutCredential(ResourceHandle* pResourceHandle, const AuthenticationChallenge&)
#else
void ResourceHandleClientWrapper::receivedRequestToContinueWithoutCredential(ResourceHandle*, const AuthenticationChallenge&)
#endif
{

    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
// rhu implement ?
//    m_pRealClient->receivedRequestToContinueWithoutCredential(pResourceHandle, authChallenge);
}

void ResourceHandleClientWrapper::receivedCancellation(ResourceHandle* pResourceHandle, const AuthenticationChallenge& authChallenge)
{
    ASSERT(m_pRealClient);
    ASSERT(m_pResourceHandle);
    ASSERT(m_pResourceHandle == pResourceHandle);
    ASSERT(!m_realClientNuked); 
    m_pRealClient->receivedCancellation(pResourceHandle, authChallenge);
}


void ResourceHandleClientWrapper::loaderDead()
{
    ASSERT(!m_loaderIsDead);
    m_loaderIsDead = true;
}

ResourceHandle::~ResourceHandle()
{
}

bool ResourceHandle::start(Frame* frame)
{
    ASSERT(frame);
    if (!frame)
        return false;
    
    FrameLoaderClientApollo* const pFrameLoaderClientApollo = FrameLoaderClientApollo::clientApollo(frame);
    ASSERT(pFrameLoaderClientApollo);

    bool result = false;
    
    ASSERT( d->m_pClientWrapper.get() == NULL );
    d->m_pClientWrapper = std::auto_ptr<ResourceHandleClientWrapper>(new ResourceHandleClientWrapper(d.get(), this, d->m_client));

    ResourceRequest mappedRequest = d->m_request;
    KURL mappedURL;
    if (FrameLoaderClientApollo::mapFrameUrl(frame, d->m_request.url(), &mappedURL)) {
        mappedRequest.setURL(mappedURL);
        d->m_pClientWrapper->setPreMappedURL(d->m_request.url());
    }

    WebResourceHandle* pWebResourceHandle = pFrameLoaderClientApollo->startResourceHandle(this, d->m_pClientWrapper.get(), mappedRequest);
    if (pWebResourceHandle)
    {
        d->m_requestPending = true;
        d->m_pWebResourceHandle = pWebResourceHandle;
        result = true;
    }
    return result;
}

void ResourceHandle::cancel()
{
    if (d->m_requestPending)
    {
        if (d->m_pWebResourceHandle)
        {
            ASSERT(d->m_pWebResourceHandle->m_pVTable);
            ASSERT(d->m_pWebResourceHandle->m_pVTable->cancel);
            d->m_requestPending = false;
            d->m_pWebResourceHandle->m_pVTable->cancel(d->m_pWebResourceHandle);
        }
    }
}

PassRefPtr<SharedBuffer> ResourceHandle::bufferedData()
{
    return 0;
}

void ResourceHandle::setDefersLoading(bool)
{
}

bool ResourceHandle::supportsBufferedData()
{
    return false;
}

bool ResourceHandle::loadsBlocked()
{
    return false;
}

bool ResourceHandle::willLoadFromCache(ResourceRequest&, Frame*)
{
    return false;
}

void ResourceHandle::loadResourceSynchronously(const ResourceRequest& request, StoredCredentials, ResourceError& error, ResourceResponse& response, Vector<char>& data, Frame* frame)
{
    ASSERT(frame);

    FrameLoaderClientApollo* const pFrameLoaderClientApollo(FrameLoaderClientApollo::clientApollo(frame));

    ASSERT(pFrameLoaderClientApollo);
    
    // Disallow network loads while parsing application content
    if (!pFrameLoaderClientApollo->isSynchronousRequestAllowed(request.url()))
    {
        error = ResourceError();
        return;
    }

    ResourceRequest mappedRequest = request;
    KURL mappedURL;
    bool wasMapped = FrameLoaderClientApollo::mapFrameUrl(frame, request.url(), &mappedURL);
    if (wasMapped)
        mappedRequest.setURL(mappedURL);

    pFrameLoaderClientApollo->loadResourceSynchronously(mappedRequest, error, response, data);

    // If this was a mapped request, set the response URL to what was originally requested.
    if (wasMapped)
        response.setURL(request.url());
}

#if 0

void ResourceLoader::executeSynchronously(  DocLoader* const pDocLoader,
                                            KURL& finalURL,
                                            String& responseHeadersString,
                                            WTF::Vector< char >& data )
{
    ASSERT( pDocLoader != NULL );
    Frame* pFrame = pDocLoader->frame();
    if ( pFrame != NULL )
    {
        FrameApollo* pFrameApollo = Apollo( pFrame );
        ASSERT( d->m_pClientWrapper.get() == NULL );
        
        ASSERT( pFrameApollo != NULL );
        pFrameApollo->loadResourceSynchronously( this, d->method, d->URL, d->postData, finalURL, responseHeadersString, data );
    }
    delete this;
}

class Loader;

Vector<char> ServeSynchronousRequest(   Loader* ,
                                        DocLoader* pDocLoader,
                                        ResourceLoader* pResourceHandle,
                                        KURL& finalURL,
                                        DeprecatedString& responseHeadersDeprecatedString )
{
    ASSERT( pResourceHandle != NULL );
    ASSERT( pDocLoader != NULL );
    WTF::Vector< char > data;
    String responseHeadersString;
    pResourceHandle->executeSynchronously( pDocLoader, finalURL, responseHeadersString, data );
    responseHeadersDeprecatedString = responseHeadersString.deprecatedString();
    return data;
}
#endif

} // namespace WebCore
