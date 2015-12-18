/*
 * Copyright (C) 2005, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Adobe Systems Incorporated.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright 
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config.h>
#include "WebFrameImpl.h"
#include <FrameView.h>
#include <PluginView.h>
#include <EventHandler.h>
#include <HTMLFormElement.h>
#include "WebResourceRequestImpl.h"
#include "WebResourceHandleClientImpl.h"
#include "WebViewImpl.h"

#include <WebKitApollo/WebHost.h>
#include <WebKitApollo/helpers/WebHostHelper.h>
#include <Page.h>
#include <ResourceError.h>
#include <DocumentLoader.h>
#include <FrameTree.h>
#include <HistoryItem.h>
#include <apollo/proxy_obj.h>
#include "runtime_root.h"
#include <ClipboardApollo.h>
#include <ClipboardApolloHelper.h>
#include <Document.h>
#include <DocumentFragment.h>
#include <Editor.h>
#include <CharacterData.h>
#include <EditorClient.h>
#include <markup.h>
#include <ReplaceSelectionCommand.h>
#include <Pasteboard.h>
#include "ResourceHandleInternal.h"
#include "MIMETypeRegistry.h"
#include "WebPopupWindowClientImpl.h"
#include <HTMLFrameOwnerElement.h>
#include <EventNames.h>
#include <FormState.h>
#include <Element.h>

#if OS(DARWIN)
#include <mac/ApolloPDFPluginView.h>
#endif

#include "JSLock.h"
#include "ScriptController.h"
#include "ScriptSourceCode.h"
#include "ScriptValue.h"

#if ENABLE(NETSCAPE_PLUGIN_API)
#include "PluginView.h"	
#include "PluginDatabase.h"
#include "PluginInfoStore.h"
#endif

#include <RenderPart.h>
#include "FrameLoaderTypes.h"
#include "WebEditorClient.h"
#include <Logging.h>

#include "DOMWrapperWorld.h"
#include "BackForwardList.h"
#include "Chrome.h"

#include "WebMainFrameView.h"

namespace WebKitApollo {

extern WebKitAPIHostFunctions* g_HostFunctions;

namespace Private {
class WebHostImplForFrames : public WebHostHelper<WebHostImplForFrames> {
public:
    WebHostImplForFrames(::WebHost* const pParentWebHost)
        : m_pParentWebHost( pParentWebHost )
    {
    }
    
    bool isUnusable()
    {
        return false;
    }

    void updateLocation(unsigned const char* urlBytes)
    {
    }

    void loadEnd(unsigned char success)
    {
    }

    void setTitle(const uint16_t* const pUTF16Title, unsigned long const numTitleCodeUnits)
    {
    }

    void setStatusText(const uint16_t* const pUTF16Status, unsigned long const numStatusCodeUnits)
    {
    }

    WebString* getUserAgentText()
    {
        return m_pParentWebHost->m_pVTable->getUserAgentText(m_pParentWebHost);
    }

    void uncaughtJSException(struct WebScriptProxyVariant* const pExceptionVariant)
    {
        m_pParentWebHost->m_pVTable->uncaughtJSException( m_pParentWebHost, pExceptionVariant );
    }
    
    void runJSAlert(const char* pUTF8Message)
    {
        m_pParentWebHost->m_pVTable->runJSAlert(m_pParentWebHost, pUTF8Message);
    }
    
    unsigned char runJSConfirm(const char* pUTF8Message)
    {
        return m_pParentWebHost->m_pVTable->runJSConfirm(m_pParentWebHost, pUTF8Message);
    }

    unsigned char runJSPrompt(const char* pUTF8Prompt, const char* pUTF8DefaultValue, char** ppUTF8Result)
    {
        return m_pParentWebHost->m_pVTable->runJSPrompt(m_pParentWebHost, pUTF8Prompt, pUTF8DefaultValue, ppUTF8Result);
    }

    WebResourceHandle* loadResource(struct WebResourceRequest* pURLRequest, struct WebResourceHandleClient* pClient)
    {
        return m_pParentWebHost->m_pVTable->loadResource(m_pParentWebHost, pURLRequest, pClient);
    }

    void loadResourceSynchronously(struct WebResourceRequest* pURLRequest, struct WebResourceHandleClient* pClient)
    {
        m_pParentWebHost->m_pVTable->loadResourceSynchronously(m_pParentWebHost, pURLRequest, pClient);
    }

    void handleOnLoadEvents()
    {
    }

    void handleOnDocumentCreated()
    {
    }

    WebViewApollo* createNewWindow(struct WebHostCreateWindowArgs* windowArgs)
    {
        return m_pParentWebHost->m_pVTable->createNewWindow(m_pParentWebHost, windowArgs);
    }

    void closeWindow()
    {
        return m_pParentWebHost->m_pVTable->closeWindow(m_pParentWebHost);
    }

    void beginModal()
    {
        return m_pParentWebHost->m_pVTable->beginModal(m_pParentWebHost);
    }

    void endModal()
    {
        return m_pParentWebHost->m_pVTable->endModal(m_pParentWebHost);
    }

    int pdfCapability()
    {
        return m_pParentWebHost->m_pVTable->pdfCapability( m_pParentWebHost );
    }
    
    int loadPDFPlugin( void **pPDFPluginObject )
    {
        return m_pParentWebHost->m_pVTable->loadPDFPlugin( m_pParentWebHost, pPDFPluginObject );
    }
    uint16_t* openFileChooser(unsigned long* pFilePathLength)
    {
        return m_pParentWebHost->m_pVTable->openFileChooser(m_pParentWebHost, pFilePathLength);
    }

    int unloadPDFPlugin( void *pdfPluginObject )
    {
        return m_pParentWebHost->m_pVTable->unloadPDFPlugin( m_pParentWebHost, pdfPluginObject );
    }

    void handlePDFError( int pdfErrorNum )
    {
        m_pParentWebHost->m_pVTable->handlePDFError( m_pParentWebHost, pdfErrorNum );
    }

    WebScriptProxyVariant* getEventObject() const
    {
        return m_pParentWebHost->m_pVTable->getEventObject( m_pParentWebHost );
    }

    WebScriptProxyVariant* getRootPackage() const
    {
        return m_pParentWebHost->m_pVTable->getRootPackage( m_pParentWebHost );
    }

    WebScriptProxyVariant* getWKMethods() const
    {
        return m_pParentWebHost->m_pVTable->getWKMethods( m_pParentWebHost );
    }

    WebScriptProxyVariant* makeDoor(WebScriptProxyVariant* pVariant) const
    {
        return m_pParentWebHost->m_pVTable->makeDoor( m_pParentWebHost, pVariant );
    }

    WebScriptProxyVariant* getHtmlControl() const
    {
        // no htmlcontrol for subframes!
        return 0;
    }

    void setMouseCursor( WebCursorType cursorType )
    {
        m_pParentWebHost->m_pVTable->setMouseCursor( m_pParentWebHost, cursorType );
    }

    void setMouseCapture( )
    {
        m_pParentWebHost->m_pVTable->setMouseCapture( m_pParentWebHost );
    }

    void releaseMouseCapture( )
    {
        m_pParentWebHost->m_pVTable->releaseMouseCapture( m_pParentWebHost );
    }

    void compositionSelectionChanged(int start, int end)
    {
        m_pParentWebHost->m_pVTable->compositionSelectionChanged(m_pParentWebHost, start, end);
    }

    void compositionAbandoned()
    {
        m_pParentWebHost->m_pVTable->compositionAbandoned(m_pParentWebHost);
    }

    void setInputMethodState(bool enable)
    {
        m_pParentWebHost->m_pVTable->setInputMethodState(m_pParentWebHost, enable);
    }
	
	bool getInputMethodState()
	{
		return m_pParentWebHost->m_pVTable->getInputMethodState(m_pParentWebHost);
	}

    void activateIMEForPlugin()
    {
        m_pParentWebHost->m_pVTable->activateIMEForPlugin(m_pParentWebHost);
    }

    void deactivateIMEForPlugin()
    {
        m_pParentWebHost->m_pVTable->deactivateIMEForPlugin(m_pParentWebHost);
    }

    void selectionChanged()
    {
        m_pParentWebHost->m_pVTable->selectionChanged(m_pParentWebHost);
    }

    const uint16_t* getAppResourcePath( unsigned long* pAppResourcePathLength )
    {
        return m_pParentWebHost->m_pVTable->getAppResourcePath( m_pParentWebHost, pAppResourcePathLength  );
    }

    const uint16_t* getAppStoragePath( unsigned long* pAppStoragePathLength )
    {
        return m_pParentWebHost->m_pVTable->getAppStoragePath( m_pParentWebHost, pAppStoragePathLength  );
    }

    WebPopupWindow* createPopupWindow( WebPopupWindowClient* popupWindowClient, float computedFontSize )
    {
        return m_pParentWebHost->m_pVTable->createPopupWindow( m_pParentWebHost, popupWindowClient, computedFontSize );
    }

    WebBitmap* createBitmap( unsigned long width, unsigned long height )
    {
        return m_pParentWebHost->m_pVTable->createBitmap( m_pParentWebHost, width, height );
    }

    WebBitmap* createBitmapFromBitmapDataObject( WebScriptProxyVariant* bitmapDataVariant )
    {
        return m_pParentWebHost->m_pVTable->createBitmapFromBitmapDataObject( m_pParentWebHost, bitmapDataVariant );
    }
    
    WebNavigationPolicyAction dispatchDecidePolicyForNavigationAction(WebString*, WebNavigationTypeApollo, WebResourceRequest*)
    {
        return WebNavigationPolicyActionUse;
    }
    
    WebNavigationPolicyAction dispatchDecidePolicyForNewWindowAction(unsigned const char*, unsigned long, WebNavigationTypeApollo, WebResourceRequest*, const uint16_t*, unsigned long)
    {
        return WebNavigationPolicyActionUse;
    }

    // Subframes currently have a null interpreter context
    void* getPlatformInterpreterContext() const { return 0; }

    WebString* getLanguage()
    {
        return m_pParentWebHost->m_pVTable->getLanguage( m_pParentWebHost );
    }

    void canShowPlugins(bool* canShowPlugins, bool* canShowWindowedPlugins)
    {
        return m_pParentWebHost->m_pVTable->canShowPlugins(m_pParentWebHost, canShowPlugins, canShowWindowedPlugins);
    }

	bool enterPlayer(WebHostEnterPlayerCallback callback, void* arg) 
	{ 
		return m_pParentWebHost->m_pVTable->enterPlayer(m_pParentWebHost,callback,arg);
	}

	bool isPlatformAPIVersion(uint32_t airVersionToCheck )
	{
		return m_pParentWebHost->m_pVTable->isPlatformAPIVersion(m_pParentWebHost,airVersionToCheck);
	}


private:
    ::WebHost* const m_pParentWebHost;
};
}
namespace {
#ifndef NDEBUG
WTFLogChannel LogWebFrameLeaks =  { 0x00000000, "", WTFLogChannelOn };

struct WebFrameCounter { 
    static int count; 
    ~WebFrameCounter() 
    { 
        if (count)
            LOG(WebFrameLeaks, "LEAK: %d WebFrame\n", count);
    }
};
int WebFrameCounter::count = 0;
static WebFrameCounter frameCounter;
#endif
}

namespace {
static const char resourceErrorDomain[] = "WebKitApollo";
static const int cancelledErrorCode = 1;
static const int blockedErrorCode = 2;
static const int cannotShowURLErrorCode = 3;
static const int interruptForPolicyChangeErrorCode = 4;
static const int cannotShowMIMETypeErrorCode = 5;
static const int fileDoesNotExistErrorCode = 6;
static const int pluginWillHandleLoadErrorCode = 7;
}

WebFrameImpl::WebFrameImpl(WebViewImpl* const pOwningWebView, WebHost* const pWebHost, WebCore::Page* const pPage)
    : m_pOwningWebView(pOwningWebView)
    , m_pWebHost(pWebHost)
    , m_pFrame(WebCore::Frame::create(pPage, 0, this).get()) // just a raw pointer, no strong ref.
    , m_dead(false)
    , m_canTransformToRootContent(true)
    , m_pluginView(0)
    , m_didRedirectDataToPlugin(false)
    , m_firstLayoutForFrameView(true)
{
    // Frame was constructed with ref count of one
    // because page has a reference to it.
    // ASSERT that the page's ref is the only ref so far.
    // ASSERT that the page in fact thinks our frame
    // is the main frame.
    ASSERT(m_pFrame->hasOneRef());
    ASSERT(pPage->mainFrame() == m_pFrame);
    
    ASSERT(m_pOwningWebView);
#ifndef NDEBUG
    ++WebFrameCounter::count;
#endif
}

WebFrameImpl::WebFrameImpl(WebViewImpl* const pOwningWebView, WebHost* const pWebHost, WebCore::HTMLFrameOwnerElement* pOwnerElement, WebFrameImpl* const pParentWebFrame)
    : m_pOwningWebView(pOwningWebView)
    , m_pWebHost(pWebHost)
    , m_pFrame(WebCore::Frame::create(pParentWebFrame->frame()->page(), pOwnerElement, this).releaseRef()) // just a raw pointer, no strong ref.
    , m_dead(false)
    , m_canTransformToRootContent(false)
    , m_pluginView(0)
    , m_didRedirectDataToPlugin(false)
    , m_firstLayoutForFrameView(true)
{
    // Frame was constructed with ref count of one
    // because that is the way webcore has done it for a while.
    // No body else has a ref to the frame yet, so
    // we can't dec the refcount just yet.
    // The refcount will remain off by one until we call WebCore::Frame::init from createFrame ( which is what calls this constructor ).
    // After calling WebCore::Frame::init, createFrame will assert that the refcount on the WebCore::Frame is no longer 1, and then
    // dec the refcount to resume refcount sanity.
    ASSERT(m_pFrame->hasOneRef());
    // This frame should not be the main frame.
    ASSERT(pParentWebFrame->frame()->page()->mainFrame() != m_pFrame);
#ifndef NDEBUG
    ++WebFrameCounter::count;
#endif
}

WebFrameImpl::~WebFrameImpl()
{
    // Just so you know, the WebViewImpl that m_pOwningWebView points to may be destroyed
    // by the time we get to this point.  This is because of the whole WebCore::Frame life support
    // business.  I don't set m_pOwningWebView to 0 from the WebViewImpl's destructor because
    // I'd prefer that m_pOwningWebView be const.
    m_dead = true;
    
    // Tell our scrollview impl that we are dead now.
    // ach: reimplement dead mechanism
    //if (m_scrollViewImpl)
    //    m_scrollViewImpl->owningWebFrameIsDead();

#ifndef NDEBUG
    --WebFrameCounter::count;
#endif
}

WebFrameImpl*
    WebFrameImpl::construct( WebViewImpl* const pOwningWebView
                           , WebHost* const pWebHost
                           , WebCore::Page* const pPage)
{
    return new WebFrameImpl(pOwningWebView, pWebHost, pPage);
}

void WebFrameImpl::transformToRootContent(WebWindow* const pWebWindow )
{
    ASSERT(m_canTransformToRootContent );
    ASSERT(frame());
    ASSERT(frame()->view());
    ASSERT(pWebWindow);
    ASSERT(pWebWindow->m_pVTable);
    m_canTransformToRootContent = false;
    // root content doesn't have host scrollbars, so allow WebKit to manage the scrollbars itself
    WTF::RefPtr<WebCore::FrameView> frameView(frame()->view());
    WebMainFrameView* webMainFrameView = static_cast<WebMainFrameView*> (frameView.get());
    webMainFrameView->setHasHostScrollbars(false);
}

void WebFrameImpl::layoutRecursive()
{
    forceLayout();
    WebCore::Frame* const coreFrame = frame();
    ASSERT(coreFrame);
    ASSERT(coreFrame->tree());
    WebCore::Frame* currChildFrame = coreFrame->tree()->firstChild();
    while (currChildFrame) {
        kit(currChildFrame)->layoutRecursive();
        currChildFrame = currChildFrame->tree()->nextSibling();
    }
}

namespace {
    inline float getPageScrollFraction()
    {
        static const float fraction = 0.75f;
        return fraction;
    }
}

bool WebFrameImpl::pageUp()
{
    WebCore::Frame* const pFrame = frame();
    ASSERT(pFrame);
    WebCore::EventHandler* const eventHandler = pFrame->eventHandler();
    ASSERT(eventHandler);
    bool handled = eventHandler->scrollOverflow(WebCore::ScrollUp, WebCore::ScrollByPage);
    if (!handled) {
        WebCore::FrameView* const frameView = pFrame->view();
        ASSERT(frameView);
        float const viewportHeight = static_cast<float>(frameView->visibleHeight());
        float const scrollFraction = getPageScrollFraction();
        int const vScrollAmount = - static_cast<int>(viewportHeight * scrollFraction);
        WebCore::IntSize const originalScrollOffset = frameView->scrollOffset();
        frameView->scrollBy(WebCore::IntSize(0, vScrollAmount));
        WebCore::IntSize const newScrollOffset = frameView->scrollOffset();
        handled = originalScrollOffset != newScrollOffset;
    }
    return handled;
}

bool WebFrameImpl::pageDown()
{
    WebCore::Frame* const pFrame = frame();
    ASSERT(pFrame);
    WebCore::EventHandler* const eventHandler = pFrame->eventHandler();
    ASSERT(eventHandler);
    bool handled = eventHandler->scrollOverflow(WebCore::ScrollDown, WebCore::ScrollByPage);
    if (!handled) {
        WebCore::FrameView* const frameView = pFrame->view();
        ASSERT(frameView);
        float const viewportHeight = static_cast<float>(frameView->visibleHeight());
        float const scrollFraction = getPageScrollFraction();
        int const vScrollAmount = - static_cast<int>(viewportHeight * scrollFraction);
        WebCore::IntSize const originalScrollOffset = frameView->scrollOffset();
        frameView->scrollBy(WebCore::IntSize(0, vScrollAmount));
        WebCore::IntSize const newScrollOffset = frameView->scrollOffset();
        handled = originalScrollOffset != newScrollOffset;
    }
    return handled; 
}

WebCore::Frame* WebFrameImpl::frame() const
{
    ASSERT(m_pFrame);
    return m_pFrame;
}

void WebFrameImpl::beginModal( ) const
{
    ASSERT(m_pWebHost);
    m_pWebHost->m_pVTable->beginModal( m_pWebHost );
}

void WebFrameImpl::endModal( ) const
{
    ASSERT(m_pWebHost);
    m_pWebHost->m_pVTable->endModal( m_pWebHost );
}

int WebFrameImpl::pdfCapability() const
{
    return m_pWebHost->m_pVTable->pdfCapability( m_pWebHost );
}

int WebFrameImpl::loadPDFPlugin( void **pPDFPluginObject ) const
{
    return m_pWebHost->m_pVTable->loadPDFPlugin( m_pWebHost, pPDFPluginObject );
}

int WebFrameImpl::unloadPDFPlugin( void *pdfPluginObject ) const
{
    return m_pWebHost->m_pVTable->unloadPDFPlugin( m_pWebHost, pdfPluginObject );
}

void WebFrameImpl::handlePDFError( int pdfErrorNum ) const
{
    m_pWebHost->m_pVTable->handlePDFError( m_pWebHost, pdfErrorNum );
}
    
WebScriptProxyVariant* WebFrameImpl::getEventObject() const
{
    return m_pWebHost->m_pVTable->getEventObject(m_pWebHost);
}

const uint16_t* WebFrameImpl::getAppResourcePath( unsigned long* pAppResourcePathLength )
{
    return m_pWebHost->m_pVTable->getAppResourcePath(m_pWebHost, pAppResourcePathLength);
}

const uint16_t* WebFrameImpl::getAppStoragePath( unsigned long* pAppStoragePathLength )
{
    return m_pWebHost->m_pVTable->getAppStoragePath(m_pWebHost, pAppStoragePathLength);
}


//Methods for ApolloWebFrame interface
void WebFrameImpl::loadHTMLString(const char* const utf8HTMLString, size_t const numHTMLStringBytes, const unsigned char* const baseURL)
{
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->loader());
    WebCore::KURL baseKURL(WebCore::ParsedURLString, reinterpret_cast<const char*>(baseURL));
	// unmap URL. This way we are executing the code in the sandbox, as we are changing baseURL from app:/ to http://, 
	WebCore::KURL unmappedURL;
	if (WebCore::FrameLoaderClientApollo::unmapFrameUrl(m_pFrame, baseKURL, &unmappedURL))
	{
		baseKURL = WebCore::KURL(unmappedURL);
	}
    WebCore::ResourceRequest const request(baseKURL);
    WTF::RefPtr<WebCore::SharedBuffer> htmlBuffer = WebCore::SharedBuffer::create(utf8HTMLString, static_cast<int>(numHTMLStringBytes));
    WebCore::KURL const unreachableURL;
    WebCore::SubstituteData const htmlData(htmlBuffer.release(), "text/html", "UTF-8", unreachableURL);
    //mihnea integrate -> lockHistory = false?
    m_pFrame->loader()->load(request, htmlData, false);
}

void WebFrameImpl::loadRequest(const WebResourceRequest* const webRequest)
{
    WebCore::ResourceRequest request(webRequest);
    WebCore::Frame* const pFrame = frame();
    ASSERT(pFrame);
    WebCore::FrameLoader* const loader = pFrame->loader();
    ASSERT(loader);
    //mihnea integrate -> lockHistory = false?
    loader->load(request, false);
}

void WebFrameImpl::evalJavaScriptString(const uint16_t* utf16Text, unsigned long numUTF16CodeUnits)
{
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->loader());
	WebCore::String text(reinterpret_cast<const UChar*>(utf16Text), numUTF16CodeUnits);
	m_pFrame->script()->evaluate(WebCore::ScriptSourceCode(text));
}

void WebFrameImpl::reload( )
{
    ASSERT(frame());
    ASSERT(frame()->loader());
    frame()->loader()->reload();
}

void WebFrameImpl::stopLoading( )
{
    frame()->loader()->stop();
}

WebScriptProxyVariant* WebFrameImpl::getGlobalObject()
{
    return WebCore::ApolloScriptBridging::getGlobalObject(frame());
}

void WebFrameImpl::historyGo( int steps )
{
    frame()->redirectScheduler()->scheduleHistoryNavigation( steps );
}

unsigned WebFrameImpl::getHistoryLength()
{
    WebCore::BackForwardList *list = frame()->page()->backForwardList();
    return static_cast<unsigned>(list->entries().size());
}

unsigned WebFrameImpl::getHistoryPosition()
{
    WebCore::BackForwardList *list = frame()->page()->backForwardList();
    return list->backListCount();
}

void WebFrameImpl::setHistoryPosition( unsigned position )
{
    WebCore::BackForwardList *list = frame()->page()->backForwardList();
    if (position < getHistoryLength())
        frame()->page()->goBackOrForward( position - list->backListCount() );
}

void WebFrameImpl::getHistoryAt( unsigned position, char** pUrl, char** pOriginalUrl, bool* pIsPost, uint16_t** pTitle )
{
    using namespace WebCore;
    BackForwardList *list = frame()->page()->backForwardList();
    int distance = position - list->backListCount();
    HistoryItem* item = list->itemAtIndex(distance);
    if (!item) {
        if (distance > 0) {
            int forwardListCount = list->forwardListCount();
            if (forwardListCount > 0)
                item = list->itemAtIndex(forwardListCount);
        } else {
            int backListCount = list->backListCount();
            if (backListCount > 0)
                item = list->itemAtIndex(-backListCount);
        }
    }

    // indicates failure by NOT initializing *pUrl.
    if (item)
    {
        String url = item->url();
        *pUrl = (char *)g_HostFunctions->allocBytes( url.length()+1 );
        if (url.length() > 0)
            memcpy(*pUrl, url.utf8().data(), url.length()+1);
        else
            **pUrl = 0;

        url = item->originalURL();
        *pOriginalUrl = (char *)g_HostFunctions->allocBytes( url.length()+1 );
        if (url.length() > 0)
            memcpy(*pOriginalUrl, url.utf8().data(), url.length()+1);
        else
            **pOriginalUrl = 0;

        const String &title = item->title();
        *pTitle = (uint16_t *)g_HostFunctions->allocBytes( sizeof(uint16_t)*(title.length()+1) );
        if (title.length() > 0)
            memcpy(*pTitle, title.characters(), sizeof(uint16_t)*(title.length()));
        (*pTitle)[title.length()] = 0;

        *pIsPost = (item->formData() != 0);
    }
}

        
void WebFrameImpl::frameLoaderDestroyed()
{
    delete this;
}
        
bool WebFrameImpl::hasWebView() const
{
    return true;
}

bool WebFrameImpl::hasFrameView() const
{
    ASSERT(m_pFrame);
    return m_pFrame->view() != 0;
}

void WebFrameImpl::makeRepresentation(WebCore::DocumentLoader*)
{
}

void WebFrameImpl::transitionToCommittedForNewPage()
{
    WTF::RefPtr<WebCore::FrameView> oldFrameView(m_pFrame->view());
        
    // Since the old frame view is going to die in short order, we need to see if we have
    // data redirection to a plugin pending.  If we do, we need to tell the plugin the stream
    // had an error and clear the data redirection.  We tell the plugin that the stream
    // had an error so that the stream will be in the correct state when it is destroyed.
    
    if (m_pluginView) {
        // The plugin to which we were redirecting to should only exist if we previously had a frame view.
        ASSERT(oldFrameView);
        
        WebCore::ResourceError const resourceError;
        m_pluginView->didFail(resourceError);

        clearPluginViewRedirect();
    }

    bool mainFrameHasHostScrollbars = true;

    // If this isn't the main frame, it must have a owner element set, or it
    // won't ever get installed in the view hierarchy.
    bool const isMainFrame = m_pFrame == m_pFrame->page()->mainFrame();
    ASSERT(isMainFrame || (m_pFrame->ownerElement()));
    if (oldFrameView) {
		if (isMainFrame)
        {
            WebMainFrameView* oldWebMainFrameView = static_cast<WebMainFrameView*> (oldFrameView.get());
            mainFrameHasHostScrollbars = oldWebMainFrameView->hasApolloHostScrollbars();
			oldFrameView->setParentVisible(false);
        }

        m_pFrame->setView(0);
    }
    
    WTF::RefPtr<WebCore::FrameView> frameView;
    if(isMainFrame)
    {
        frameView = adoptRef(new WebMainFrameView(m_pFrame, m_pOwningWebView->getTargetWindow()));
        WebMainFrameView* newWebMainFrameView = static_cast<WebMainFrameView*> (frameView.get());
        newWebMainFrameView->setHasHostScrollbars(mainFrameHasHostScrollbars);
    }
    else
    {
        frameView = WebCore::FrameView::create(m_pFrame);
    }
    ASSERT(frameView->hasOneRef());
    
    ASSERT(m_pFrame);
    // frame now holds strong reference on frameView we just constructed.
    m_pFrame->setView(frameView.get());
    ASSERT(!(frameView->hasOneRef()));

    // Added this code to fix bug [1603295] - HTML: setting systemChrome=none / transparent=true doesn't make the app transparent
    //
    if ((oldFrameView) && (oldFrameView->baseBackgroundColor().alpha() < 0xff))
        frameView->setBaseBackgroundColor( oldFrameView->baseBackgroundColor() );
    
	if (isMainFrame)
        frameView->setParentVisible(true);

    // This is kooky code from WebCoreFrameBridge.mm, installInFrame.
    // This code is important because if we are making a new WebCore::FrameView
    // for a WebCore::Frame that already had a WebCore::FrameView and that frame
    // was a subframe, the renderer for the frame element in the DOM holds a reference
    // to the old WebCore::FrameView.  This code updates that reference in the renderer
    // to point at the new WebCore::FrameView we just made.  God knows why it works this
    // way.  Hopefully somebody will change this such that we either do not make new FrameView's
    // all the time or at least do a better job of encapsulating this mess in WebCore.
    if (m_pFrame->ownerRenderer())
        m_pFrame->ownerRenderer()->setWidget(frameView.get());
    
    if (WebCore::HTMLFrameOwnerElement* frameOwner = m_pFrame->ownerElement())
        m_pFrame->view()->setScrollbarModes(frameOwner->scrollingMode(), frameOwner->scrollingMode());

    // Fix Watson #2986160
    // updating according to http://trac.webkit.org/changeset/50665
    // More details in https://bugs.webkit.org/show_bug.cgi?id=29167
    // In short, the initScrollbars function which was called here before has
    // been replaced by updateCanHaveScrollbars, abstracting some scrollbar
    // state info from the FrameView (into ScrollView)
    // Update the canHaveScrollbars flag here, where the scrolling mode has just
    // been set. When laying out the ScrollView, the layout process adds
    // scrollbars, which without this flag are never disabled afterwards.
    frameView->updateCanHaveScrollbars();


    // Once we have called setWidget on the frame's associated
    // renderer ( if it had one ), then the last ref on the
    // old FrameView should be the RefPtr we made at the top of this
    // method.  If this assert fails then something about
    // how we manage FrameView's is wrong.  It is a delicate
    // dance and must always be inferred from PLATFORM(MAC).
    ASSERT((!oldFrameView) || (oldFrameView->hasOneRef()));
    
    // ach: check the following assert
    //ASSERT((!oldFrameView) || (!oldFrameView->getApolloImpl()->getParent()));
    // If we clear this scope's reference to the oldFrameView it will be destroyed
    // at which point the only hard reference left to the old scrollview impl will be
    // the hard reference in this functions scope.
    oldFrameView.clear();

    // Any time we change out the frameView, we need to set m_firstLayoutForFrameView to true.
    m_firstLayoutForFrameView = true;

    
    
}

void WebFrameImpl::savePlatformDataToCachedFrame(WebCore::CachedFrame*)
{
}

void WebFrameImpl::transitionToCommittedFromCachedFrame(WebCore::CachedFrame*)
{
}

void WebFrameImpl::forceLayout()
{
    WebCore::Frame* const theFrame = frame();
    ASSERT(theFrame);
    ASSERT(hasFrameView());
    
    WebCore::FrameView* const frameView = theFrame->view();
    ASSERT(frameView);

    frameView->forceLayout(true);
    ASSERT(!frameView->needsLayout());
    WebCore::IntSize const currFrameSize(frameView->frameRect().size());
    
    if ((!m_firstLayoutForFrameView) && (currFrameSize != m_lastLayoutSize)) {
        theFrame->eventHandler()->sendResizeEvent();
        ASSERT(theFrame->view() == frameView);
        if (frameView->needsLayout())
            frameView->forceLayout(true);
    }
    
    m_lastLayoutSize = currFrameSize;
    m_firstLayoutForFrameView = false;
    
    ASSERT(!frameView->needsLayout());
}

void WebFrameImpl::forceLayoutForNonHTML()
{
}

void WebFrameImpl::setCopiesOnScroll()
{
}

void WebFrameImpl::detachedFromParent2()
{
}

void WebFrameImpl::detachedFromParent3()
{
}

void WebFrameImpl::detachedFromParent4()
{
}

void WebFrameImpl::loadedFromPageCache()
{
}

void WebFrameImpl::assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&)
{
}

void WebFrameImpl::dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long identifier, WebCore::ResourceRequest&, const WebCore::ResourceResponse& redirectResponse)
{
}
    
bool WebFrameImpl::shouldUseCredentialStorage(WebCore::DocumentLoader*, unsigned long identifier)    
{
    //mihnea integrate -> implement?
    return false;
}

void WebFrameImpl::dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&)
{
}

void WebFrameImpl::dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&)
{
}

void WebFrameImpl::dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceResponse& resourceResponse)
{
}

void WebFrameImpl::dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long identifier, int lengthReceived)
{
}

void WebFrameImpl::dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long identifier)
{
}

void WebFrameImpl::dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceError&)
{
}

bool WebFrameImpl::dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int length)
{
    return false;
}

void WebFrameImpl::dispatchDidLoadResourceByXMLHttpRequest(unsigned long /* identifier */, const WebCore::ScriptString&)
{
// rhu - implement
}

