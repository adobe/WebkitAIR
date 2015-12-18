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
#ifndef WebFrame_h
#define WebFrame_h

#include <WebKitApollo/WebConfig.h>

struct WebFrameVTable;
struct WebFrameApollo;
struct WebScriptProxyVariant;
struct WebResourceRequest;

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameLoadHTMLStringFunction )( struct WebFrameApollo* pWebFrame
                                                                                          , const char* utf8HTMLString
                                                                                          , size_t numHTMLStringBytes
                                                                                          , const unsigned char* baseURL );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameLoadRequestFunction )( struct WebFrameApollo* pWebFrame
                                                                                       , const WebResourceRequest* request );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameEvalJavaScriptStringFunction )( struct WebFrameApollo* pWebFrame
                                                                                       , const uint16_t* utf16Text, 
																					   unsigned long numUTF16CodeUnits);

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameReloadFunction )( struct WebFrameApollo* pWebFrame );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameStopLoadingFunction )( struct WebFrameApollo* pWebFrame );

typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* ( WEBKIT_APOLLO_PROTO2 *WebFrameGetGlobalObjectFunction )( struct WebFrameApollo* pWebFrame );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameHistoryGoFunction )( struct WebFrameApollo* pWebFrame, int steps );

typedef WEBKIT_APOLLO_PROTO1 unsigned ( WEBKIT_APOLLO_PROTO2 *WebFrameGetHistoryLengthFunction )( struct WebFrameApollo* pWebFrame );

typedef WEBKIT_APOLLO_PROTO1 unsigned ( WEBKIT_APOLLO_PROTO2 *WebFrameGetHistoryPositionFunction )( struct WebFrameApollo* pWebFrame );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameSetHistoryPositionFunction )( struct WebFrameApollo* pWebFrame, unsigned position );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFrameGetHistoryAtFunction )( struct WebFrameApollo* pWebFrame
																						, unsigned position
																						, char** pUrl
																						, char** pOriginalUrl
																						, bool* pIsPost
																						, uint16_t** pTitle );

struct WebFrameVTable {
    unsigned long m_vTableSize;
    WebFrameLoadHTMLStringFunction loadHTMLString;
    WebFrameLoadRequestFunction loadRequest;
	WebFrameEvalJavaScriptStringFunction evalJavaScriptString;
    WebFrameReloadFunction reload;
    WebFrameStopLoadingFunction stopLoading;
    WebFrameGetGlobalObjectFunction getGlobalObject;
	WebFrameHistoryGoFunction historyGo;
	WebFrameGetHistoryLengthFunction getHistoryLength;
	WebFrameGetHistoryPositionFunction getHistoryPosition;
	WebFrameSetHistoryPositionFunction setHistoryPosition;
	WebFrameGetHistoryAtFunction getHistoryAt;
};

struct WebFrameApollo {
    const WebFrameVTable* m_pVTable;
};


#endif
