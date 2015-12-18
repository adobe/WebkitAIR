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
#include "ClipboardApollo.h"

#include "CharacterNames.h"
#include "ClipboardApolloHelper.h"
#include "Document.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "FrameLoaderClientApollo.h"
#include "HTMLCanvasElement.h"
#include "HTMLImageElement.h"
#include "KURL.h"
#include "Pasteboard.h"
#include "Range.h"
#include "runtime/JSObject.h"
#include "JSDOMBinding.h"
#include <CachedImage.h>
#include <GraphicsContext.h>
#include <Image.h>
#include <RenderImage.h>
#include <WebKitApollo/WebBitmap.h>
#include <WebKitApollo/WebHost.h>
#include <WebKitApollo/WebScriptProxy.h>
#include <apollo/proxy_obj.h>
#include "HTMLNames.h"
#include "JSLock.h"
#include "FileList.h"

#if PLATFORM(CG)
#include <ApplicationServices/ApplicationServices.h>
#elif PLATFORM(CAIRO)
#include <cairo.h>
#endif

namespace WebCore {

ClipboardApollo::ClipboardApollo(bool forDragging, ClipboardApolloHelper* clipboard, ClipboardAccessPolicy policy, Frame* frame) :
    Clipboard(policy, forDragging)
    , m_clipboard(clipboard)
    , m_frame(frame)
{
    ASSERT(m_clipboard);
    ASSERT(m_frame);
}

ClipboardApollo::~ClipboardApollo()
{
}

void ClipboardApollo::clearData(const String &type)
{
    if (policy() == ClipboardWritable)
        m_clipboard->clearData(type);
}

void ClipboardApollo::clearAllData()
{
    if (policy() == ClipboardWritable)
        m_clipboard->clearAllData();
}

String ClipboardApollo::getData(const String &type, bool &success) const
{
    success = false;
    if (policy() != ClipboardReadable)
        return String();
    return m_clipboard->getDataString(type, success);
}

JSC::JSValue ClipboardApollo::getDataValue(const String& type, bool& success) const
{
    success = false;
    if (policy() != ClipboardReadable)
        return JSC::jsUndefined();
    return m_clipboard->getData(type, success);
}

WTF::PassRefPtr<Node> ClipboardApollo::getDataNode(const String& type, bool& success) const
{
    success = false;
    WTF::PassRefPtr<Node> result;
    if (policy() != ClipboardReadable)
        return result;

    JSC::JSValue dataValue = m_clipboard->getData(type, success);
    if (!dataValue.isNull() && dataValue.isObject()) {
        FrameLoaderClientApollo* clientApollo = FrameLoaderClientApollo::clientApollo(m_frame);
        WebHost* webHost = clientApollo->webHost();
        ASSERT(webHost);

        WebScriptProxyVariant* dataVariant = WebCore::ApolloScriptBridging::getApolloVariantForJSValue(m_clipboard->execState(), dataValue);
        WebBitmap* webBitmap = webHost->m_pVTable->createBitmapFromBitmapDataObject(webHost, dataVariant);

        int width = webBitmap->m_pVTable->getWidth(webBitmap);
        int height = webBitmap->m_pVTable->getHeight(webBitmap);
        void *pixelData = webBitmap->m_pVTable->getPixelData(webBitmap);
        unsigned long stride = webBitmap->m_pVTable->getStride(webBitmap);

        WTF::RefPtr<HTMLCanvasElement> canvasElement = new HTMLCanvasElement(HTMLNames::canvasTag, m_frame->document());
        canvasElement->setWidth(width);
        canvasElement->setHeight(height);
        GraphicsContext* canvasContext = canvasElement->drawingContext();

#if PLATFORM(CG)
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
        CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst;
        CFDataRef cfData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, static_cast<UInt8*>(pixelData), stride * height, kCFAllocatorNull);
        CGDataProviderRef dataProvider = CGDataProviderCreateWithCFData(cfData);
        CGImageRef image = CGImageCreate(width, height, 8, 32, stride, colorSpace, bitmapInfo, dataProvider, NULL, false, kCGRenderingIntentDefault);
        CGRect rect = CGRectMake(0, 0, width, height);
        canvasContext->save();
        canvasContext->translate(0, height);
        canvasContext->scale(FloatSize(1, -1));
        CGContextDrawImage(canvasContext->platformContext(), rect, image);
        canvasContext->restore();
        CGImageRelease(image);
        CGDataProviderRelease(dataProvider);
        CFRelease(cfData);
        CGColorSpaceRelease(colorSpace);
#elif PLATFORM(CAIRO)
        cairo_surface_t* surface = cairo_image_surface_create_for_data((unsigned char*) pixelData, CAIRO_FORMAT_RGB24, width, height, stride);
        cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surface);
        cairo_matrix_t patternMatrix;
#if PLATFORM(APOLLO_UNIX)
        /*
         * The image on the surface has the correct orientation, however
         * by applying the matrix incorrectly inverts the image. This works
         * fine on Windows as the underlying bitmap object expects it to be
         * upside down. However it fails on Linux which uses a GdkPixbuf
         * underneath and that expects the image in the correct orientation.
         */
        cairo_matrix_init(&patternMatrix, 1, 0, 0, 1, 0, 0);
#else
        cairo_matrix_init(&patternMatrix, 1, 0, 0, -1, 0, height);
#endif
        cairo_pattern_set_matrix(pattern, &patternMatrix);
        canvasContext->save();
        cairo_t* cr = canvasContext->platformContext();
        cairo_set_source(cr, pattern);
        cairo_rectangle(cr, 0, 0, width, height);
        cairo_fill(cr);
        canvasContext->restore();
        cairo_pattern_destroy(pattern);
        cairo_surface_destroy(surface);
#endif