void WebFrameImpl::dispatchDidHandleOnloadEvents()
{
    if (m_pWebHost) {
        ASSERT(m_pWebHost->m_pVTable);
        ASSERT(m_pWebHost->m_pVTable->handleOnLoadEvents);
        m_pWebHost->m_pVTable->handleOnLoadEvents(m_pWebHost);
    }
}

void WebFrameImpl::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
}

void WebFrameImpl::dispatchDidCancelClientRedirect()
{
}

void WebFrameImpl::dispatchWillPerformClientRedirect(const WebCore::KURL&, double interval, double fireDate)
{
}

void WebFrameImpl::dispatchDidChangeLocationWithinPage()
{
}

void WebFrameImpl::dispatchWillClose()
{
}

void WebFrameImpl::dispatchDidReceiveIcon()
{
}

void WebFrameImpl::dispatchDidStartProvisionalLoad()
{
    WebCore::Frame* const pFrame = frame();
    ASSERT(pFrame);
    ASSERT(pFrame->loader());
    ASSERT(pFrame->loader()->activeDocumentLoader());
    updateLocation(pFrame->loader()->activeDocumentLoader()->requestURL());
}

void WebFrameImpl::dispatchDidReceiveTitle(const WebCore::String& title)
{
    ASSERT(!m_dead);
    if (m_pWebHost) {
        ASSERT(sizeof(uint16_t) == sizeof(UChar));
        const uint16_t* const titleUTF16 = reinterpret_cast<const uint16_t*>(title.characters());
        unsigned long const numTitleCodeUnits = title.length();
        m_pWebHost->m_pVTable->setTitle(m_pWebHost, titleUTF16, numTitleCodeUnits);
    }
}

