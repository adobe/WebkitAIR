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
#include <Pasteboard.h>

#include <CharacterNames.h>
#include <ClipboardApolloHelper.h>
#include <Document.h>
#include <DocumentFragment.h>
#include <Frame.h>
#include <FrameLoader.h>
#include <FrameLoaderClientApollo.h>
#include <HitTestResult.h>
#include <Image.h>
#include <Range.h>
#include <RenderImage.h>
#include <WebKitApollo/WebBitmap.h>
#include <WebKitApollo/WebScriptProxy.h>
#include <apollo/proxy_obj.h>
#include <JSDOMBinding.h>
#include <ScriptController.h>
#include <markup.h>
#include "JSLock.h"

namespace WebCore {

Pasteboard* Pasteboard::generalPasteboard(Frame* frame)
{
    FrameLoaderClientApollo* const frameLoaderClient = FrameLoaderClientApollo::clientApollo(frame);
    ASSERT(frameLoaderClient);
    Pasteboard* generalPasteboard = frameLoaderClient->getGeneralPasteboard();
    if (!generalPasteboard) {
        ClipboardApolloHelper* clipboard = frameLoaderClient->createGeneralClipboard();
        ASSERT(clipboard);
        generalPasteboard = new Pasteboard(clipboard);
        frameLoaderClient->setGeneralPasteboard(generalPasteboard);
    }
    return generalPasteboard;
}

Pasteboard::Pasteboard(ClipboardApolloHelper* clipboard)
    : m_clipboard(clipboard)
{
}

Pasteboard::Pasteboard()
{
}

Pasteboard::~Pasteboard()
{
}

bool Pasteboard::canSmartReplace()
{
    return false;
}

void Pasteboard::clear()
{
    ASSERT(m_clipboard);
    m_clipboard->clearAllData();
}

PassRefPtr<DocumentFragment> Pasteboard::documentFragment(Frame* frame, PassRefPtr<Range> range, bool allowPlainText, bool& chosePlainText)
{
    ASSERT(frame);
    ASSERT(m_clipboard);

    RefPtr<DocumentFragment> result;

    chosePlainText = false;
    bool hasHTML = false;
    String htmlString = m_clipboard->getDataString(ClipboardApolloHelper::HTML_TYPE, hasHTML);
    if (hasHTML) {
        if (htmlString.length()) {
            result = createFragmentFromMarkup(frame->document(), htmlString, "");
        }
    }
    if (allowPlainText && !result) {
        bool hasText = false;
        String textString = m_clipboard->getDataString(ClipboardApolloHelper::TEXT_TYPE, hasText);
        if (hasText) {
            chosePlainText = true;
            result = createFragmentFromText(range.get(), textString);
        }
    }

    return result.release();
}

String Pasteboard::plainText(Frame* /*frame*/)
{
    bool success = false;
    return m_clipboard->getDataString(ClipboardApolloHelper::TEXT_TYPE, success);
}

void Pasteboard::writeSelection(ClipboardApolloHelper* clipboard, Range* selectedRange, bool /*canSmartCopyOrDelete*/, Frame* frame)
{
    ASSERT(selectedRange);
    ASSERT(frame);
    String text = frame->displayStringModifiedByEncoding(selectedRange->text());
    
#if OS(WINDOWS)
	static const UChar sNewLine = '\n';
    static const char* const sWindowsNewline("\r\n");
    text.replace(sNewLine, sWindowsNewline);
#endif
	
    // Map &nbsp; to a plain space.
    text.replace(noBreakSpace, ' ');
    clipboard->setData(ClipboardApolloHelper::TEXT_TYPE, text);
	
	String html = createMarkup(selectedRange, 0, AnnotateForInterchange);
#if OS(DARWIN)
    html = String("<meta charset='utf-8'>") + html;
#endif
    clipboard->setData(ClipboardApolloHelper::HTML_TYPE, html);
}

void Pasteboard::writeSelection(Range* selectedRange, bool canSmartCopyOrDelete, Frame* frame)
{
    writeSelection(m_clipboard.get(), selectedRange, canSmartCopyOrDelete, frame);
}

void Pasteboard::writePlainText(const String& text, Frame *frame)
{
    writePlainText(m_clipboard.get(), text, frame);
}

void Pasteboard::writeURL(ClipboardApolloHelper* clipboard, const KURL& url, const String& /*title*/, Frame* /*frame*/)
{
    clipboard->setData(ClipboardApolloHelper::URI_LIST_TYPE, String(url.string()));
}

void Pasteboard::writeURL(const KURL& url, const String& title, Frame* frame)
{
    writeURL(m_clipboard.get(), url, title, frame);
}

void Pasteboard::writeImage(Node* node, const KURL& url, const String& title)
{
    Frame* frame = node->document()->frame();

    writeURL(url, title, frame);

    ASSERT(node && node->renderer() && node->renderer()->isImage());

    // We have to make sure we have a renderer here to get the image from.
    // If the right context menu with copy image was shown before the image is hidden
    // or removed from DOM, then CopyImage item is still available but the image does 
    // not have a renderer.
    // There is also a WebKit bug: https://bugs.webkit.org/show_bug.cgi?id=31721.
    // I leave the above assert in place as a reminder that this is only a crash fix.
    // When the webkit bugs is fixed, the assert will no longer trigger for already described case.
    bool nodeHasValidRenderer = node->renderer() && node->renderer()->isImage();
    if (!nodeHasValidRenderer)
        return;

    RenderImage* renderer = toRenderImage(node->renderer());
    CachedImage* cachedImage = static_cast<CachedImage*>(renderer->cachedImage());
    ASSERT(cachedImage);
    Image* image = cachedImage->image();
    ASSERT(image);

    WebBitmap* webBitmap = image->getWebBitmap(frame);
    WebScriptProxyVariant* webBitmapVariant = webBitmap->m_pVTable->getBitmapVariant(webBitmap);
    JSC::JSLock lock(false);
    JSC::JSValue webBitmapValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(m_clipboard->execState(), webBitmapVariant);
    m_clipboard->setData(ClipboardApolloHelper::BITMAP_TYPE, webBitmapValue);
    // Decrement bitmap variant reference count.
    if (webBitmapVariant)
        webBitmapVariant->m_pVTable->release(webBitmapVariant);
}

void Pasteboard::writePlainText(ClipboardApolloHelper* clipboard, const String& text, Frame *frame)
{
    ASSERT(frame);
    ASSERT(clipboard);

    String textWithEncoding;
    if (frame)
        textWithEncoding = frame->displayStringModifiedByEncoding(text);
    
#if OS(WINDOWS)
	static const UChar sNewLine = '\n';
    static const char* const sWindowsNewline("\r\n");
    textWithEncoding.replace(sNewLine, sWindowsNewline);
#endif
	
    // Map &nbsp; to a plain space.
    textWithEncoding.replace(noBreakSpace, ' ');
    if (clipboard)
        clipboard->setData(ClipboardApolloHelper::TEXT_TYPE, textWithEncoding);
}

}
