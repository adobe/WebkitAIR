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
#ifndef FrameLoaderClientApollo_h
#define FrameLoaderClientApollo_h

#include "ContextMenuItem.h"
#include <FrameLoaderClient.h>

struct WebResourceHandle;
struct WebHost;
struct WebWindow;
struct WebPopupWindow;

namespace JSC {
    class JSObject;
}

namespace WebCore {
    class ResourceHandleClient;
    class ResourceRequest;
    class Scrollbar;
    class Document;
    class CachedResource;
    class Pasteboard;
    class PluginView;
    class ClipboardApolloHelper;
    class PopupMenuClient;
    class FrameView;

    class FrameLoaderClientApollo : public FrameLoaderClient
    {
    public:
        static inline FrameLoaderClientApollo* clientApollo(FrameLoaderClient* const client) { ASSERT(client); return static_cast<FrameLoaderClientApollo*>(client); }
        static FrameLoaderClientApollo* clientApollo(const FrameLoader* const);
        static FrameLoaderClientApollo* clientApollo(const Frame* const);
        static FrameLoaderClientApollo* clientApollo(const FrameView* const);
    protected:
        FrameLoaderClientApollo();
        virtual ~FrameLoaderClientApollo();
    public:
        virtual WebResourceHandle* startResourceHandle(ResourceHandle*, ResourceHandleClient*, const ResourceRequest&) = 0;
        virtual void loadResourceSynchronously(const ResourceRequest&, ResourceError&, ResourceResponse&, Vector<char>& data) = 0;
        virtual void putFrameOnLifeSupport(WTF::PassRefPtr<Frame>) = 0;
        
        virtual JSC::JSObject* getRootPackage() = 0;
        virtual JSC::JSObject* getWKMethods() = 0;
        virtual ClipboardApolloHelper* createDraggingClipboard() = 0;
        virtual ClipboardApolloHelper* createGeneralClipboard() = 0;
        virtual Pasteboard* getGeneralPasteboard() = 0;
        virtual void setGeneralPasteboard(Pasteboard*) = 0;
        virtual WebPopupWindow* createWebPopupMenu(PopupMenuClient*, float computedFontSize) = 0;
        virtual WebHost *webHost() = 0;
        virtual WebWindow *webWindow() = 0;        
        virtual void documentCreated() = 0;
        virtual void* getPlatformInterpreterContext() const = 0;
        virtual void activateIMEForPlugin() = 0;
        virtual void deactivateIMEForPlugin() = 0;
        
        static bool canLoad(const KURL& url, const String& referrer);
        static bool canLoad(const KURL& url, const Document* doc);
        //static bool canLoad(const CachedResource& resource, const Document* doc);
		static bool canLoad(const KURL& url, const String& referrer, const Document* doc);
        static bool mapFrameUrl(Frame* frame, const KURL& url, KURL* mappedUrl);
		static bool unmapFrameUrl(Frame* frame, const KURL& url, KURL* unmappedUrl);
        static bool frameHasAllowCrossDomainXHRAttr(Frame* frame);

        virtual void dispatchDidPushStateWithinPage() { }
        virtual void dispatchDidReplaceStateWithinPage() { }
        virtual void dispatchDidPopStateWithinPage() { } 
        virtual void dispatchDidChangeIcons() { }
        virtual void dispatchWillSendSubmitEvent(HTMLFormElement*) { }
        virtual void dispatchDidAddBackForwardItem(HistoryItem*) const { }
        virtual void dispatchDidRemoveBackForwardItem(HistoryItem*) const { }
        virtual void dispatchDidChangeBackForwardIndex() const { }
        virtual void didDisplayInsecureContent() { }
        virtual void didRunInsecureContent(SecurityOrigin*) { }
        virtual void didTransferChildFrameToNewDocument() { }
        virtual void dispatchDidClearWindowObjectInWorld(DOMWrapperWorld*) {}
        
    };
}

#endif