void WebFrameImpl::dispatchDidCommitLoad()
{
    WebCore::Frame* const theFrame = frame();
    
    // If either of these is non-null it means that windowObjectDead was not called before this method
    // was called.  We used to deadify script bridging objects in here, but that is no longer
    // soon enough in the loading process.  It is still important that the script bridging objects
    // are deadified before this method get called to make sure we never run a JS function
    // from an old html document against the current html document's object model.
    ASSERT(!m_childDoor);
    ASSERT(!m_childDoorProxy);

    theFrame->view()->setScrollPosition(WebCore::IntPoint(0, 0));
    
    ASSERT(theFrame->loader());
    ASSERT(theFrame->loader()->documentLoader());
    updateLocation(theFrame->loader()->documentLoader()->requestURL());
}

void WebFrameImpl::dispatchDidFailProvisionalLoad(const WebCore::ResourceError&)
{
}

void WebFrameImpl::dispatchDidFailLoad(const WebCore::ResourceError&)
{
}

void WebFrameImpl::dispatchDidFinishDocumentLoad()
{
}

void WebFrameImpl::dispatchDidFinishLoad()
{
}

void WebFrameImpl::dispatchDidFirstLayout()
{
}
    
void WebFrameImpl::dispatchDidFirstVisuallyNonEmptyLayout()    
{
}
    
