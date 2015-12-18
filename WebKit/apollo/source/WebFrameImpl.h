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
#ifndef WebFrameImpl_h
#define WebFrameImpl_h

#include <wtf/Platform.h>
#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif

#include <wtf/RefPtr.h>
#include <WebKitApollo/helpers/WebFrameHelper.h>
#include <ResourceHandleClient.h>
#include <FrameLoaderClientApollo.h>
#include <FrameLoader.h>
#include <Frame.h>
#include <WebViewImpl.h>

#include <wtf/Vector.h>
#include "Protect.h"
#include "runtime_root.h"

namespace JSC
{
    class JSValue;
}

namespace WebCore
{
	class ResourceRequest;
	class ResourceHandle;
	class ResourceError;
	class ResourceResponse;
	class PlatformMouseEvent;
	class PlatformWheelEvent;
	class PlatformKeyboardEvent;
    class HTMLFrameOwnerElement;
}

namespace WebKitApollo
{
    class WebViewImpl;
    namespace Private
    {
        class WebHostImplForFrames;
    }

    class WebFrameImpl : private WebCore::FrameLoaderClientApollo,
                         public WebFrameHelper< WebFrameImpl >
    {
        friend class WebFrameHelper< WebFrameImpl >;
    private:
        WebFrameImpl(WebViewImpl* const, WebHost* const, WebCore::Page* const);

        WebFrameImpl(WebViewImpl* const, WebHost* const, WebCore::HTMLFrameOwnerElement*, WebFrameImpl* const);
    protected:
        virtual ~WebFrameImpl();

    public:
        static inline WebFrameImpl* kit(const WebCore::Frame* const pFrame)
        {
            ASSERT(pFrame);
            WebCore::FrameLoader* const pFrameLoader = pFrame->loader();
            ASSERT(pFrameLoader);
            WebCore::FrameLoaderClient* const pFrameLoaderClient = pFrameLoader->client();
            ASSERT(pFrameLoaderClient);
            return static_cast<WebFrameImpl*>(pFrameLoaderClient);
        }
    
        static WebFrameImpl* construct(WebViewImpl* const, WebHost* const, WebCore::Page* const);

        void transformToRootContent(WebWindow* const pWindow);
        
        void beginModal( ) const;
        void endModal( ) const;

		int pdfCapability() const;
		int loadPDFPlugin( void **pPDFPluginObject ) const;
		int unloadPDFPlugin( void *pdfPluginObject ) const;
		void handlePDFError( int pdfErrorNum ) const;


        WebScriptProxyVariant* getEventObject() const;
		
		const uint16_t* getAppResourcePath( unsigned long *pAppResourcePathLength );
		const uint16_t* getAppStoragePath( unsigned long *pAppStoragePathLength );

		WebBitmap* createBitmap( unsigned long width, unsigned long height );

        void addPluginRootObject(WTF::RefPtr<JSC::Bindings::RootObject>& root);
        void cleanupPluginRootObjects();
        void clearPluginViewRedirect();

		void layoutRecursive();

        void sendResizeEvent( );
        void sendScrollEvent( );
        
        bool pageUp();
        bool pageDown();

        WebCore::Frame* frame() const;
        
        inline WebViewImpl* webViewImpl() const { ASSERT(m_pOwningWebView); return m_pOwningWebView; }

        virtual JSC::JSValue getChildDoor(JSC::ExecState *exec) const;
        void setChildDoor(JSC::ExecState *exec, JSC::JSValue);
        virtual JSC::JSValue getParentDoor(JSC::ExecState *exec) const;
        void setParentDoor(JSC::ExecState *exec, JSC::JSValue);
        virtual JSC::JSValue getHtmlLoader(JSC::ExecState *exec) const;

    protected:

        //Methods for ApolloWebFrame interface
        void loadHTMLString(    const char* const utf8HTMLString,
                                size_t const numHTMLStringBytes,
                                const unsigned char* const baseURL );
        void loadRequest(const WebResourceRequest* const webRequest);
		
		void evalJavaScriptString(const uint16_t* utf16Text, unsigned long numUTF16CodeUnits);
        
        void reload( );
        void stopLoading( );
        WebScriptProxyVariant* getGlobalObject();

		void historyGo( int steps );
		unsigned getHistoryLength();
		unsigned getHistoryPosition();
		void setHistoryPosition( unsigned position );
		void getHistoryAt( unsigned position, char** pUrl, char** pOriginalUrl, bool* pIsPost, uint16_t** pTitle );
        
    private:
        //FrameLoader client methods
        virtual void frameLoaderDestroyed();
        
        virtual bool hasWebView() const; // mainly for assertions
        virtual bool hasFrameView() const; // ditto

        virtual void makeRepresentation(WebCore::DocumentLoader*);
        virtual void forceLayout();
        virtual void forceLayoutForNonHTML();

        virtual void setCopiesOnScroll();

        virtual void detachedFromParent2();
        virtual void detachedFromParent3();
        virtual void detachedFromParent4();

        virtual void loadedFromPageCache();

        virtual void assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&);

