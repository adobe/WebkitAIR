/*
 * Copyright (C) 2009 Adobe Systems Incorporated.  All rights reserved.
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

#ifndef WebSamplerListener_h
#define WebSamplerListener_h

#include <WebKitApollo/WebKit.h>

struct WebSamplerListener;

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerListenerDidEnterFunction )( struct WebSamplerListener* pWebSamplerListener, uint64_t identifier, int32_t lineno );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerListenerDidExitFunction )( struct WebSamplerListener* pWebSamplerListener, uint64_t identifier );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerListenerAtStatement )( struct WebSamplerListener* pWebSamplerListener, int32_t lineno );

typedef WEBKIT_APOLLO_PROTO1 uint64_t ( WEBKIT_APOLLO_PROTO2 *WebSamplerListenerDidAllocateObject )( struct WebSamplerListener* pWebSamplerListener, const void* ptr, size_t size);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerListenerDidDeallocateObject )( struct WebSamplerListener* pWebSamplerListener, const void* ptr, size_t size);

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerListenerSetWebSamplerRef )( struct WebSamplerListener* pWebSamplerListener, WebSampler **pWebSamplerRef );


struct WebSamplerListenerVTable {
	unsigned long m_vTableSize;
    WebSamplerListenerDidEnterFunction didEnterFunction;
    WebSamplerListenerDidExitFunction didExitFunction;
    WebSamplerListenerAtStatement atStatement;
    WebSamplerListenerDidAllocateObject didAllocateObject;
    WebSamplerListenerDidDeallocateObject didDeallocateObject;
    WebSamplerListenerSetWebSamplerRef setWebSamplerRef;
};

struct WebSamplerListener {
    const WebSamplerListenerVTable* m_pVTable;
};

#endif // WebSamplerListener_h