WebCore::Frame* WebFrameImpl::dispatchCreatePage()
{
    WebHost* const host = webHost();
    ASSERT(host);
    ASSERT(host->m_pVTable);
    ASSERT(host->m_pVTable->createNewWindow);
    WebHostCreateWindowArgs createArgs = {
        sizeof(WebHostCreateWindowArgs),
        WebHostCreateWindowArgs::USE_DEFAULT,
        WebHostCreateWindowArgs::USE_DEFAULT,
        WebHostCreateWindowArgs::USE_DEFAULT,
        WebHostCreateWindowArgs::USE_DEFAULT,
        true,
        true,
        true,
        true,
        true,
        true,
        false
    };
    WebViewApollo* const webView = host->m_pVTable->createNewWindow(host, &createArgs);
    WebCore::Frame* newPageFrame = 0;
    if (webView) {
        WebViewImpl* const webViewImpl = WebViewImpl::getImpl(webView);
        WebCore::Page* const page = webViewImpl->page();
        newPageFrame = page->mainFrame();
    }
    return newPageFrame;
}

void WebFrameImpl::dispatchShow()
{
    WebCore::Frame* const pFrame = frame();
    ASSERT(pFrame);
    WebCore::Page* const page = pFrame->page();
    ASSERT(page);
    WebCore::Chrome* const chrome = page->chrome();
    chrome->show();
}

void WebFrameImpl::dispatchDecidePolicyForMIMEType(WebCore::FramePolicyFunction function, const WebCore::String& MIMEType, const WebCore::ResourceRequest&)
{
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->loader());
    (m_pFrame->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

namespace {
    // ASSERT at compile time that the WebNavigationPolicy and WebNavigationTypeApollo enumeration match
    // their webcore counterparts.  This way we can use reinterpret_cast to do the conversion between the WebCore
    // and WebKit types.
    COMPILE_ASSERT(WebCore::PolicyUse == static_cast<WebCore::PolicyAction>(WebNavigationPolicyActionUse), PolicyUseNotEqualWebNavigationPolicyActionUse);
    COMPILE_ASSERT(WebCore::PolicyIgnore == static_cast<WebCore::PolicyAction>(WebNavigationPolicyActionIgnore), PolicyIgnoreNotEqualWebNavigationPolicyActionIgnore);
    COMPILE_ASSERT(WebCore::PolicyDownload == static_cast<WebCore::PolicyAction>(WebNavigationPolicyActionDownload), PolicyDownloadNotEqualWebNavigationPolicyActionDownload);
    COMPILE_ASSERT(static_cast<WebNavigationTypeApollo>(WebCore::NavigationTypeLinkClicked) == WebNavigationTypeApolloLinkClicked, NavigationLinkClickedNotEqualWebNavigationTypeApolloLinkClicked);
    COMPILE_ASSERT(static_cast<WebNavigationTypeApollo>(WebCore::NavigationTypeFormSubmitted) == WebNavigationTypeApolloFormSubmitted, NavigationTypeFormSubmittedNotEqualWebNavigationTypeApolloFormSubmitted);
    COMPILE_ASSERT(static_cast<WebNavigationTypeApollo>(WebCore::NavigationTypeBackForward) == WebNavigationTypeApolloBackForward, NavigationTypeBackForwardNotEqualWebNavigationTypeApolloBackForward);
    COMPILE_ASSERT(static_cast<WebNavigationTypeApollo>(WebCore::NavigationTypeReload) == WebNavigationTypeApolloReload, NavigationTypeReloadNotEqualWebNavigationTypeApolloReload);
    COMPILE_ASSERT(static_cast<WebNavigationTypeApollo>(WebCore::NavigationTypeFormResubmitted) == WebNavigationTypeApolloFormResubmitted, NavigationTypeFormResubmittedNotEqualWebNavigationTypeApolloFormResubmitted);
    COMPILE_ASSERT(static_cast<WebNavigationTypeApollo>(WebCore::NavigationTypeOther) == WebNavigationTypeApolloOther, NavigationTypeOtherNotEqualWebNavigationTypeApolloOther);
}

void WebFrameImpl::dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction& navAction, const WebCore::ResourceRequest& resourceRequest, WTF::PassRefPtr<WebCore::FormState>, const WebCore::String& frameName)
{
//RHU Need to take into account formState at dispatchDecidePolicyForNewWindowAction
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->loader());
    
    WebCore::KURL const navActionURL(navAction.url());
    WTF::CString const navActionURLString(navActionURL.string().latin1());
    
    const unsigned char* navActionURLBytes = reinterpret_cast<const unsigned char*>(navActionURLString.data());
    unsigned long const navActionURLLen = navActionURLString.length();
    ASSERT((navActionURLBytes) || (navActionURLLen == 0));
    
    WebCore::NavigationType const navType = navAction.type();
    WebNavigationTypeApollo const webNavType = static_cast<WebNavigationTypeApollo>(navType);
    
    WTF::RefPtr<WebResourceRequestImpl> const webResourceRequestImpl(WebResourceRequestImpl::construct(resourceRequest));
    WebResourceRequest* const webResourceRequest = webResourceRequestImpl->getWebResourceRequest();
    
#if defined (__clang__)
#pragma clang diagnostic push
#if (__clang_major__ > 6)
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif // defined __clang__
    COMPILE_ASSERT(sizeof(uint16_t) == sizeof(UChar), sizeOfUInt16MustEqualSizeOfUChar);
#if defined (__clang__)
#pragma clang diagnostic pop
#endif // defined __clang__
    
    const uint16_t* const frameNameUTF16 = reinterpret_cast<const uint16_t*>(frameName.characters());
    unsigned long const numFrameNameUTF16CodeUnits = frameName.length();
    
    ASSERT(m_pWebHost);
    ASSERT(m_pWebHost->m_pVTable);
    ASSERT(m_pWebHost->m_pVTable->dispatchDecidePolicyForNewWindowAction);
    
    WebNavigationPolicyAction const webNavigationPolicyAction =
        m_pWebHost->m_pVTable->dispatchDecidePolicyForNewWindowAction(m_pWebHost
                                                                     , navActionURLBytes
                                                                     , navActionURLLen
                                                                     , webNavType
                                                                     , webResourceRequest
                                                                     , frameNameUTF16
                                                                     , numFrameNameUTF16CodeUnits);
                                                          
    WebCore::PolicyAction const policyAction = static_cast<WebCore::PolicyAction>(webNavigationPolicyAction);
    (m_pFrame->loader()->policyChecker()->*function)(policyAction);
}