        virtual void dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long identifier, WebCore::ResourceRequest&, const WebCore::ResourceResponse& redirectResponse);
        virtual bool shouldUseCredentialStorage(WebCore::DocumentLoader*, unsigned long identifier);
        virtual void dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&);
        virtual void dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&);        
        virtual void dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceResponse&);
        virtual void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long identifier, int lengthReceived);
        virtual void dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long identifier);
        virtual void dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceError&);
        virtual bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int length);
        virtual void dispatchDidLoadResourceByXMLHttpRequest(unsigned long identifier, const WebCore::ScriptString&);

        virtual void dispatchDidHandleOnloadEvents();
        virtual void dispatchDidReceiveServerRedirectForProvisionalLoad();
        virtual void dispatchDidCancelClientRedirect();
        virtual void dispatchWillPerformClientRedirect(const WebCore::KURL&, double interval, double fireDate);
        virtual void dispatchDidChangeLocationWithinPage();
        virtual void dispatchWillClose();
        virtual void dispatchDidReceiveIcon();
        virtual void dispatchDidStartProvisionalLoad();
        virtual void dispatchDidReceiveTitle(const WebCore::String& title);
        virtual void dispatchDidCommitLoad();
        virtual void dispatchDidFailProvisionalLoad(const WebCore::ResourceError&);
        virtual void dispatchDidFailLoad(const WebCore::ResourceError&);
        virtual void dispatchDidFinishDocumentLoad();
        virtual void dispatchDidFinishLoad();
        virtual void dispatchDidFirstLayout();
        virtual void dispatchDidFirstVisuallyNonEmptyLayout();

        virtual WebCore::Frame* dispatchCreatePage();
        virtual void dispatchShow();

        virtual void dispatchDecidePolicyForMIMEType(WebCore::FramePolicyFunction, const WebCore::String& MIMEType, const WebCore::ResourceRequest&);
        virtual void dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, WTF::PassRefPtr<WebCore::FormState>, const WebCore::String& frameName);
        virtual void dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, WTF::PassRefPtr<WebCore::FormState>);
        virtual void cancelPolicyCheck();

        virtual void dispatchUnableToImplementPolicy(const WebCore::ResourceError&);

        virtual void dispatchWillSubmitForm(WebCore::FramePolicyFunction, WTF::PassRefPtr<WebCore::FormState>);

        virtual void dispatchDidLoadMainResource(WebCore::DocumentLoader*);
        virtual void revertToProvisionalState(WebCore::DocumentLoader*);
        virtual void setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&);

        virtual void postProgressStartedNotification();
        virtual void postProgressEstimateChangedNotification();
        virtual void postProgressFinishedNotification();
        
        virtual void setMainFrameDocumentReady(bool);

        virtual void startDownload(const WebCore::ResourceRequest&);

        virtual void willChangeTitle(WebCore::DocumentLoader*);
        virtual void didChangeTitle(WebCore::DocumentLoader*);

        virtual void committedLoad(WebCore::DocumentLoader*, const char*, int);
        virtual void finishedLoading(WebCore::DocumentLoader*);
        
        virtual void updateGlobalHistory();
        virtual void updateGlobalHistoryRedirectLinks();
        virtual void updateGlobalHistoryForRedirectWithoutHistoryItem();
        virtual bool shouldGoToHistoryItem(WebCore::HistoryItem*) const;

        virtual WebCore::ResourceError cancelledError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError blockedError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError cannotShowURLError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError interruptForPolicyChangeError(const WebCore::ResourceRequest&);

        virtual WebCore::ResourceError cannotShowMIMETypeError(const WebCore::ResourceResponse&);
        virtual WebCore::ResourceError fileDoesNotExistError(const WebCore::ResourceResponse&);
        virtual WebCore::ResourceError pluginWillHandleLoadError(const WebCore::ResourceResponse&);

        virtual bool shouldFallBack(const WebCore::ResourceError&);

        virtual bool canHandleRequest(const WebCore::ResourceRequest&) const;
        virtual bool canShowMIMEType(const WebCore::String& MIMEType) const;
        virtual bool representationExistsForURLScheme(const WebCore::String& URLScheme) const;
        virtual WebCore::String generatedMIMETypeForURLScheme(const WebCore::String& URLScheme) const;

        virtual void frameLoadCompleted();
        virtual void saveViewStateToItem(WebCore::HistoryItem*);
        virtual void restoreViewState();
        virtual void provisionalLoadStarted();
        virtual void didFinishLoad();
        virtual void prepareForDataSourceReplacement();

        virtual WTF::PassRefPtr<WebCore::DocumentLoader> createDocumentLoader(const WebCore::ResourceRequest&, const WebCore::SubstituteData&);
        virtual void setTitle(const WebCore::String& title, const WebCore::KURL&);

        virtual WebCore::String userAgent(const WebCore::KURL&);
        
        virtual void savePlatformDataToCachedFrame(WebCore::CachedFrame*);
        virtual void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*);
        virtual void transitionToCommittedForNewPage();

        virtual bool canCachePage() const;

        virtual void download(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&);
        virtual PassRefPtr<WebCore::Frame> createFrame(const WebCore::KURL& url, const WebCore::String& name, WebCore::HTMLFrameOwnerElement* ownerElement,
                                            const WebCore::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight);

        virtual WTF::PassRefPtr<WebCore::Widget> createPlugin(const WebCore::IntSize&, WebCore::HTMLPlugInElement*, const WebCore::KURL&, const WTF::Vector<WebCore::String>&, const WTF::Vector<WebCore::String>&, const WebCore::String&, bool loadManually);
        virtual void redirectDataToPlugin(WebCore::Widget* pluginWidget);

        virtual WTF::PassRefPtr<WebCore::Widget> createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL, const WTF::Vector<WebCore::String>& paramNames, const WTF::Vector<WebCore::String>& paramValues);

        virtual WebCore::ObjectContentType objectContentType(const WebCore::KURL& url, const WebCore::String& mimeType);
        virtual WebCore::String overrideMediaType() const;

        virtual void windowObjectCleared();
        virtual void documentElementAvailable();
        
        virtual void didPerformFirstNavigation() const; // "Navigation" here means a transition from one page to another that ends up in the back/forward list.
        virtual void registerForIconNotification(bool listen = true);
        
        virtual void documentCreated();
        virtual void* getPlatformInterpreterContext() const;
        
        virtual WebResourceHandle* startResourceHandle(WebCore::ResourceHandle*, WebCore::ResourceHandleClient*, const WebCore::ResourceRequest&);
        virtual void loadResourceSynchronously(const WebCore::ResourceRequest&, WebCore::ResourceError&, WebCore::ResourceResponse&, WTF::Vector<char>& data);
        
        virtual void putFrameOnLifeSupport(WTF::PassRefPtr<WebCore::Frame>);
        
        bool isChildFrame(const WebCore::AtomicString& childFrameName);

        virtual void windowObjectDead();
        virtual bool hasApplicationPrivileges() const;
        virtual bool hasApplicationRestrictions() const;
        virtual bool canLoadScript(const WebCore::KURL& url) const;
        virtual WebCore::String loadingScriptURL() const { return m_loadingScriptURL; }
        virtual void setLoadingScriptURL(WebCore::String loadingScriptURL) { m_loadingScriptURL = loadingScriptURL; }
        bool loadingApplicationScript() const;
        virtual bool canSetStringTimer() const;
        virtual JSC::EvalLevel currentEvalLevel() const;
        virtual bool canConstructFunctions() const;
        virtual bool isSynchronousRequestAllowed(const WebCore::KURL& url) const;
        virtual AllowXHR allowCrossDomainXMLHttpRequest(const WebCore::KURL& url) const;
        virtual bool canFrameBeClosedOrFocusedFrom(JSC::ExecState *exec) const;
        virtual bool canExecuteScriptURL() const;
		virtual bool isDebuggingEnabled() const;
		virtual void logWarning(WebCore::String msg) const;
        //End of frame loader client methods.
        
    public:
        //Frame loader client apollo methods
        virtual JSC::JSObject* getRootPackage();
        virtual JSC::JSObject* getWKMethods();
        virtual WebCore::ClipboardApolloHelper* createDraggingClipboard();
        virtual WebCore::ClipboardApolloHelper* createGeneralClipboard();
        virtual WebCore::Pasteboard* getGeneralPasteboard();
        virtual void setGeneralPasteboard(WebCore::Pasteboard*);
        virtual WebPopupWindow* createWebPopupMenu(WebCore::PopupMenuClient*, float computedFontSize);
        
        virtual void addPlugin(WebCore::Widget* const);
        virtual void removePlugin(WebCore::Widget* const);
        
        virtual void activateIMEForPlugin();
        virtual void deactivateIMEForPlugin();
        //End frame loader client apollo methods.

    private:
        WebCore::ClipboardApolloHelper* createClipboard(bool forDragging);

		virtual WebHost *webHost() { return m_pOwningWebView->webHost(); }
        virtual WebWindow *webWindow() { return m_pOwningWebView->getTargetWindow(); }

		void updateLocation(const WebCore::KURL& newLocation);

        WebViewImpl* const m_pOwningWebView;
        WebHost* m_pWebHost;
        WebCore::Frame* const m_pFrame;

        std::auto_ptr< Private::WebHostImplForFrames > m_pWebHostImplForChildFrames;
        
        bool m_dead;
        bool m_canTransformToRootContent;
        WebCore::String m_loadingScriptURL;

        WTF::Vector<WTF::RefPtr<JSC::Bindings::RootObject> > m_rootObjects;

		WTF::OwnPtr<WebCore::Pasteboard> m_generalPasteboard;

        WebFrameImpl( const WebFrameImpl& other );
        WebFrameImpl& operator=( const WebFrameImpl& other );  

        JSC::ProtectedJSValue m_childDoor;
        JSC::ProtectedJSValue m_childDoorProxy;
        JSC::ProtectedJSValue m_parentDoor;
        JSC::ProtectedJSValue m_parentDoorProxy;
        
        WebCore::KURL m_mappedToURL;

        // Points to the plugin view that data should be redirected to.
        WebCore::PluginView* m_pluginView;
        bool m_didRedirectDataToPlugin;
        
        WebCore::IntSize m_lastLayoutSize;
        bool m_firstLayoutForFrameView;
    };
}

#endif
