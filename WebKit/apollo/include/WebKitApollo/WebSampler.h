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

#ifndef WebSampler_h
#define WebSampler_h

#include <WebKitApollo/WebKit.h>

struct WebSampler;

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerStartSampling )( struct WebSampler* pWebSampler );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerStopSampling )( struct WebSampler* pWebSampler );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerPauseSampling )( struct WebSampler* pWebSampler );

typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebSamplerGetObjectTypeName )( struct WebSampler* pWebSampler, uint64_t identifier );
typedef WEBKIT_APOLLO_PROTO1 size_t ( WEBKIT_APOLLO_PROTO2 *WebSamplerGetObjectSize )( struct WebSampler* pWebSampler, uint64_t identifier );

typedef WEBKIT_APOLLO_PROTO1 uint64_t ( WEBKIT_APOLLO_PROTO2 *WebSamplerGetObjectCallCount )( struct WebSampler* pWebSampler, uint64_t identifier );
typedef WEBKIT_APOLLO_PROTO1 bool ( WEBKIT_APOLLO_PROTO2 *WebSamplerGetObjectMembers )( struct WebSampler* pWebSampler, uint64_t identifier, WebSamplerMembersList* pWebSamplerMembersList );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSamplerClearDeadObjects )( struct WebSampler* pWebSampler );

typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebSamplerGetObjectFilename )( struct WebSampler* pWebSampler, uint64_t identifier );
typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebSamplerGetObjectName )( struct WebSampler* pWebSampler, uint64_t identifier );

struct WebSamplerVTable {
	unsigned long m_vTableSize;
    WebSamplerStartSampling startSampling;
    WebSamplerStopSampling stopSampling;
	WebSamplerStopSampling pauseSampling;
    WebSamplerGetObjectTypeName getObjectType;
    WebSamplerGetObjectSize getObjectSize;
    WebSamplerGetObjectMembers getObjectMembers;
	WebSamplerGetObjectFilename getObjectFilename;
	WebSamplerGetObjectName getObjectName;
    WebSamplerGetObjectCallCount getObjectCallCount;
    WebSamplerClearDeadObjects clearDeadObjects;
};

struct WebSampler {
    const WebSamplerVTable* m_pVTable;
};

#endif // WebSampler_h