void WebFrameImpl::dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction& navAction, const WebCore::ResourceRequest& resourceRequest, WTF::PassRefPtr<WebCore::FormState>)
{
//RHU Need to take into account formState at dispatchDecidePolicyForNavigationAction
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->loader());

    WebCore::PolicyAction policyAction = WebCore::PolicyUse;

    WebCore::NavigationType const navType = navAction.type();
    WebCore::KURL const navActionURL(navAction.url());

    switch (navType)
    {
    case WebCore::NavigationTypeLinkClicked:
    case WebCore::NavigationTypeFormSubmitted:
    case WebCore::NavigationTypeOther:          // includes window.open case and HTMLLoader.load() case
        {
            FrameLoaderClientApollo *client = static_cast<FrameLoaderClientApollo *>(m_pFrame->loader()->client());
            // If there's a referrer, check that.
            // Otherwise, this is one of thise situations (HTMLLoader API call, etc.)
            // where the request should be allowed irrespective of the current page.
            WebCore::String const referrer(resourceRequest.httpReferrer());
            if (!referrer.isEmpty()) {
                if (!client->canLoad(navActionURL, referrer)) {
                    policyAction = WebCore::PolicyIgnore;
                }
            }

            // Now we prevent opened windows from escalating to local or app content
            if (WebCore::Frame *opener = m_pFrame->loader()->opener()) {
                if (WebCore::Document *doc = opener->document()) {
                    if (!client->canLoad(navActionURL, doc))
                        policyAction = WebCore::PolicyIgnore;
                }
            }

            // if the navigation url is null we have to stop loading and ignore
            // the request, otherwise it will crash the loading in WebCore since
            // the request created based on a null url is not valid (see Watson 2423346)
            if (navActionURL.isNull())
                policyAction = WebCore::PolicyIgnore;
        }
        break;
    case WebCore::NavigationTypeBackForward:
    case WebCore::NavigationTypeReload:
    case WebCore::NavigationTypeFormResubmitted:
    default:
        // leave PolicyAction as PolicyUse
        break;
    }

    // If the above switch statement did not block the load,
    // then call through to the WebHost and let it have a crack
    // at blocking the request.
    if (policyAction == WebCore::PolicyUse) {
        WebCore::KURL const navActionURL(navAction.url());
        
        WebCore::NavigationType const navType = navAction.type();
        WebNavigationTypeApollo const webNavType = static_cast<WebNavigationTypeApollo>(navType);
        
        WTF::RefPtr<WebResourceRequestImpl> const webResourceRequestImpl(WebResourceRequestImpl::construct(resourceRequest));
        WebResourceRequest* const webResourceRequest = webResourceRequestImpl->getWebResourceRequest();
        
        ASSERT(m_pWebHost);
        ASSERT(m_pWebHost->m_pVTable);
        ASSERT(m_pWebHost->m_pVTable->dispatchDecidePolicyForNewWindowAction);
        
		WebString* navActionURLString = navActionURL.string().webString();
        WebNavigationPolicyAction const webNavigationPolicyAction =
            m_pWebHost->m_pVTable->dispatchDecidePolicyForNavigationAction(m_pWebHost
                                                                          , navActionURLString
                                                                          , webNavType
                                                                          , webResourceRequest);
		navActionURLString->m_vTable->free(navActionURLString);
        policyAction = static_cast<WebCore::PolicyAction>(webNavigationPolicyAction);

        // i am not blocking mailto protocol above, because i want to process mailto
        // above webkit
        if (navActionURL.protocolIs("mailto"))
            policyAction = WebCore::PolicyIgnore;
    }

    (m_pFrame->loader()->policyChecker()->*function)(policyAction);
}

void WebFrameImpl::cancelPolicyCheck()
{
}

void WebFrameImpl::dispatchUnableToImplementPolicy(const WebCore::ResourceError&)
{
}

void WebFrameImpl::dispatchWillSubmitForm(WebCore::FramePolicyFunction function, WTF::PassRefPtr<WebCore::FormState>)
{
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->loader());
    (m_pFrame->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

void WebFrameImpl::dispatchDidLoadMainResource(WebCore::DocumentLoader*)
{
}

void WebFrameImpl::revertToProvisionalState(WebCore::DocumentLoader*)
{
}

void WebFrameImpl::setMainDocumentError(WebCore::DocumentLoader* loader, const WebCore::ResourceError& error)
{
    bool hadPluginView = m_pluginView != NULL;
    
    if (m_pluginView && m_pluginView->status() == WebCore::PluginStatusLoadedSuccessfully)
        m_pluginView->didFail(error);
    
    clearPluginViewRedirect();

	// do not send policy errors, as they are treated in HTMLLoader before being sent here
    if (!hadPluginView && error.errorCode() != interruptForPolicyChangeErrorCode && error.errorCode() != cancelledErrorCode)
    {
		ASSERT(m_pWebHost);
		ASSERT(m_pWebHost->m_pVTable);
		
		m_pWebHost->m_pVTable->loadEnd(m_pWebHost, false);
    }
}

void WebFrameImpl::postProgressStartedNotification()
{
}

void WebFrameImpl::postProgressEstimateChangedNotification()
{
}

void WebFrameImpl::postProgressFinishedNotification()
{
}

void WebFrameImpl::setMainFrameDocumentReady(bool)
{
}

void WebFrameImpl::startDownload(const WebCore::ResourceRequest&)
{
}

void WebFrameImpl::willChangeTitle(WebCore::DocumentLoader*)
{
}

void WebFrameImpl::didChangeTitle(WebCore::DocumentLoader*)
{
}

void WebFrameImpl::committedLoad(WebCore::DocumentLoader* loader, const char* bytes, int length)
{
    if(m_pluginView)
    {
        m_pluginView->didReceiveData(bytes, length);
        return;
    }
    else
    {
        WebCore::Frame* const pFrame = frame();
        ASSERT(pFrame);
        WebCore::FrameLoader* const pFrameLoader = pFrame->loader();
        ASSERT(pFrameLoader);
		const WebCore::SubstituteData& substituteData = pFrameLoader->documentLoader()->substituteData();
		WebCore::String encoding;
		bool userChosen;
		if ( substituteData.isValid() )
		{
			encoding = substituteData.textEncoding();
			userChosen = false;
		}
		else
		{
			encoding = pFrameLoader->documentLoader()->overrideEncoding();
			userChosen = ! encoding.isNull();
			if (encoding.isNull())
				encoding = pFrameLoader->documentLoader()->response().textEncodingName();
		}
		pFrameLoader->writer()->setEncoding(encoding, userChosen);
		pFrameLoader->addData( bytes, length );
    }

    // we may now be sending data to plug-in, so check again

    if (m_pluginView)
        m_pluginView->didReceiveData(bytes, length);
    else if (m_didRedirectDataToPlugin)
    {
        WebCore::Frame* const pFrame = frame();
        ASSERT(pFrame);
        WebCore::FrameLoader* const pFrameLoader = pFrame->loader();
        ASSERT(pFrameLoader);
        WebCore::DocumentLoader* const pDocLoader = pFrameLoader->activeDocumentLoader();
        ASSERT(pDocLoader);
        WebCore::ResourceError const loadError;
        pDocLoader->cancelMainResourceLoad(loadError);
        clearPluginViewRedirect();
    }
}

void WebFrameImpl::finishedLoading(WebCore::DocumentLoader* loader)
{
    if(m_pluginView) {
        ASSERT(m_didRedirectDataToPlugin);
        // If there we are redirect of data to a plugin, time to tell the plugin no more data is coming.
        m_pluginView->didFinishLoading();
        clearPluginViewRedirect();// Don't need to vector data to the plug-in anymore
    }
    else
    {
        WebCore::FrameLoader* const pFrameLoader = frame()->loader();
        ASSERT(pFrameLoader);
        ASSERT(pFrameLoader->documentLoader());
        // Get the encoding and set it back because FrameLoader really wants setEncoding to be called
        // before any calls to addData.  Calling setEncoding more than needed is harmless.
 		const WebCore::SubstituteData& substituteData = pFrameLoader->documentLoader()->substituteData();
		WebCore::String encoding;
		bool userChosen;
		if ( substituteData.isValid() )
		{
			encoding = substituteData.textEncoding();
			userChosen = false;
		}
		else
		{
			encoding = pFrameLoader->documentLoader()->overrideEncoding();
			userChosen = ! encoding.isNull();
			if (encoding.isNull())
				encoding = pFrameLoader->documentLoader()->response().textEncodingName();
		}

		pFrameLoader->writer()->setEncoding(encoding, userChosen);

        m_pFrame->loader()->addData( 0, 0 );
    }
}

void WebFrameImpl::updateGlobalHistory()
{
}

void WebFrameImpl::updateGlobalHistoryRedirectLinks()
{
    //rhu - implement ?
}
    
void WebFrameImpl::updateGlobalHistoryForRedirectWithoutHistoryItem()    
{
    //mihnea integrate -> implement?
}
    
bool WebFrameImpl::shouldGoToHistoryItem(WebCore::HistoryItem*) const
{
    return true;
}

WebCore::ResourceError WebFrameImpl::cancelledError(const WebCore::ResourceRequest& req)
{
    return WebCore::ResourceError(resourceErrorDomain, cancelledErrorCode, req.url().string(), WebCore::String());
}

WebCore::ResourceError WebFrameImpl::blockedError(const WebCore::ResourceRequest& req)
{
    return WebCore::ResourceError(resourceErrorDomain, blockedErrorCode, req.url().string(), WebCore::String());
}

WebCore::ResourceError WebFrameImpl::cannotShowURLError(const WebCore::ResourceRequest& req)
{
    return WebCore::ResourceError(resourceErrorDomain, cannotShowURLErrorCode, req.url().string(), WebCore::String());
}

WebCore::ResourceError WebFrameImpl::interruptForPolicyChangeError(const WebCore::ResourceRequest& req)
{
    return WebCore::ResourceError(resourceErrorDomain, interruptForPolicyChangeErrorCode, req.url().string(), WebCore::String());
}

WebCore::ResourceError WebFrameImpl::cannotShowMIMETypeError(const WebCore::ResourceResponse& response)
{
    return WebCore::ResourceError(resourceErrorDomain, cannotShowMIMETypeErrorCode, response.url().string(), WebCore::String());
}

WebCore::ResourceError WebFrameImpl::fileDoesNotExistError(const WebCore::ResourceResponse& response)
{
    return WebCore::ResourceError(resourceErrorDomain, fileDoesNotExistErrorCode, response.url().string(), WebCore::String());
}

WebCore::ResourceError WebFrameImpl::pluginWillHandleLoadError(const WebCore::ResourceResponse& response)
{
//RHU implement pluginWillHandleLoadError
    return WebCore::ResourceError(resourceErrorDomain, pluginWillHandleLoadErrorCode, response.url().string(), WebCore::String());
}

bool WebFrameImpl::shouldFallBack(const WebCore::ResourceError&)
{
    return false;
}

bool WebFrameImpl::canHandleRequest(const WebCore::ResourceRequest&) const
{
    return true;
}

bool WebFrameImpl::canShowMIMEType(const WebCore::String& MIMEType) const
{
    if (WebCore::MIMETypeRegistry::isSupportedImageMIMEType(MIMEType)
     || WebCore::MIMETypeRegistry::isSupportedImageResourceMIMEType(MIMEType)
     || WebCore::MIMETypeRegistry::isSupportedNonImageMIMEType(MIMEType)
#if ENABLE(NETSCAPE_PLUGIN_API)
     || WebCore::PluginInfoStore::isPDFMIMEType(MIMEType)
     || WebCore::PluginInfoStore::isFlashMIMEType(MIMEType)
#endif
     )
        return true;
    return false;
}

bool WebFrameImpl::representationExistsForURLScheme(const WebCore::String& URLScheme) const
{
    return false;
}

WebCore::String WebFrameImpl::generatedMIMETypeForURLScheme(const WebCore::String& URLScheme) const
{
    return WebCore::String();
}

void WebFrameImpl::frameLoadCompleted()
{
}

void WebFrameImpl::saveViewStateToItem(WebCore::HistoryItem*)
{
}

void WebFrameImpl::restoreViewState()
{
}

void WebFrameImpl::provisionalLoadStarted()
{
}

void WebFrameImpl::didFinishLoad()
{
}

void WebFrameImpl::prepareForDataSourceReplacement()
{
}

WTF::PassRefPtr<WebCore::DocumentLoader> WebFrameImpl::createDocumentLoader(const WebCore::ResourceRequest& request, const WebCore::SubstituteData& data)
{
    RefPtr<WebCore::DocumentLoader> pDocLoader(WebCore::DocumentLoader::create(request, data));
    return pDocLoader.release();
}

void WebFrameImpl::setTitle(const WebCore::String& title, const WebCore::KURL&)
{
}

WebCore::String WebFrameImpl::userAgent(const WebCore::KURL&)
{
    ASSERT(m_pWebHost);
    ASSERT(m_pWebHost->m_pVTable);

    WebString* pUserAgent = m_pWebHost->m_pVTable->getUserAgentText(m_pWebHost);
    if (!pUserAgent)
    {
        // This should happen when the WebViewApollo is being destroyed
        // and the "unload" event triggers a new WebRequest that needs a user agent
        // header argument. The helper will return 0 because 
        // AIR is collecting garbage.
        return WebCore::String();
    }

    return WebCore::adoptWebString(pUserAgent);
}

bool WebFrameImpl::canCachePage() const
{
    return false;
}

void WebFrameImpl::download(WebCore::ResourceHandle* resourceHandle, const WebCore::ResourceRequest& resourceRequest, const WebCore::ResourceRequest&, const WebCore::ResourceResponse& resourceResponse)
{
}

WTF::PassRefPtr<WebCore::Frame> WebFrameImpl::createFrame(const WebCore::KURL& url, const WebCore::String& name, WebCore::HTMLFrameOwnerElement* ownerElement,
                                                          const WebCore::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
    ASSERT( ! m_dead );
    if (!m_pWebHostImplForChildFrames.get())
    {
        m_pWebHostImplForChildFrames =
            std::auto_ptr<Private::WebHostImplForFrames>( new Private::WebHostImplForFrames(m_pWebHost));
    }
    ASSERT(m_pWebHostImplForChildFrames.get());
    WebHost* const pWebHostForChildFrames =
        m_pWebHostImplForChildFrames->getWebHost();

    WebFrameImpl* const pChildWebFrame(new WebFrameImpl( m_pOwningWebView
                                                       , pWebHostForChildFrames
                                                       , ownerElement
                                                       , this));
    // ** EVIL **
    // child web frame points at the webcore frame, but does not own the WebCore::Frame.
    // nothing else points at the webcore frame at the momement.  The WebCore::Frame
    // is just sort of hanging out for the moment.
    // Calling the magic init method on the webcore frame will cause
    // the FrameView to be constructed which will in turn take a reference on the
    // WebCore::Frame and increment the refcount of the WebCore::Frame.
    //
    // Make sure the WebCore::Frame's refcount is one.
    WebCore::Frame* const rawNewFrame = pChildWebFrame->frame();
    ASSERT(rawNewFrame->hasOneRef());
    
    // Make sure we don't already have a frame by this name.
    ASSERT( ! isChildFrame( name ) );
    
    rawNewFrame->tree()->setName(name);
    
    // Make sure the WebCore::Frame's refcount is one.
    ASSERT(rawNewFrame->hasOneRef());
    
    ASSERT(!rawNewFrame->view());

    //Watson 1910885: FrameTree crash
	//we have to insert the frame into the tree before init 
	//<rdar://problem/5823684> - Crash manipulating frame tree of a new frame before the new frame
    //    has been installed in a frame tree.
    //    The root of this problem was that calling init() on a new frame could end up calling arbitrary
    //    javascript that might end up removing the frame from the tree.  This opened up a small can of worms
    //    such as the frame not having yet been installed in its frame tree, and other assumed behavior while
    //    destroying the frame.	
    WTF::RefPtr<WebCore::Frame> newFrame(pChildWebFrame->frame());
    ASSERT(newFrame);
    frame()->tree()->appendChild(newFrame);
    
    // Calling init results in a call to transitionToCommittedForNewPage ( defined below ).
    // transitionToCommittedForNewPage creates the FrameView which will take a strong reference
    // to the newFrame.
    rawNewFrame->init();

    //init may trigger code to remove the frame from the tree, therefore we have
	//to check that before going further
    if (!newFrame->tree()->parent()) {
        ASSERT(!newFrame->view());  // the view should be null, because the tear down logic
                                    // that starts nuking the frame, set's the view to 0,
                                    // which should cause the view to be destroyed.
		//necessary, otherwise you will end up leaking info
		//LEAK: 1 RenderObject
		//LEAK: 1 Node
		//LEAK: 1 Frame
		//LEAK: 1 WebFrame
		//LEAK: 1 WebFrame
		rawNewFrame->deref();
        return 0;
    }

    ASSERT(rawNewFrame->view());
    ASSERT(rawNewFrame->view()->frame() == rawNewFrame);
    ASSERT(!(rawNewFrame->hasOneRef()));
    rawNewFrame->deref();
    
    newFrame->loader()->loadURLIntoChildFrame(url, referrer, newFrame.get());

    // loading the frame may have resulted in the new frame being removed from the frame tree,
    // if so we should return NULL.
    if (!newFrame->tree()->parent()) {
        ASSERT(!newFrame->view());  // the view should be null, because the tear down logic
                                    // that starts nuking the frame, set's the view to 0,
                                    // which should cause the view to be destroyed.
        return 0;
    }

    ASSERT(newFrame->view());

    return newFrame.release();
}

WTF::PassRefPtr<WebCore::Widget> WebFrameImpl::createPlugin(const WebCore::IntSize& pluginSize, WebCore::HTMLPlugInElement* element, const WebCore::KURL&url, 
                                            const WTF::Vector<WebCore::String>& paramNames, const WTF::Vector<WebCore::String>& paramValues,
                                            const WebCore::String& mimeType, bool loadManually)
{
#if ENABLE(NETSCAPE_PLUGIN_API)

#if OS(DARWIN)
    // on MAC Acrobat Reader doesn't support the NPAPI
    // so we have to use a special implementation
    WebCore::String resolvedMimeType = mimeType;
    if (resolvedMimeType.isEmpty())
        resolvedMimeType = WebCore::MIMETypeRegistry::getMIMETypeForExtension(url.path().substring(url.path().reverseFind('.')+1));
    
    if (WebCore::PluginInfoStore::isPDFMIMEType(resolvedMimeType))
    {
        WTF::PassRefPtr<WebCore::ApolloPDFPluginView> pdfPluginView = WebCore::ApolloPDFPluginView::create(frame(), pluginSize, (WebCore::Element*)(element), url, paramNames, paramValues, resolvedMimeType, loadManually, webWindow(), webHost());
        if (pdfPluginView.get())
            return pdfPluginView;
    }
#endif //OS(DARWIN)
    
    RefPtr<WebCore::PluginView> pluginView = WebCore::PluginView::create(frame(), pluginSize, (WebCore::Element *)element, url, paramNames, paramValues, mimeType, loadManually).get();

    if (pluginView->status() == WebCore::PluginStatusLoadedSuccessfully)
        return pluginView; 
    
#endif // ENABLE(PLUGINS)

    return 0;
}

void WebFrameImpl::redirectDataToPlugin(WebCore::Widget* pluginWidget)
{
    m_didRedirectDataToPlugin = true;
    if(pluginWidget && pluginWidget->isPluginView())
    {
        m_pluginView = static_cast<WebCore::PluginView*>(pluginWidget);
        WebCore::DocumentLoader *loader = frame()->loader()->activeDocumentLoader();
        const WebCore::ResourceResponse& response = loader->response();
        m_pluginView->didReceiveResponse(response);
    }
}

WTF::PassRefPtr<WebCore::Widget> WebFrameImpl::createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL, const WTF::Vector<WebCore::String>& paramNames, const WTF::Vector<WebCore::String>& paramValues)
{
    return 0;
}

