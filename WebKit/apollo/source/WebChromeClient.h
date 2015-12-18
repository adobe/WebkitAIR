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
 
#ifndef WebChromeClient_h
#define WebChromeClient_h
#include <wtf/Platform.h>
#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif
#include <ChromeClient.h>
#include <Timer.h>
#include "WindowFeatures.h"

struct WebWindow;
struct WebHost;
class Frame;
class FileChooser;

namespace WebKitApollo
{
	class WebChromeClient : public WebCore::ChromeClient
	{
	public:
		WebChromeClient(WebWindow* const pWindow, WebHost* const pHost/*, WebViewImpl* const pWebViewImpl*/);
		virtual ~WebChromeClient();
        virtual void chromeDestroyed();
        
        virtual void setWindowRect(const WebCore::FloatRect&);
        virtual WebCore::FloatRect windowRect();
        
        virtual WebCore::FloatRect pageRect();
        
        virtual float scaleFactor();
    
        virtual void focus();
        virtual void unfocus();

        virtual bool canTakeFocus(WebCore::FocusDirection);
        virtual void takeFocus(WebCore::FocusDirection);

        virtual void focusedNodeChanged(WebCore::Node*);

        virtual WebCore::Page* createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest&, const WebCore::WindowFeatures&);
        virtual void show();

        virtual bool canRunModal();
        virtual void runModal();

        virtual void setToolbarsVisible(bool);
        virtual bool toolbarsVisible();
        
        virtual void setStatusbarVisible(bool);
        virtual bool statusbarVisible();
        
        virtual void setScrollbarsVisible(bool);
        virtual bool scrollbarsVisible();
        
        virtual void setMenubarVisible(bool);
        virtual bool menubarVisible();

        virtual void setResizable(bool);
        
        virtual void addMessageToConsole(WebCore::MessageSource, WebCore::MessageType, WebCore::MessageLevel, const WebCore::String& message, 
            unsigned int lineNumber, const WebCore::String& sourceID);

        virtual bool canRunBeforeUnloadConfirmPanel();
        virtual bool runBeforeUnloadConfirmPanel(const WebCore::String& message, WebCore::Frame* frame);

        virtual void closeWindowSoon();
        
        virtual void runJavaScriptAlert(WebCore::Frame*, const WebCore::String&);
        virtual bool runJavaScriptConfirm(WebCore::Frame*, const WebCore::String&);
        virtual bool runJavaScriptPrompt(WebCore::Frame*, const WebCore::String& message, const WebCore::String& defaultValue, WebCore::String& result);
        
        virtual void setStatusbarText(const WebCore::String&);
        virtual bool shouldInterruptJavaScript();
        virtual bool tabsToLinks() const;

        virtual WebCore::IntRect windowResizerRect() const;

        virtual void invalidateWindow(const WebCore::IntRect&, bool);
        virtual void invalidateContentsAndWindow(const WebCore::IntRect&, bool);
        virtual void invalidateContentsForSlowScroll(const WebCore::IntRect&, bool);

        virtual void mouseDidMoveOverElement(const WebCore::HitTestResult&, unsigned modifierFlags);

        virtual void setToolTip(const WebCore::String&, WebCore::TextDirection);

        virtual void print(WebCore::Frame*);

        virtual bool processingUserGesture();

        virtual void startWindowedPluginUserGesture();
        virtual void endWindowedPluginUserGesture();


        virtual void uncaughtJavaScriptException(const JSC::ExecState* const exec, JSC::JSValue const pExceptionValue);

        virtual WebWindow* getApolloWindow();

        // Methods used by HostWindow.
        virtual void repaint(const WebCore::IntRect&, bool contentChanged, bool immediate = false, bool repaintContentOnly = false);
        virtual void scroll(const WebCore::IntSize& scrollDelta, const WebCore::IntRect& rectToScroll, const WebCore::IntRect& clipRect) ;
        virtual WebCore::IntPoint screenToWindow(const WebCore::IntPoint&) const;
        virtual WebCore::IntRect windowToScreen(const WebCore::IntRect&) const;

        virtual PlatformPageClient platformPageClient() const;

        virtual void contentsSizeChanged(WebCore::Frame*, const WebCore::IntSize&) const;
        virtual void scrollRectIntoView(const WebCore::IntRect&, const WebCore::ScrollView*) const; 
        // End methods used by HostWindow.

        virtual void scrollbarsModeDidChange() const;

        virtual void requestGeolocationPermissionForFrame(WebCore::Frame*, WebCore::Geolocation*);
        virtual void cancelGeolocationPermissionRequestForFrame(WebCore::Frame*, WebCore::Geolocation*);
        
        virtual void runOpenPanel(WebCore::Frame*, WTF::PassRefPtr<WebCore::FileChooser>);

        // Asynchronous request to load an icon for specified filenames.
        virtual void chooseIconForFiles(const WTF::Vector<WebCore::String>&, WebCore::FileChooser*);

        
        virtual bool setCursor(WebCore::PlatformCursorHandle);
        
        virtual void formStateDidChange(const WebCore::Node*);
        virtual WTF::PassOwnPtr<WebCore::HTMLParserQuirks> createHTMLParserQuirks();
		
	private:

		WebCore::WindowFeatures m_features;
        void SetDefaultWindowFeatures();

        void onCloseTimer(WebCore::Timer<WebChromeClient>*);

		WebWindow* const m_pWindow;
		WebHost* const m_pHost;
        //WebViewImpl* const m_pWebViewImpl;
        WebCore::Timer<WebChromeClient> m_closeTimer;
	};
}

#endif

