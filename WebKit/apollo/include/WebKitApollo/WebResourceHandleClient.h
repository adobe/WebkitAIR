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
 
#ifndef WebResourceHandleClient_h
#define WebResourceHandleClient_h

#include <WebKitApollo/WebConfig.h>

struct WebResourceHandleClient;

typedef WEBKIT_APOLLO_PROTO1 WebResourceRequest* ( WEBKIT_APOLLO_PROTO2 *WebResourceHandleClientReceivedRedirectFunction )( struct WebResourceHandleClient* resourceHandleClient
                                                                                                                          , struct WebResourceRequest* proposedNewRequest
                                                                                                                          , struct WebResourceResponse* redirectResponse );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceHandleClientReceivedResponseFunction )( struct WebResourceHandleClient* pURLRequest
                                                                                                           , struct WebResourceResponse* response );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceHandleClientReceivedDataFunction )( struct WebResourceHandleClient* resourceHandleClient
                                                                                                       , const unsigned char* pData
                                                                                                       , unsigned long numDataBytes );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceHandleClientReceivedRawDataURLBufferFunction )( struct WebResourceHandleClient* resourceHandleClient
                                                                                                        , const unsigned char* pData
                                                                                                        , unsigned long numDataBytes
                                                                                                        , bool isBase64Encoded );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceHandleClientReceivedAllDataFunction )( struct WebResourceHandleClient* resourceHandleClient );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceHandleClientDidFail )( struct WebResourceHandleClient* resourceHandleClient
                                                                                          , const unsigned char* failingURLBytes
                                                                                          , unsigned long numFailingURLBytes ); 
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceHandleClientReleaseFunction )( struct WebResourceHandleClient* resourceHandleClient );
                                                                                                                                                        

struct WebResourceHandleClientVTable
{
    unsigned long m_vTableSize;
    WebResourceHandleClientReceivedRedirectFunction receivedRedirect;
    WebResourceHandleClientReceivedResponseFunction receivedResponse;
    WebResourceHandleClientReceivedDataFunction receivedData;
    WebResourceHandleClientReceivedRawDataURLBufferFunction receivedRawDataURLBuffer;
    WebResourceHandleClientReceivedAllDataFunction receivedAllData;
    WebResourceHandleClientDidFail didFail;
    WebResourceHandleClientReleaseFunction release;
};

struct WebResourceHandleClient
{
    const struct WebResourceHandleClientVTable* m_pVTable;
};

#endif