        result = canvasElement.release();
    }
    return result;
}

bool ClipboardApollo::setData(const String &type, const String &data)
{
    if (policy() != ClipboardWritable)
        return false;
    return m_clipboard->setData(type, data);
}

bool ClipboardApollo::setData(const String& type, JSC::JSValue data)
{
    if (policy() != ClipboardWritable)
        return false;
    return m_clipboard->setData(type, data);
}

bool ClipboardApollo::setData(const String& type, Node* node)
{
    if (policy() != ClipboardWritable)
        return false;
    if (!node)
        return false;
    if (!node->isElementNode())
        return false;

    if (type != ClipboardApolloHelper::BITMAP_TYPE)
        return false;

    Element* element = static_cast<Element*>(node);
    WebCore::RenderObject* renderer = element->renderer();
    WebBitmap* webBitmap = NULL;
    if (renderer && renderer->isImage()) {
        WebCore::RenderImage* img = static_cast<WebCore::RenderImage*>(renderer);
        if (img->cachedImage() && !img->cachedImage()->errorOccurred())
            webBitmap = img->cachedImage()->image()->getWebBitmap(m_frame);
    }

    if (!webBitmap) return false;

    WebScriptProxyVariant* webBitmapVariant = webBitmap->m_pVTable->getBitmapVariant(webBitmap);
    ASSERT(webBitmapVariant);
    JSC::JSLock lock(false);
    JSC::JSValue webBitmapValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(m_clipboard->execState(), webBitmapVariant);
    ASSERT(webBitmapValue);

    bool result = m_clipboard->setData(type, webBitmapValue);

    webBitmapVariant->m_pVTable->release(webBitmapVariant);
    webBitmap->m_pVTable->detach(webBitmap);

    return result;
}

bool ClipboardApollo::hasData()
{
    return m_clipboard->hasData();
}

HashSet<String> ClipboardApollo::types() const
{
    HashSet<String> result;
    ClipboardAccessPolicy accessPolicy = policy();
    if ((accessPolicy == ClipboardReadable) || (accessPolicy == ClipboardTypesReadable))
        result = m_clipboard->types();
    return result;
}

PassRefPtr<FileList> ClipboardApollo::files() const
{
    return 0;
}

void ClipboardApollo::setDragImage(CachedImage *image, const IntPoint &loc)
{
    setDragImage(image, 0, loc);
}

void ClipboardApollo::setDragImageElement(Node *node, const IntPoint &loc)
{
    setDragImage(0, node, loc);
}

void ClipboardApollo::setDragImage(CachedImage* image, Node* node, const IntPoint& loc)
{
    if (policy() != ClipboardImageWritable && policy() != ClipboardWritable)
        return;

    if (m_dragImage)
        m_dragImage->removeClient(this);
    m_dragImage = image;
    if (m_dragImage)
        m_dragImage->addClient(this);

    m_dragLoc = loc;
    m_dragImageElement = node;
}

DragImageRef ClipboardApollo::createDragImage(IntPoint &dragLoc) const
{
    return dragWebBitmap(dragLoc);
}

void ClipboardApollo::declareAndWriteDragImage(Element* element, const KURL& url, const String& title, Frame* frame)
{
    ASSERT(frame);

    writeURL(url, title, frame);

    WebCore::RenderObject* renderer = element->renderer();
    if (renderer && renderer->isImage()) {
        WebCore::RenderImage* img = static_cast<WebCore::RenderImage*>(renderer);
        if (img->cachedImage() && !img->cachedImage()->errorOccurred()) {
            WebBitmap* webBitmap = img->cachedImage()->image()->getWebBitmap(frame);
            ASSERT(webBitmap);
            WebScriptProxyVariant* webBitmapVariant = webBitmap->m_pVTable->getBitmapVariant(webBitmap);
            ASSERT(webBitmapVariant);
            JSC::JSLock lock(false);
            JSC::JSValue webBitmapValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(m_clipboard->execState(), webBitmapVariant);
            ASSERT(webBitmapValue);
            m_clipboard->setData(ClipboardApolloHelper::BITMAP_TYPE, webBitmapValue);
            // Decrement bitmap variant reference count.
            webBitmapVariant->m_pVTable->release(webBitmapVariant);
        }
    }
}

void ClipboardApollo::writeRange(Range* range, Frame* frame)
{
    //no permission check because ClipboardMac does not do a permission check.
    ASSERT(range);
    ASSERT(frame);
    Pasteboard::writeSelection(m_clipboard.get(), range, false, frame);
}

void ClipboardApollo::writeURL(const KURL &url, const String &title, Frame *frame)
{
    //no permission check because ClipboardMac does not do a permission check.
    Pasteboard::writeURL(m_clipboard.get(), url, title, frame);
}

void ClipboardApollo::writePlainText(const String& text, Frame* frame)
{
    Pasteboard::writePlainText(m_clipboard.get(), text, frame);
}

JSC::JSObject* ClipboardApollo::dataTransfer() const
{
    return m_clipboard->dataTransfer();
}

WebBitmap* ClipboardApollo::dragWebBitmap(IntPoint &loc) const
{
    WebBitmap* result = NULL;
    if (m_dragImageElement) {
        if (m_frame) {
            IntRect imageRect, elementRect;
            result = m_frame->snapshotDragImage(m_dragImageElement.get(), imageRect, elementRect);
            // Client specifies point relative to element, not the whole image, which may include child
            loc.setX(elementRect.x() - imageRect.x() + m_dragLoc.x());
            loc.setY(elementRect.y() - imageRect.y() + m_dragLoc.y());
        }
    } else if (m_dragImage) {
        result = m_dragImage->image()->getWebBitmap(m_frame);
        loc = m_dragLoc;
    }
    return result;
}

}