WebCore::ObjectContentType WebFrameImpl::objectContentType(const WebCore::KURL& url, const WebCore::String& mimeTypeIn)
{
    WebCore::String mimeType = mimeTypeIn;
    if (mimeType.isEmpty()) {
        mimeType = WebCore::MIMETypeRegistry::getMIMETypeForExtension(url.path().substring(url.path().reverseFind('.')+1));
        if(mimeType.isEmpty())
            return WebCore::ObjectContentNone;
    }

    if (mimeType.isEmpty())
        return WebCore::ObjectContentFrame; // Go ahead and hope that we can display the content.

#if ENABLE(NETSCAPE_PLUGIN_API)
    //    If it's a PDF MIME type, we want ObjectContentPlugin... if we can show PDFs.
    if ( WebCore::PluginInfoStore::isPDFMIMEType(mimeType) )
    {
        //    Paranoia: these should never be nil.
        if (m_pWebHost && m_pWebHost->m_pVTable && m_pWebHost->m_pVTable->pdfCapability)
        {
            if (m_pWebHost->m_pVTable->pdfCapability(m_pWebHost) == 0)
                return WebCore::ObjectContentNetscapePlugin;
        }
    }
    else if (WebCore::PluginInfoStore::isFlashMIMEType(mimeType))
        return WebCore::ObjectContentNetscapePlugin;
#endif
    if (WebCore::MIMETypeRegistry::isSupportedImageMIMEType(mimeType))
        return WebCore::ObjectContentImage;

    if (WebCore::MIMETypeRegistry::isSupportedNonImageMIMEType(mimeType))
        return WebCore::ObjectContentFrame;

    return WebCore::ObjectContentNone;
}

WebCore::String WebFrameImpl::overrideMediaType() const
{
    return WebCore::String();
}

void WebFrameImpl::windowObjectCleared()
{
}

void WebFrameImpl::documentElementAvailable()
{
    // rhu - implement ?
}

void WebFrameImpl::didPerformFirstNavigation() const
{
}

void WebFrameImpl::registerForIconNotification(bool)
{
}

void WebFrameImpl::documentCreated()
{
    ASSERT(!m_dead);
    if (m_pWebHost) {
        ASSERT(m_pWebHost->m_pVTable);
        ASSERT(m_pWebHost->m_pVTable->handleOnDocumentCreated);
        ASSERT((frame()->loader()->state() == WebCore::FrameStateCommittedPage) || (frame()->loader()->state() == WebCore::FrameStateComplete));
        setGeneralPasteboard(0);
        m_pWebHost->m_pVTable->handleOnDocumentCreated(m_pWebHost);
    }
}

void* WebFrameImpl::getPlatformInterpreterContext() const
{
    ASSERT(!m_dead);
    if (m_pWebHost) {
        ASSERT(m_pWebHost->m_pVTable);
        ASSERT(m_pWebHost->m_pVTable->getPlatformInterpreterContext);
        return m_pWebHost->m_pVTable->getPlatformInterpreterContext(m_pWebHost);
    }
    return 0;
}

