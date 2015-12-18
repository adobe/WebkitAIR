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

#include "config.h"
#include "WebDragClient.h"
#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif
#include "WebFrameImpl.h"
#include <ClipboardApollo.h>
#include <Frame.h>
#include <FrameView.h>
#include <Page.h>
#include <FrameLoaderClientApollo.h>
#include <apollo/proxy_obj.h>

#include "JSLock.h"
#include "ScriptController.h"
#include "JSDOMWindow.h"
#include "DOMWrapperWorld.h"

namespace WebKitApollo {

WebDragClient::WebDragClient()
{
}

WebDragClient::~WebDragClient()
{
}

void WebDragClient::willPerformDragDestinationAction(WebCore::DragDestinationAction, WebCore::DragData*)
{
}

void WebDragClient::willPerformDragSourceAction(WebCore::DragSourceAction, const WebCore::IntPoint&, WebCore::Clipboard*)
{
}

WebCore::DragDestinationAction WebDragClient::actionMaskForDrag(WebCore::DragData*)
{
    return (WebCore::DragDestinationAction) (WebCore::DragDestinationActionAny & ~WebCore::DragDestinationActionLoad);
}

WebCore::DragSourceAction WebDragClient::dragSourceActionMaskForPoint(const WebCore::IntPoint& windowPoint)
{
    return WebCore::DragSourceActionAny;
}

void WebDragClient::startDrag(WebCore::DragImageRef dragImage, const WebCore::IntPoint& dragImageOrigin, const WebCore::IntPoint& eventPos, WebCore::Clipboard* clipboard, WebCore::Frame* frame, bool linkDrag)
{
    m_frameProtector = frame;
    m_viewProtector = m_frameProtector->view();

    JSC::JSLock lock(false);

    WebCore::FrameLoaderClientApollo* clientApollo = WebCore::FrameLoaderClientApollo::clientApollo(frame);

    WebCore::JSDOMWindow* const globalObj(frame->page()->mainFrame()->script()->globalObject(WebCore::mainThreadNormalWorld()));
    JSC::ExecState* exec = globalObj->globalExec();
    JSC::JSObject* wkMethods = clientApollo->getWKMethods();
    JSC::JSObject* doDrag = wkMethods->get(exec, JSC::Identifier(exec, "doDrag")).toObject(exec);

    JSC::JSObject* jsClipboard = static_cast<WebCore::ClipboardApollo*>(clipboard)->dataTransfer();

	JSC::PutPropertySlot slot;
    jsClipboard->put(exec, JSC::Identifier(exec, "effectAllowed"), JSC::jsString(exec, stringToUString(clipboard->effectAllowed())), slot);

    WebCore::IntSize dragImageOffset(dragImageOrigin - eventPos);

    WebScriptProxyVariant* dragImageVariant = NULL;
    JSC::JSValue dragImageValue = JSC::jsNull();
    if (dragImage) {
        dragImageVariant = dragImage->m_pVTable->getBitmapVariant(dragImage);
        dragImageValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, dragImageVariant);
        if (linkDrag)
            dragImageOffset -= WebCore::IntSize(0, dragImage->m_pVTable->getHeight(dragImage));
    }

    JSC::MarkedArgumentBuffer args;
    args.append(jsClipboard);
    args.append(dragImageValue);
    args.append(JSC::jsNumber(exec, dragImageOffset.width()));
    args.append(JSC::jsNumber(exec, dragImageOffset.height()));
	
	JSC::CallData callData;
    JSC::CallType callType = doDrag->getCallData(callData);
    ASSERT(callType == JSC::CallTypeHost);
    JSC::call(exec, doDrag, callType, callData, wkMethods, args);	

    // Decrement drag image variant reference count.
    if (dragImageVariant)
        dragImageVariant->m_pVTable->release(dragImageVariant);
}

void WebDragClient::dragEnded()
{
    m_frameProtector = 0;
    m_viewProtector = 0;
}

WebCore::DragImageRef WebDragClient::createDragImageForLink(WebCore::KURL& url, const WebCore::String& label, WebCore::Frame* frame)
{
    JSC::JSLock lock(false);

    WebCore::FrameLoaderClientApollo* clientApollo = WebCore::FrameLoaderClientApollo::clientApollo(frame);
    WebHost* webHost = clientApollo->webHost();

    WebCore::JSDOMWindow* const globalObj(frame->page()->mainFrame()->script()->globalObject(WebCore::mainThreadNormalWorld()));
    JSC::ExecState* exec = globalObj->globalExec();
    JSC::JSObject* wkMethods = clientApollo->getWKMethods();
    JSC::JSObject* dragImageForLink = wkMethods->get(exec, JSC::Identifier(exec, "dragImageForLink")).toObject(exec);

    JSC::MarkedArgumentBuffer args;
    args.append(JSC::jsString(exec, stringToUString(url)));
    args.append(JSC::jsString(exec, stringToUString(label)));

	JSC::CallData callData;
    JSC::CallType callType = dragImageForLink->getCallData(callData);
    ASSERT(callType == JSC::CallTypeHost);
    JSC::JSValue bitmapValue = JSC::call(exec, dragImageForLink, callType, callData, wkMethods, args);
	
    WebScriptProxyVariant* bitmapVariant = WebCore::ApolloScriptBridging::getApolloVariantForJSValue(exec, bitmapValue);
    WebBitmap* bitmap = webHost->m_pVTable->createBitmapFromBitmapDataObject(webHost, bitmapVariant);
    if (bitmapVariant)
        bitmapVariant->m_pVTable->release(bitmapVariant);
    return bitmap;
}

void WebDragClient::dragControllerDestroyed()
{
}

}
