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
 
#ifndef WebResourceResponse_h
#define WebResourceResponse_h

#include <WebKitApollo/WebConfig.h>

struct WebResourceResponse;

struct WebResourceResponseHeader {
    const uint16_t* m_pUTF16HeaderName;
    unsigned long m_numHeaderNameCodeUnits;
    const uint16_t* m_pUTF16HeaderValue;
    unsigned long m_numHeaderValueCodeUnits;
};

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceResponseAcquireFunction )( struct WebResourceResponse* );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebResourceResponseReleaseFunction )( struct WebResourceResponse* );
typedef WEBKIT_APOLLO_PROTO1 const unsigned char* ( WEBKIT_APOLLO_PROTO2 *WebResourceResponseGetURLFunction )( const struct WebResourceResponse*, unsigned long* numURLBytes );
typedef WEBKIT_APOLLO_PROTO1 long ( WEBKIT_APOLLO_PROTO2 *WebResourceResponseGetStatusCodeFunction )( const struct WebResourceResponse* );
typedef WEBKIT_APOLLO_PROTO1 const struct WebResourceResponseHeader* ( WEBKIT_APOLLO_PROTO2 *WebResourceResponseGetHeadersFunction )( const struct WebResourceResponse*, unsigned long* numHeaders );

struct WebResourceResponseVTable {
    unsigned long m_vTableSize;
    WebResourceResponseAcquireFunction acquire;
    WebResourceResponseReleaseFunction release;
    WebResourceResponseGetURLFunction getURL;
    WebResourceResponseGetStatusCodeFunction getStatusCode;
    WebResourceResponseGetHeadersFunction getHeaders;
};

struct WebResourceResponse {
    const struct WebResourceResponseVTable* m_pVTable;
};

#endif
