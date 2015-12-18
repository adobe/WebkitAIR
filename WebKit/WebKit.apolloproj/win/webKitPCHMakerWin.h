/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
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
#ifndef webKitPCHMakerWin_h
#define webKitPCHMakerWin_h

#include <config.h>

#ifdef __cplusplus
#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>
#include <wtf/GetPtr.h>
#include <wtf/HashCountedSet.h>
#include <wtf/HashFunctions.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/HashTable.h>
#include <wtf/HashTraits.h>
#include <wtf/ListHashSet.h>
#include <wtf/ListRefPtr.h>
#include <wtf/MathExtras.h>
#include <wtf/Noncopyable.h>
#include <wtf/OwnArrayPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/UnusedParam.h>
#include <wtf/Vector.h>
#include <wtf/VectorTraits.h>

#include <AtomicString.h>
#include <FloatPoint.h>
#include <FloatRect.h>
#include <FloatSize.h>
#include <IntPoint.h>
#include <IntRect.h>
#include <IntSize.h>
#include <PlatformString.h>

#include <WebKitApollo/WebBitmap.h>
#include <WebKitApollo/WebCursor.h>
#include <WebKitApollo/WebError.h>
#include <WebKitApollo/WebFrame.h>
#include <WebKitApollo/WebHost.h>
#include <WebKitApollo/WebKit.h>
#include <WebKitApollo/WebPoint.h>
#include <WebKitApollo/WebRect.h>
#include <WebKitApollo/WebResourceHandle.h>
#include <WebKitApollo/WebResourceHandleClient.h>
#include <WebKitApollo/WebResourceRequest.h>
#include <WebKitApollo/WebResourceResponse.h>
#include <WebKitApollo/WebScriptProxy.h>
#include <WebKitApollo/WebView.h>
#include <WebKitApollo/WebWindow.h>
#include <WebKitApollo/WebDebugState.h>
#include <WebKitApollo/WebDebugListener.h>
#include <WebKitApollo/helpers/WebBitmapHelper.h>
#include <WebKitApollo/helpers/WebFrameHelper.h>
#include <WebKitApollo/helpers/WebHostHelper.h>
#include <WebKitApollo/helpers/WebResourceHandleClientHelper.h>
#include <WebKitApollo/helpers/WebResourceHandleHelper.h>
#include <WebKitApollo/helpers/WebResourceRequestHelper.h>
#include <WebKitApollo/helpers/WebScriptProxyHelper.h>
#include <WebKitApollo/helpers/WebViewHelper.h>
#include <WebKitApollo/helpers/WebWindowHelper.h>
#include <WebKitApollo/helpers/WebDebugStateHelper.h>
#include <WebKitApollo/helpers/WebDebugListenerHelper.h>
#endif

#endif