WebResourceHandle* WebFrameImpl::startResourceHandle(WebCore::ResourceHandle* pResourceHandle, WebCore::ResourceHandleClient* pResourceHandleClient, const WebCore::ResourceRequest& request)
{
    ASSERT(!m_dead);
    ASSERT(m_pFrame);
    
    WTF::RefPtr<WebResourceRequestImpl> const pWebResourceRequestImpl(WebResourceRequestImpl::construct(request));
    WebResourceRequest* const pWebResourceRequest = pWebResourceRequestImpl->getWebResourceRequest();
    
    WebResourceHandleClientImpl* pWebResourceHandleClientImpl = new WebResourceHandleClientImpl(pResourceHandle, pResourceHandleClient);
    WebResourceHandleClient* const pWebResourceHandleClient = pWebResourceHandleClientImpl->getWebResourceHandleClient();
    
    WebResourceHandle* const retValue = m_pWebHost->m_pVTable->loadResource(m_pWebHost, pWebResourceRequest, pWebResourceHandleClient);                                
    return retValue;
}

void WebFrameImpl::loadResourceSynchronously(const WebCore::ResourceRequest& request, WebCore::ResourceError& error, WebCore::ResourceResponse& response, WTF::Vector<char>& data)
{
    ASSERT(!m_dead);
    ASSERT(m_pFrame);
    WebResourceLoaderSynchronousClientImpl::loadResourceSynchronously(m_pWebHost, request, error, response, data);
}


void WebFrameImpl::putFrameOnLifeSupport(WTF::PassRefPtr<WebCore::Frame> frame)
{
    ASSERT(frame);
    ASSERT(frame == m_pFrame);
    ASSERT(m_pOwningWebView);
    m_pOwningWebView->putFrameOnLifeSupport(frame);
}

bool WebFrameImpl::isChildFrame(const WebCore::AtomicString& childFrameName)
{
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->tree());
    return m_pFrame->tree()->child(childFrameName) != 0;
}

void WebFrameImpl::windowObjectDead()
{
    WebCore::Frame* const theFrame = frame();
    ASSERT(theFrame);
    ASSERT(theFrame->script());
    
    WebCore::JSDOMWindow* const jsDOMWindow = theFrame->script()->globalObject(WebCore::mainThreadNormalWorld());
    if (m_childDoor)
        WebCore::ApolloScriptBridging::markObjectProxyDead(m_childDoor, jsDOMWindow);
    m_childDoor = JSC::JSValue();
    m_childDoorProxy = JSC::JSValue();
    
    // The JS interpreter is cleared when ever a new load is committed,
    // we need to go through the script bridging tables and deadify all
    // bridging object referencing that interpreter.  The most scary case that
    // this fixes is the case of a JS object being registered on a AS3 event
    // dispatcher.  Imagine a navigate of the frame containing the JS object
    // followed by the AS3 event being dispatched.  If we don't deadify the JS
    // function object, the JS function will execute in the context of the new
    // html document, which could be in a different domain.
    WebCore::ApolloScriptBridging::notifyGlobalObjectDead(jsDOMWindow);
}

static const char *appResourceProtocol = "app";

bool WebFrameImpl::hasApplicationPrivileges() const
{
    // It's annoying that some code uses the document URL and some uses the frame url.
    // Adding assert to validate as much as possible that we have chosen the correct one for
    // our security check.
	// An empty document URL get reported as "about:blank"
	ASSERT((frame()->loader()->url().isEmpty() && (WebCore::equalIgnoringFragmentIdentifier(frame()->document()->url(), WebCore::KURL(WebCore::KURL(), "about:blank")))) 
		   ||	(WebCore::equalIgnoringFragmentIdentifier(frame()->loader()->url(), WebCore::KURL(frame()->document()->url()))));
	
    // Special privileges are given when the URL has the app scheme.
    WebCore::String frameProtocol = frame()->loader()->url().protocol();
    return frameProtocol.lower() == appResourceProtocol;
}

static bool shouldLoadAsEmptyDocument(const WebCore::KURL &url)
{
    return url.protocol().lower() == "about" || url.isEmpty();
}

bool WebFrameImpl::hasApplicationRestrictions() const
{
    // Special restrictions apply if the frame is priviliged or has scripting access to a privileged parent or owner.
    // See Window::isSafeScript
    if (hasApplicationPrivileges())
        return true;

    if (shouldLoadAsEmptyDocument(frame()->loader()->url())) {
        WebCore::Frame* ancestorFrame = frame()->loader()->opener() ? frame()->loader()->opener() : frame()->tree()->parent();
        while (ancestorFrame && shouldLoadAsEmptyDocument(ancestorFrame->loader()->url()))
            ancestorFrame = ancestorFrame->tree()->parent();
        if (ancestorFrame)
            if (ancestorFrame->loader()->client()->hasApplicationPrivileges())
                return true;
    }

    return false;
}

bool WebFrameImpl::canLoadScript(const WebCore::KURL& url) const
{
    if (hasApplicationPrivileges()) {
        // If this is inline script, only process it when parsing
        // otherwise, possible injection attack.
        if (url.isEmpty()) {
            if (frame()->document()->parsing())
                return true;
            if (frame()->document()->processingDOMContentLoadedEvent())
                return true;
            if (frame()->document()->processingLoadEvent())
                return true;
            return false;
        }

        // Allow application resource URL scripts.
        WebCore::String protocol = url.protocol();
        if (protocol.lower() == appResourceProtocol)
            return true;

        // But we want to deny any other URLs, period.
        return false;
    }

    // Otherwise whatever, but the canLoad constraints are enforced elsewhere
    return true;
}

bool WebFrameImpl::canSetStringTimer() const
{
    // Same as whether you can insert inline script
    return canLoadScript(WebCore::KURL());
}

JSC::EvalLevel WebFrameImpl::currentEvalLevel() const
{
    if (hasApplicationRestrictions())
    {
        // In application code, only allow full eval while loading that code. 
        if (frame()->document()->parsing() || frame()->document()->processingDOMContentLoadedEvent() || frame()->document()->processingLoadEvent() || loadingApplicationScript())
            return JSC::EvalAny;
        return JSC::EvalConstantExpr;
    }
    return JSC::EvalAny;
}

bool WebFrameImpl::loadingApplicationScript() const
{
    WebCore::KURL url(WebCore::ParsedURLString, m_loadingScriptURL);
    if (url.protocol().lower() == appResourceProtocol)
        return true;
    return false;
}

bool WebFrameImpl::canConstructFunctions() const
{
    // Allow "new Function()" when eval not restricted.
    if (currentEvalLevel() >= JSC::EvalAny)
        return true;
    return false;
}

bool WebFrameImpl::isSynchronousRequestAllowed(const WebCore::KURL& url) const
{
    // Special rules for application content
    WebCore::KURL frameUrl = m_pFrame->document()->url();
    if (frameUrl.protocol().lower() == appResourceProtocol) {
        // Don't allow synchronous requests while full eval is allowed.
        if (currentEvalLevel() >= JSC::EvalAny) {
            // unless it's an application resource
            if (url.protocol().lower() != appResourceProtocol)
                return false;
        }
    }
    return true;
}

JSC::JSObject* WebFrameImpl::getRootPackage()
{
    WebScriptProxyVariant* const rootPackageVariant = m_pWebHost->m_pVTable->getRootPackage(m_pWebHost);
    ASSERT(rootPackageVariant);
    JSC::JSLock lock(false);
    WebCore::JSDOMWindow* jsDOMWindow = frame()->page()->mainFrame()->script()->globalObject(WebCore::mainThreadNormalWorld());
    JSC::ExecState* exec = jsDOMWindow->globalExec();	
    JSC::JSValue const rootPackageValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, rootPackageVariant);
    rootPackageVariant->m_pVTable->release(rootPackageVariant);
    ASSERT(rootPackageValue.isObject());
    return rootPackageValue.getObject();
}

JSC::JSObject* WebFrameImpl::getWKMethods()
{
    WebScriptProxyVariant* wkMethodsVariant = m_pWebHost->m_pVTable->getWKMethods(m_pWebHost);
    ASSERT(wkMethodsVariant);
    JSC::JSLock lock(false);
    WebCore::JSDOMWindow* jsDOMWindow = frame()->page()->mainFrame()->script()->globalObject(WebCore::mainThreadNormalWorld());
    JSC::ExecState* exec = jsDOMWindow->globalExec();	
    JSC::JSValue wkMethodsValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, wkMethodsVariant);
    wkMethodsVariant->m_pVTable->release(wkMethodsVariant);
    ASSERT(wkMethodsValue.isObject());
    return wkMethodsValue.getObject();
}

WebCore::ClipboardApolloHelper* WebFrameImpl::createDraggingClipboard()
{
    return createClipboard(true);
}

WebCore::ClipboardApolloHelper* WebFrameImpl::createGeneralClipboard()
{
    return createClipboard(false);
}

WebCore::Pasteboard* WebFrameImpl::getGeneralPasteboard()
{
    return m_generalPasteboard.get();
}

void WebFrameImpl::setGeneralPasteboard(WebCore::Pasteboard* pasteboard)
{
    m_generalPasteboard.set(pasteboard);
}
    
void WebFrameImpl::activateIMEForPlugin()
{
    ASSERT(m_pOwningWebView);
    m_pOwningWebView->activateIMEForPlugin();
}
    
void WebFrameImpl::deactivateIMEForPlugin()
{
    ASSERT(m_pOwningWebView);
    m_pOwningWebView->deactivateIMEForPlugin();
}

WebPopupWindow* WebFrameImpl::createWebPopupMenu(WebCore::PopupMenuClient* client, float computedFontSize)
{
    WebHost* const host = webHost();
    ASSERT(host);
    ASSERT(client);
    WebPopupWindowClientImpl* const webMenuClient = WebPopupWindowClientImpl::construct(host, client);
    return host->m_pVTable->createPopupWindow(host, webMenuClient->getWebPopupWindowClient(), computedFontSize);
}

