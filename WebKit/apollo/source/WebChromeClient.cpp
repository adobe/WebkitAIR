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
#include "WebChromeClient.h"
#include <FloatRect.h>
#include <IntRect.h>
#include <wtf/Assertions.h>
#include <PlatformString.h>
#include <TextEncoding.h>
#include <WebKitApollo/WebKit.h>
#include <apollo/proxy_obj.h>
#include "WebViewImpl.h"
#include <Frame.h>
#include <FrameView.h>
#include <FrameLoader.h>
#include <FrameLoadRequest.h>
#include "Page.h"
#include <Cursor.h>
#include "Chrome.h"

namespace WebKitApollo
{

extern WebKitAPIHostFunctions* g_HostFunctions;

WebChromeClient::WebChromeClient(WebWindow* const pWindow, WebHost* const pHost/*, WebViewImpl* const pWebViewImpl*/)
	: m_pWindow(pWindow)
	, m_pHost(pHost)
    //, m_pWebViewImpl( pWebViewImpl )
    , m_closeTimer(this, &WebChromeClient::onCloseTimer)
{
    SetDefaultWindowFeatures();
}

WebChromeClient::~WebChromeClient()
{
}

void WebChromeClient::chromeDestroyed()
{
}

void WebChromeClient::SetDefaultWindowFeatures()
{
    m_features.x = 0;
    m_features.xSet = false;
    m_features.y = 0;
    m_features.ySet = false;
    m_features.width = 0;
    m_features.widthSet = false;
    m_features.height = 0;
    m_features.heightSet = false;
    m_features.menuBarVisible = true;
    m_features.statusBarVisible = true;
    m_features.toolBarVisible = true;
    m_features.locationBarVisible = true;
    m_features.scrollbarsVisible = true;
    m_features.resizable = true;
    m_features.fullscreen = false;
    m_features.dialog = false;
}

void WebChromeClient::setWindowRect(const WebCore::FloatRect& windowRect)
{
    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    WebFloatRect const newWindowRect = {   windowRect.x(),
                                           windowRect.location().y(),
                                           windowRect.location().x() + windowRect.width(),
                                           windowRect.location().y() + windowRect.height() };
    m_pWindow->m_pVTable->setWindowRect( m_pWindow, &newWindowRect );
}

WebCore::FloatRect WebChromeClient::windowRect()
{
    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    ASSERT(m_pWindow->m_pVTable->getWindowRect);
    WebFloatRect windowRect = { 0, 0, 0, 0 };
    m_pWindow->m_pVTable->getWindowRect(m_pWindow, &windowRect);

    ASSERT(windowRect.m_left <= windowRect.m_right);
    ASSERT(windowRect.m_top <= windowRect.m_bottom);

    float const windowWidth = windowRect.m_right - windowRect.m_left;
    float const windowHeight = windowRect.m_bottom - windowRect.m_top;
    
    WebCore::FloatRect const frameRect(windowRect.m_left, windowRect.m_top, windowWidth, windowHeight);
	return frameRect;
}


WebCore::FloatRect WebChromeClient::pageRect()
{
    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    ASSERT(m_pWindow->m_pVTable->getPageRect);
    WebFloatRect pageRect = { 0, 0, 0, 0 };
    m_pWindow->m_pVTable->getPageRect(m_pWindow, &pageRect);

    ASSERT(pageRect.m_left <= pageRect.m_right);
    ASSERT(pageRect.m_top <= pageRect.m_bottom);

    float const pageWidth = pageRect.m_right - pageRect.m_left;
    float const pageHeight = pageRect.m_bottom - pageRect.m_top;
    
    WebCore::FloatRect const pageGeometry(pageRect.m_left, pageRect.m_top, pageWidth, pageHeight);
	return pageGeometry;
}


float WebChromeClient::scaleFactor()
{
	return 1;
}


void WebChromeClient::focus()
{
    ASSERT(m_pWindow);
    m_pWindow->m_pVTable->focus(m_pWindow);
}

void WebChromeClient::unfocus()
{
    ASSERT(m_pWindow);
    m_pWindow->m_pVTable->unfocus(m_pWindow);
}


bool WebChromeClient::canTakeFocus(WebCore::FocusDirection)
{
    ASSERT(m_pWindow);
    return (m_pWindow->m_pVTable->canTakeFocusFromWebKit(m_pWindow) != 0);
}

void WebChromeClient::takeFocus(WebCore::FocusDirection direction)
{
    ASSERT(m_pWindow);
    m_pWindow->m_pVTable->takeFocusFromWebKit(m_pWindow, (int)direction);
}

void WebChromeClient::focusedNodeChanged(WebCore::Node*)
{
    //apollo integrate TODO:
}

WebCore::Page* WebChromeClient::createWindow(WebCore::Frame* frame, const WebCore::FrameLoadRequest& loadRequest, const WebCore::WindowFeatures& features)
{
	if (m_pHost) {
        ASSERT(m_pHost->m_pVTable);
        ASSERT(m_pHost->m_pVTable->createNewWindow);

		// Copy features into binary-stable structure.
		WebHostCreateWindowArgs createArgs;
		memset(&createArgs, 0, sizeof createArgs);
		createArgs.windowArgsSize = sizeof(WebHostCreateWindowArgs);
		createArgs.x = features.xSet ? int(features.x) : WebHostCreateWindowArgs::USE_DEFAULT;
		createArgs.y = features.ySet ? int(features.y) : WebHostCreateWindowArgs::USE_DEFAULT;
		createArgs.width = features.widthSet ? int(features.width) : WebHostCreateWindowArgs::USE_DEFAULT;
		createArgs.height = features.heightSet ? int(features.height) : WebHostCreateWindowArgs::USE_DEFAULT;
		createArgs.menuBarVisible = features.menuBarVisible;
		createArgs.statusBarVisible = features.statusBarVisible;
		createArgs.toolBarVisible = features.toolBarVisible;
		createArgs.locationBarVisible = features.locationBarVisible;
		createArgs.scrollBarsVisible = features.scrollbarsVisible;
		createArgs.resizable = features.resizable;
		createArgs.fullscreen = features.fullscreen;

        WebViewApollo *webView = m_pHost->m_pVTable->createNewWindow(m_pHost, &createArgs);
        if (webView != NULL)
        {
            WebViewImpl* const webViewImpl = WebViewImpl::getImpl(webView);
            if (!loadRequest.isEmpty())
                //mihnea integrate -> lockHistory = false
                webViewImpl->mainFrame()->loader()->load(loadRequest.resourceRequest(), false);

			WebCore::Page *page = webViewImpl->page();

            static_cast<WebChromeClient*>(page->chrome()->client())->m_features = features;
			return page;       
        }
    }
	return NULL;
}

void WebChromeClient::show()
{
	// Not needed for Apollo
}


bool WebChromeClient::canRunModal()
{
	return false;
}

void WebChromeClient::runModal()
{
}


void WebChromeClient::setToolbarsVisible(bool bVisible)
{
}

bool WebChromeClient::toolbarsVisible()
{
    return m_features.toolBarVisible;
}


void WebChromeClient::setStatusbarVisible(bool bVisible)
{
}

bool WebChromeClient::statusbarVisible()
{
    return m_features.statusBarVisible;
}


void WebChromeClient::setScrollbarsVisible(bool bVisible)
{
}

bool WebChromeClient::scrollbarsVisible()
{
    return m_features.scrollbarsVisible;
}


void WebChromeClient::setMenubarVisible(bool bVisible)
{
}

bool WebChromeClient::menubarVisible()
{
    return m_features.menuBarVisible;
}


void WebChromeClient::setResizable(bool bResizable)
{
}


void WebChromeClient::addMessageToConsole(WebCore::MessageSource, WebCore::MessageType, WebCore::MessageLevel, const WebCore::String& message, unsigned int lineNumber, const WebCore::String& sourceID)
{
}


bool WebChromeClient::canRunBeforeUnloadConfirmPanel()
{
	return false;
}

bool WebChromeClient::runBeforeUnloadConfirmPanel(const WebCore::String& message, WebCore::Frame* frame)
{
	return false;
}


void WebChromeClient::closeWindowSoon()
{
    m_closeTimer.startOneShot(0);
}


void WebChromeClient::runJavaScriptAlert(WebCore::Frame*, const WebCore::String& message)
{
    ASSERT(m_pHost);
    WTF::CString messageAsUTF8(message.utf8());
    m_pHost->m_pVTable->runJSAlert(m_pHost, messageAsUTF8.data());
}

bool WebChromeClient::runJavaScriptConfirm(WebCore::Frame*, const WebCore::String& message)
{
    ASSERT(m_pHost);
	WTF::CString messageAsUTF8(message.utf8());
	return m_pHost->m_pVTable->runJSConfirm(m_pHost, messageAsUTF8.data()) != 0;
}

bool WebChromeClient::runJavaScriptPrompt(WebCore::Frame*, const WebCore::String& prompt, const WebCore::String& defaultValue, WebCore::String& resultStr)
{
    ASSERT(m_pHost);
    WTF::CString promptAsUTF8(prompt.utf8());
    WTF::CString defaultValueAsUTF8(defaultValue.utf8());
    char* pResultUTF8 = 0;
    bool result = m_pHost->m_pVTable->runJSPrompt(m_pHost, promptAsUTF8.data(), defaultValueAsUTF8.data(), &pResultUTF8) != 0;
    if (result) {
        ASSERT(pResultUTF8);
        const WebCore::TextEncoding& utf8Encoding = WebCore::UTF8Encoding();
        size_t const resultByteLen = strlen(pResultUTF8);
        resultStr = utf8Encoding.decode(pResultUTF8, resultByteLen);

        ASSERT(g_HostFunctions);
        ASSERT(g_HostFunctions->freeBytes);
        g_HostFunctions->freeBytes( pResultUTF8 );
    }

	return result;
}


void WebChromeClient::setStatusbarText(const WebCore::String& statusText)
{
	if (m_pHost) {
		ASSERT(sizeof(uint16_t) == sizeof(UChar));
		const uint16_t* const statusUTF16 = reinterpret_cast<const uint16_t*>(statusText.characters());
		unsigned long const numStatusCodeUnits = statusText.length();
        m_pHost->m_pVTable->setStatusText(m_pHost, statusUTF16, numStatusCodeUnits);
    }
}

bool WebChromeClient::shouldInterruptJavaScript()
{
	return false;
}

bool WebChromeClient::tabsToLinks() const
{
	return true;
}

WebCore::IntRect WebChromeClient::windowResizerRect() const
{
	return WebCore::IntRect();
}

void WebChromeClient::mouseDidMoveOverElement(const WebCore::HitTestResult&, unsigned)
{
}

void WebChromeClient::setToolTip(const WebCore::String&, WebCore::TextDirection)
{
}

void WebChromeClient::print(WebCore::Frame*)
{
}

void WebChromeClient::uncaughtJavaScriptException(const JSC::ExecState* const exec, JSC::JSValue const pExceptionValue)
{
    if(m_pHost)
    {
        WebScriptProxyVariant* const pExceptionVariant = WebCore::ApolloScriptBridging::getApolloVariantForJSValue(exec, pExceptionValue);
        ASSERT(pExceptionVariant);
        if (pExceptionVariant)
        {
            ASSERT(m_pHost);
            ASSERT(m_pHost->m_pVTable);
            ASSERT(m_pHost->m_pVTable->uncaughtJSException);
            m_pHost->m_pVTable->uncaughtJSException(m_pHost, pExceptionVariant);
            ASSERT(pExceptionVariant->m_pVTable);
            ASSERT(pExceptionVariant->m_pVTable->release);
            pExceptionVariant->m_pVTable->release(pExceptionVariant);
        }
    }
}

WebWindow* WebChromeClient::getApolloWindow()
{
    return m_pWindow;
}

void WebChromeClient::onCloseTimer(WebCore::Timer<WebChromeClient>* pTimer)
{
    ASSERT(m_pHost);
    ASSERT(m_pHost->m_pVTable);
    
    // The Host may have been destroyed before the onCloseTimer gets called
    if(m_pHost->m_pVTable)
    {
        ASSERT(m_pHost->m_pVTable->closeWindow);
        ASSERT(pTimer == &m_closeTimer);
        (void)pTimer;
        m_pHost->m_pVTable->closeWindow(m_pHost);
    }
}

bool WebChromeClient::processingUserGesture()
{
    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    ASSERT(m_pWindow->m_pVTable->processingUserGesture);
    return m_pWindow->m_pVTable->processingUserGesture(m_pWindow) != 0;
}

void WebChromeClient::startWindowedPluginUserGesture()
{
    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    ASSERT(m_pWindow->m_pVTable->startProcessingWindowedPluginUserGesture);
    m_pWindow->m_pVTable->startProcessingWindowedPluginUserGesture(m_pWindow);
}

void WebChromeClient::endWindowedPluginUserGesture()
{
    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    ASSERT(m_pWindow->m_pVTable->endProcessingWindowedPluginUserGesture);
    m_pWindow->m_pVTable->endProcessingWindowedPluginUserGesture(m_pWindow);
}

void WebChromeClient::invalidateWindow(const WebCore::IntRect& dirtyRect, bool immediate)
{
    // forward call to repaint
    repaint(dirtyRect, false /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void WebChromeClient::invalidateContentsAndWindow(const WebCore::IntRect& dirtyRect, bool immediate)
{
    repaint(dirtyRect, true /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void WebChromeClient::invalidateContentsForSlowScroll(const WebCore::IntRect& dirtyRect, bool immediate)
{
    repaint(dirtyRect, true /*contentChanged*/, immediate, true /*repaintContentOnly*/);
}

void WebChromeClient::repaint(const WebCore::IntRect& dirtyRect, 
                              bool contentChanged, 
                              bool immediate, 
                              bool repaintContentOnly)
{
    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    ASSERT(m_pWindow->m_pVTable->invalidateRect);

    int const visibleWidth = m_pWindow->m_pVTable->visibleWidth( m_pWindow );
    int const visibleHeight = m_pWindow->m_pVTable->visibleHeight( m_pWindow );
    WebCore::IntRect visibleRect( 0, 0, visibleWidth, visibleHeight );

    WebCore::IntRect adjustedDirtyRect;
    
    // host window has a function that sends a full repaint with an empty rect
    if(dirtyRect.isEmpty())
    {
        adjustedDirtyRect = visibleRect;
    }
    else
    {
        adjustedDirtyRect = dirtyRect;
        adjustedDirtyRect.intersect(visibleRect);
    }
    
    if (!adjustedDirtyRect.isEmpty()) {
        WebIntRect const invalidApolloRect = { adjustedDirtyRect.x()
                                             , adjustedDirtyRect.y()
                                             , adjustedDirtyRect.right()
                                             , adjustedDirtyRect.bottom()};
        ASSERT(invalidApolloRect.m_left >= 0);
        ASSERT(invalidApolloRect.m_right >= invalidApolloRect.m_left);
        ASSERT(invalidApolloRect.m_right <= visibleRect.width());
        ASSERT(invalidApolloRect.m_top >= 0);
        ASSERT(invalidApolloRect.m_bottom >= invalidApolloRect.m_top);
        ASSERT(invalidApolloRect.m_bottom <= visibleRect.height());

        unsigned char const nowAsWebBool = static_cast<unsigned char>( immediate );
        ASSERT((nowAsWebBool == 0 ) || (nowAsWebBool == 1 ));

        m_pWindow->m_pVTable->invalidateRect(m_pWindow, &invalidApolloRect, nowAsWebBool);
    }

    
}

void WebChromeClient::scroll(const WebCore::IntSize& scrollDelta, const WebCore::IntRect& rectToScroll, const WebCore::IntRect& clipRect)
{
    // we've set blitting to none, so this will never be called
    ASSERT(false);
}

WebCore::IntPoint WebChromeClient::screenToWindow(const WebCore::IntPoint&) const
{
    ASSERT(false);
    return WebCore::IntPoint(0, 0);
}

WebCore::IntRect WebChromeClient::windowToScreen(const WebCore::IntRect&) const
{
    ASSERT(false);
    return WebCore::IntRect(0, 0, 0, 0);
}

void WebChromeClient::runOpenPanel(WebCore::Frame*, WTF::PassRefPtr<WebCore::FileChooser> fileChooser)
{
    ASSERT(m_pHost);
    unsigned long filePathLength = 0;   
    uint16_t* filePathAsUTF16 = m_pHost->m_pVTable->openFileChooser(m_pHost, &filePathLength);
    ASSERT(sizeof(uint16_t) == sizeof(UChar));
    
    WebCore::String resultFilePath( reinterpret_cast<const UChar*>(filePathAsUTF16), filePathLength );
    
    if (filePathAsUTF16)
    {
        ASSERT(g_HostFunctions);
        ASSERT(g_HostFunctions->freeBytes);
        g_HostFunctions->freeBytes( filePathAsUTF16 );
    }
    
    fileChooser->chooseFile(resultFilePath);
}

#if OS(WINDOWS) || OS(DARWIN)
PlatformPageClient WebChromeClient::platformPageClient() const
{
    if(m_pWindow)
        return m_pWindow->m_pVTable->getPlatformWindow( m_pWindow );

    return 0;
}

#elif PLATFORM(APOLLO_UNIX)

PlatformPageClient WebChromeClient::platformPageClient() const
{
    GtkWidget *cWin = NULL;

    // On Unix, the top level window contains a VBox and that VBox in turn
    // contains a GtkLayout. All the AIR content (except platform menu bar) 
    // is a part of this GtkLayout. Therefore, we need to find the GtkLayout
    // widget here.
    if(m_pWindow)
    {
        GtkContainer *container = &((m_pWindow->m_pVTable->getPlatformWindow(m_pWindow)->bin).container);

        if(!container)
            return NULL;

        GList *children = gtk_container_get_children(container);
        if(children)
        {

            GtkWidget *vbox = (GtkWidget*)(children->data);
            GList *vboxChildren = gtk_container_get_children(GTK_CONTAINER(vbox));
            GList *child;
            for(child = vboxChildren; child != NULL; child = child->next)
            {
                GtkWidget *widget = (GtkWidget*)(child->data);
                if(GTK_WIDGET_TYPE(widget) == GTK_TYPE_LAYOUT)
                {
                    cWin = widget;
                    break;
                }
            }
            if(vboxChildren)
                g_list_free(vboxChildren);
        }
        if(children)
            g_list_free(children);
    }

    return cWin;
}
/*    
#elif OS(DARWIN)
PlatformWidget WebChromeClient::platformWindow() const
{
    //WEBKIT_MAC_INTEGRATION
    return 0;
    //end  WEBKIT_MAC_INTEGRATION
}
*/
#endif
    
void WebChromeClient::contentsSizeChanged(WebCore::Frame* frame, const WebCore::IntSize& size) const    
{
    ASSERT(frame);
    // we don't care about iframes here
    if (!frame->view() || !frame->view()->isApolloMainFrameView())
        return;

    ASSERT(m_pWindow);
    ASSERT(m_pWindow->m_pVTable);
    ASSERT(m_pWindow->m_pVTable->resizeContents);

    ASSERT(m_pHost);
    ASSERT(m_pHost->m_pVTable);
    ASSERT(m_pHost->m_pVTable->isPlatformAPIVersion);

    bool useOldBehavior = !m_pHost->m_pVTable->isPlatformAPIVersion(m_pHost, WEBKIT_APOLLO_AIR_2_0);
    
    // We prevent scrollbars in Flex Apps when the body has overflow:hidden, by
    // sending the width and the height of the htmlLoader as the content size 
    // for that type of pages. This behavior is used only for AIR 2 Apps or later.

    if (useOldBehavior || !frame->view()->hasApolloHostScrollbars() || frame->view()->canHaveScrollbars()) {
        m_pWindow->m_pVTable->resizeContents(m_pWindow, size.width(), size.height());
    } else {
        // set the content size to the size of the HTMLLoader, so that Flex
        // will not show the scrollbars
        WebCore::IntSize frameSize = frame->view()->apolloContentSize();
        m_pWindow->m_pVTable->resizeContents(m_pWindow, frameSize.width(), frameSize.height());
    }
}
    
void WebChromeClient::formStateDidChange(const WebCore::Node*)    
{
    //Safari 4.0.3 integrate: implement?
}

WTF::PassOwnPtr<WebCore::HTMLParserQuirks> WebChromeClient::createHTMLParserQuirks()
{
    //rhu - implement ?
    return 0;
}
    
void WebChromeClient::scrollRectIntoView(const WebCore::IntRect&, const WebCore::ScrollView*) const    
{
    //Safari 4.0.3 integrate: implement?
}

void WebChromeClient::scrollbarsModeDidChange() const
{
}
    
void WebChromeClient::requestGeolocationPermissionForFrame(WebCore::Frame*, WebCore::Geolocation*)   
{
    //Safari 4.0.3 integrate: implement?
}

void WebChromeClient::cancelGeolocationPermissionRequestForFrame(WebCore::Frame*, WebCore::Geolocation*)
{
}
    
bool WebChromeClient::setCursor(WebCore::PlatformCursorHandle)  
{
    //Safari 4.0.3 integrate: implement?
    return false;
}

// Asynchronous request to load an icon for specified filenames.
void WebChromeClient::chooseIconForFiles(const WTF::Vector<WebCore::String>&, WebCore::FileChooser*)
{
}
    
}