WebCore::ClipboardApolloHelper* WebFrameImpl::createClipboard(bool forDragging)
{
    JSC::JSObject* wkMethods = getWKMethods();
    ASSERT(wkMethods);

    WebCore::JSDOMWindow* jsDOMWindow = frame()->page()->mainFrame()->script()->globalObject(WebCore::mainThreadNormalWorld());
    JSC::ExecState* exec = jsDOMWindow->globalExec();

    JSC::JSLock lock(false);

    JSC::JSObject* createClipboard = wkMethods->get(exec, JSC::Identifier(exec, "createClipboard")).toObject(exec);

    JSC::MarkedArgumentBuffer args;
    args.append(JSC::jsBoolean(true)); // writable
    args.append(JSC::jsBoolean(forDragging));
    if (!forDragging) {
        JSC::JSObject* rootPackage = getRootPackage();
        ASSERT(rootPackage);
        JSC::JSObject* flash = rootPackage->get(exec, JSC::Identifier(exec, "flash")).toObject(exec);
        JSC::JSObject* desktop = flash->get(exec, JSC::Identifier(exec, "desktop")).toObject(exec);
        JSC::JSObject* Clipboard = desktop->get(exec, JSC::Identifier(exec, "Clipboard")).toObject(exec);
        JSC::JSObject* generalClipboard = Clipboard->get(exec, JSC::Identifier(exec, "generalClipboard")).toObject(exec);
        args.append(generalClipboard);
    }

	JSC::CallData callData;
    JSC::CallType callType = createClipboard->getCallData(callData);
    ASSERT(callType != JSC::CallTypeNone);
    JSC::JSObject* jsClipboardObject = JSC::call(exec, createClipboard, callType, callData, wkMethods, args).toObject(exec);	
    return new WebCore::ClipboardApolloHelper(jsClipboardObject, exec);
}

void WebFrameImpl::updateLocation( const WebCore::KURL& newLocation )
{
    ASSERT(!m_dead);
    WTF::CString const urlAsUTF8( newLocation.prettyURL().utf8() );
    ASSERT(m_pFrame);
    ASSERT(m_pFrame->loader());
    const char* const rawURLStrAsUTF8 = urlAsUTF8.data();
    if (!m_pFrame || !m_pFrame->loader() || m_pFrame->loader()->creatingInitialEmptyDocument())
        return;
    if (m_pWebHost)
    {
        m_pWebHost->m_pVTable->updateLocation(m_pWebHost, reinterpret_cast<const unsigned char*>(rawURLStrAsUTF8));
    }
}

void WebFrameImpl::clearPluginViewRedirect()
{
    m_pluginView = 0;
    m_didRedirectDataToPlugin = false;
}

void WebFrameImpl::addPluginRootObject(WTF::RefPtr<JSC::Bindings::RootObject>& root)
{
    m_rootObjects.append(root);
}

void WebFrameImpl::cleanupPluginRootObjects()
{
    JSC::JSLock lock(false);

    size_t count = m_rootObjects.size();
    for (size_t i = 0; i < count; i++)
        m_rootObjects[i]->invalidate(); // XXX - used to be removeAllNativeReferences, perhaps instead the destructing of the object is enough

    m_rootObjects.clear();
}

namespace
{
    enum InvokedBy { InvokedBySelf, InvokedByParent, InvokedByOpener, InvokedByOther };
    static InvokedBy invokedBy(JSC::ExecState* exec, WebCore::Frame* frame)
    {
        if (frame) {
            WebCore::Frame* invokingFrame = static_cast<WebCore::JSDOMWindow*>(exec->dynamicGlobalObject())->impl()->frame();
            if (invokingFrame) {
                if (frame == invokingFrame)
                    return InvokedBySelf;
                if (frame->tree()->parent() == invokingFrame)
                    return InvokedByParent;
                if (frame->loader()->opener() == invokingFrame)
                    return InvokedByOpener;
            }
        }
        return InvokedByOther;
    }
}
JSC::JSValue WebFrameImpl::getChildDoor(JSC::ExecState* exec) const
{
    switch (invokedBy(exec, m_pFrame)) {
    case InvokedBySelf:
        return m_childDoor ? m_childDoor.get() : JSC::jsUndefined();
    case InvokedByParent:
    case InvokedByOpener:
        return m_childDoorProxy ? m_childDoorProxy.get() : JSC::jsUndefined() ;
    default:
        ;
    }
    return JSC::jsUndefined();
}

void WebFrameImpl::setChildDoor(JSC::ExecState* exec, JSC::JSValue value)
{
    switch (invokedBy(exec, m_pFrame)) {
    case InvokedBySelf:
        if (!value || value.isUndefined())
            m_childDoor = m_childDoorProxy = JSC::jsUndefined();
        else {
            m_childDoor = value;
            WebScriptProxyVariant* variant = WebCore::ApolloScriptBridging::getApolloVariantForJSValue(exec, value);
            ASSERT(variant);
            WebScriptProxyVariant* resultVariant = m_pWebHost->m_pVTable->makeDoor(m_pWebHost, variant);
            // the resultVariant can be null if we are destroying the WebViewApollo.
            if (resultVariant) {
                m_childDoorProxy = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, resultVariant);
                resultVariant->m_pVTable->release(resultVariant);
            }
            else {
                variant->m_pVTable->release(variant);
                m_childDoor = m_childDoorProxy = JSC::jsUndefined();
            }
        }
    default:
        ;
    }
}

JSC::JSValue WebFrameImpl::getParentDoor(JSC::ExecState* exec) const
{
    switch (invokedBy(exec, m_pFrame)) {
    case InvokedBySelf:
        return m_parentDoorProxy ? m_parentDoorProxy.get() : JSC::jsUndefined();
    case InvokedByParent:
    case InvokedByOpener:
        return m_parentDoor ? m_parentDoor.get() : JSC::jsUndefined();
    default:
        ;
    }
    return JSC::jsUndefined();
}

void WebFrameImpl::setParentDoor(JSC::ExecState* exec, JSC::JSValue value)
{
    switch (invokedBy(exec, m_pFrame)) {
    case InvokedByParent:
    case InvokedByOpener:
        if (!value || value.isUndefined())
            m_parentDoor = m_parentDoorProxy = JSC::jsUndefined();
        else {
            m_parentDoor = value;
            WebScriptProxyVariant* variant = WebCore::ApolloScriptBridging::getApolloVariantForJSValue(exec, value);
            ASSERT(variant);
            WebScriptProxyVariant* resultVariant = m_pWebHost->m_pVTable->makeDoor(m_pWebHost, variant);
            // the resultVariant can be null if we are destroying the WebViewApollo.
            if (resultVariant) {
                m_parentDoorProxy = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, resultVariant);
                resultVariant->m_pVTable->release(resultVariant);
            }
            else {
                variant->m_pVTable->release(variant);
                m_parentDoor = m_parentDoorProxy = JSC::jsUndefined();
            }
        }
    default:
        ;
    }
}

JSC::JSValue WebFrameImpl::getHtmlLoader(JSC::ExecState* exec) const
{
    WebCore::Frame* invokingFrame = static_cast<WebCore::JSDOMWindow*>(exec->dynamicGlobalObject())->impl()->frame();
    if (!invokingFrame)
        return JSC::jsUndefined();

    // Only app content can access the HTMLLoader
    WebCore::KURL invokingFrameUrl = invokingFrame->document()->url();
    if (invokingFrameUrl.protocol().lower() != appResourceProtocol)
        return JSC::jsUndefined();

    WebScriptProxyVariant* resultVariant = m_pWebHost->m_pVTable->getHtmlControl(m_pWebHost);
    if (!resultVariant)
        return JSC::jsUndefined();
    JSC::JSValue const htmlLoaderValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, resultVariant);
    resultVariant->m_pVTable->release(resultVariant);
    return htmlLoaderValue;
}

WebCore::FrameLoaderClient::AllowXHR WebFrameImpl::allowCrossDomainXMLHttpRequest(const WebCore::KURL& url) const
{
    ASSERT(m_pFrame);
    if (!m_pFrame)
        return AllowXHRDefault;

    // Allow application content to access anything
    WebCore::KURL frameUrl = m_pFrame->document()->url();
    if (frameUrl.protocol().lower() == appResourceProtocol)
        return AllowXHRAlways;

    // If the parent frame's an application frame
    WebCore::Frame* parent = m_pFrame->tree()->parent();
    if (parent && WebCore::KURL(parent->document()->url()).protocol().lower() == appResourceProtocol) {
        // and the frame element has the magic attribute
        if (frameHasAllowCrossDomainXHRAttr(m_pFrame)) {
            // and the url is not local
            if (!WebCore::SecurityOrigin::shouldTreatURLAsLocal(url))
                return AllowXHRAlways;
        }
    }
	
	//file is allowed to load file.	
	if(WebCore::SecurityOrigin::shouldTreatURLAsLocal(frameUrl) && WebCore::SecurityOrigin::shouldTreatURLAsLocal(url))
		return AllowXHRDefault;

    // Don't allow non-app content to access files, app, or app-storage
    // This is to some extent redundant with what the caller checks.
    if (WebCore::SecurityOrigin::shouldTreatURLAsLocal(url))
        return AllowXHRNever;
    if (url.protocol().lower() == appResourceProtocol)
        return AllowXHRNever;

    return AllowXHRDefault;
}

bool WebFrameImpl::canFrameBeClosedOrFocusedFrom(JSC::ExecState *exec) const
{
    if (!m_pFrame)
        return false;

    WebCore::Frame* invokingFrame = static_cast<WebCore::JSDOMWindow*>(exec->dynamicGlobalObject())->impl()->frame();
    if (!invokingFrame)
        return false;

    // Application content can do whatever it wants to
    WebCore::KURL invokingFrameUrl = invokingFrame->document()->url();
    if (invokingFrameUrl.protocol().lower() == appResourceProtocol)
        return true;

    // App content can't be closed by non-app content
    if (hasApplicationRestrictions())
        return false;

    // The Firefox/Safari behavior:
    //   Windows can be closed as long as they were opened via script (have an opener)
    if (m_pFrame->loader()->opener())
        return true;

    return false;
}

bool WebFrameImpl::canExecuteScriptURL() const
{
    // App content can't execute JavaScript links.
    if (!m_pFrame)
        return false;

    WebCore::Frame *frame = m_pFrame;

    WebCore::KURL frameUrl = frame->loader()->url();

    // A messy detail.  If this is an "empty" frame (perhaps loaded using document.open, document.write),
    // then check the parent or owner.
    if (hasApplicationRestrictions())
        return false;

    return true;
}

bool WebFrameImpl::isDebuggingEnabled() const
{
	ASSERT(WebKitApollo::g_HostFunctions);
	ASSERT(WebKitApollo::g_HostFunctions->isDebuggingEnabled);

	return WebKitApollo::g_HostFunctions->isDebuggingEnabled();
}

void WebFrameImpl::logWarning(WebCore::String msg) const 
{
	ASSERT(WebKitApollo::g_HostFunctions);
	ASSERT(WebKitApollo::g_HostFunctions->logWarning);

	WebKitApollo::g_HostFunctions->logWarning(msg.webString());
}
    
void WebFrameImpl::addPlugin(WebCore::Widget* const widget)
{
    m_pOwningWebView->addPluginImpl(widget);
}
    
void WebFrameImpl::removePlugin(WebCore::Widget* const widget) 
{
    m_pOwningWebView->removePluginImpl(widget);
}


}
