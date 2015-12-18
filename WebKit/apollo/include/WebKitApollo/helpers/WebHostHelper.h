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
#ifndef WebHostHelper_h
#define WebHostHelper_h

#include <WebKitApollo/WebKit.h>
#include <stddef.h>

namespace WebKitApollo {
	template <class ImplClass>
	class WebHostHelper : private WebHost {
	public:
		inline WebHost* getWebHost() { return this; }
		static ImplClass* fromWebHost( WebHost* const pWebHost ) { return getThis(pWebHost); }
        
    protected:
        WebHostHelper();
		virtual ~WebHostHelper();

	private:
		static WebHostVTable const s_VTable;
		static ImplClass* getThis( WebHost* const pWebHost ) { return static_cast<ImplClass*>(pWebHost); }
		static const ImplClass* getThis( const WebHost* const pWebHost ) { return static_cast<const ImplClass*>(pWebHost); }
		
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sUpdateLocation( struct WebHost* pWebHost, unsigned const char* urlBytes );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sLoadEnd( struct WebHost* pWebHost, unsigned char success );

		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetTitle( struct WebHost* pWebHost, const uint16_t* pUTF16Title, unsigned long numTitleCodeUnits );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetStatusText( struct WebHost* pWebHost,const uint16_t* pUTF16Status, unsigned long numStatusCodeUnits );
 		static WEBKIT_APOLLO_PROTO1 WebString* WEBKIT_APOLLO_PROTO2 sGetUserAgentText( WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sUnCaughtJSException( struct WebHost* pWebHost, struct WebScriptProxyVariant* pExceptionVariant );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sRunJSAlert( struct WebHost* pWebHost, const char* pUTF8Message );
		static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sRunJSConfirm( struct WebHost* pWebHost, const char* pUTF8Message );
		static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sRunJSPrompt( struct WebHost* pWebHost, const char* pUTF8Prompt, const char* pUTF8DefaultValue, char** ppUTF8Result );
		static WEBKIT_APOLLO_PROTO1 WebResourceHandle* WEBKIT_APOLLO_PROTO2 sLoadResource( struct WebHost* pWebHost, struct WebResourceRequest* pURLRequest, struct WebResourceHandleClient* pClient );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sLoadResourceSynchronously( struct WebHost* pWebHost, struct WebResourceRequest* pURLRequest, struct WebResourceHandleClient* pClient );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sHandleOnLoadEvents( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sHandleOnDocumentCreated( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 WebViewApollo* WEBKIT_APOLLO_PROTO2 sCreateNewWindow( struct WebHost* pWebHost, struct WebHostCreateWindowArgs* windowArgs );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sCloseWindow( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sBeginModal( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sEndModal( struct WebHost* pWebHost );

		static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sPDFCapability( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sLoadPDFPlugin( struct WebHost* pWebHost, void **pPDFPluginObject );
		static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sUnloadPDFPlugin( struct WebHost* pWebHost, void *pdfPluginObject );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sHandlePDFError( struct WebHost* pWebHost, int pdfErrorNum );
		static WEBKIT_APOLLO_PROTO1 uint16_t* WEBKIT_APOLLO_PROTO2 sOpenFileChooser( struct WebHost* pWebHost, unsigned long* pFilePathLength );
		static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 sGetEventObject( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 sGetRootPackage( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 sGetWKMethods( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 sMakeDoor( struct WebHost* pWebHost, WebScriptProxyVariant* );
		static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 sGetHtmlControl( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetMouseCursor( struct WebHost* pWebHost, WebCursorType cursorType );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetMouseCapture( struct WebHost* pWebHost );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sReleaseMouseCapture( struct WebHost* pWebHost );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sCompositionSelectionChanged( struct WebHost* pWebHost, int start, int end );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sCompositionAbandoned( struct WebHost* pWebHost );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetInputMethodState( struct WebHost* pWebHost, unsigned char enable );
        static WEBKIT_APOLLO_PROTO1 bool WEBKIT_APOLLO_PROTO2 sGetInputMethodState( struct WebHost* pWebHost );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sActivateIMEForPlugin( struct WebHost* pWebHost );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDeactivateIMEForPlugin( struct WebHost* pWebHost );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSelectionChanged( struct WebHost* pWebHost );

		static WEBKIT_APOLLO_PROTO1 const uint16_t* WEBKIT_APOLLO_PROTO2 sGetAppResourcePath( WebHost* pWebHost, unsigned long* pAppResourcePathLength );
		static WEBKIT_APOLLO_PROTO1 const uint16_t* WEBKIT_APOLLO_PROTO2 sGetAppStoragePath( WebHost* pWebHost, unsigned long* pAppStoragePathLength );
		static WEBKIT_APOLLO_PROTO1 WebPopupWindow* WEBKIT_APOLLO_PROTO2 sCreatePopupWindow ( WebHost* pWebHost, struct WebPopupWindowClient* pWebPopupWindowClient, float computedFontSize );
		static WEBKIT_APOLLO_PROTO1 WebBitmap* WEBKIT_APOLLO_PROTO2 sCreateBitmap( WebHost* pWebHost, unsigned long width, unsigned long height );
		static WEBKIT_APOLLO_PROTO1 WebBitmap* WEBKIT_APOLLO_PROTO2 sCreateBitmapFromBitmapDataObject( WebHost* pWebHost, WebScriptProxyVariant* bitmapDataObject );
        static WEBKIT_APOLLO_PROTO1 void* WEBKIT_APOLLO_PROTO2 sGetPlatformInterpreterContext( struct WebHost* pWebHost );
        static WEBKIT_APOLLO_PROTO1 WebNavigationPolicyAction WEBKIT_APOLLO_PROTO2 sDispatchDecidePolicyForNavigationAction(struct WebHost* pWebHost, WebString *url, WebNavigationTypeApollo navType, struct WebResourceRequest*);
        static WEBKIT_APOLLO_PROTO1 WebNavigationPolicyAction WEBKIT_APOLLO_PROTO2 sDispatchDecidePolicyForNewWindowAction(struct WebHost* pWebHost, unsigned const char* urlBytes, unsigned long numURLBytes, WebNavigationTypeApollo navType, struct WebResourceRequest*, const uint16_t* frameNameUTF16, unsigned long numFrameNameUTF16CodeUnits);
        static WEBKIT_APOLLO_PROTO1 WebString* WEBKIT_APOLLO_PROTO2 sGetLanguage(struct WebHost* pWebHost);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sCanShowPlugins(struct WebHost* pWebHost, bool* canShowPlugins, bool* canShowWindowedPlugins);
		static WEBKIT_APOLLO_PROTO1 bool WEBKIT_APOLLO_PROTO2 sEnterPlayer(struct WebHost* webHost, WebHostEnterPlayerCallback callback, void* arg );
		static WEBKIT_APOLLO_PROTO1 bool WEBKIT_APOLLO_PROTO2 sIsPlatformAPIVersion(struct WebHost* webHost, uint32_t airVersionToCheck);
    };

	template <class ImplClass>
	WebHostVTable const WebHostHelper<ImplClass>::s_VTable = {
		sizeof(WebHostVTable),
		WebHostHelper<ImplClass>::sUpdateLocation,
		WebHostHelper<ImplClass>::sLoadEnd,
		WebHostHelper<ImplClass>::sSetTitle,
		WebHostHelper<ImplClass>::sSetStatusText,
		WebHostHelper<ImplClass>::sGetUserAgentText,
		WebHostHelper<ImplClass>::sUnCaughtJSException,
		WebHostHelper<ImplClass>::sRunJSAlert,
		WebHostHelper<ImplClass>::sRunJSConfirm,
		WebHostHelper<ImplClass>::sRunJSPrompt,
		WebHostHelper<ImplClass>::sLoadResource,
		WebHostHelper<ImplClass>::sLoadResourceSynchronously,
		WebHostHelper<ImplClass>::sHandleOnLoadEvents,
		WebHostHelper<ImplClass>::sHandleOnDocumentCreated,
		WebHostHelper<ImplClass>::sCreateNewWindow,
		WebHostHelper<ImplClass>::sCloseWindow,
		WebHostHelper<ImplClass>::sBeginModal,
		WebHostHelper<ImplClass>::sEndModal,
		WebHostHelper<ImplClass>::sPDFCapability,
		WebHostHelper<ImplClass>::sLoadPDFPlugin,
		WebHostHelper<ImplClass>::sUnloadPDFPlugin,
		WebHostHelper<ImplClass>::sHandlePDFError,
        WebHostHelper<ImplClass>::sOpenFileChooser,
		WebHostHelper<ImplClass>::sGetEventObject,
        WebHostHelper<ImplClass>::sGetRootPackage,
        WebHostHelper<ImplClass>::sGetWKMethods,
        WebHostHelper<ImplClass>::sMakeDoor,
        WebHostHelper<ImplClass>::sGetHtmlControl,
        WebHostHelper<ImplClass>::sSetMouseCursor,
        WebHostHelper<ImplClass>::sSetMouseCapture,
        WebHostHelper<ImplClass>::sReleaseMouseCapture,
        WebHostHelper<ImplClass>::sCompositionSelectionChanged,
        WebHostHelper<ImplClass>::sCompositionAbandoned,
        WebHostHelper<ImplClass>::sSetInputMethodState,
		WebHostHelper<ImplClass>::sGetInputMethodState,
        WebHostHelper<ImplClass>::sActivateIMEForPlugin,
        WebHostHelper<ImplClass>::sDeactivateIMEForPlugin,
        WebHostHelper<ImplClass>::sSelectionChanged,
        WebHostHelper<ImplClass>::sGetAppResourcePath,
        WebHostHelper<ImplClass>::sGetAppStoragePath,
		WebHostHelper<ImplClass>::sCreatePopupWindow,
		WebHostHelper<ImplClass>::sCreateBitmap,
		WebHostHelper<ImplClass>::sCreateBitmapFromBitmapDataObject,
        WebHostHelper<ImplClass>::sGetPlatformInterpreterContext,
        WebHostHelper<ImplClass>::sDispatchDecidePolicyForNavigationAction,
        WebHostHelper<ImplClass>::sDispatchDecidePolicyForNewWindowAction,
        WebHostHelper<ImplClass>::sGetLanguage,
        WebHostHelper<ImplClass>::sCanShowPlugins,
		WebHostHelper<ImplClass>::sEnterPlayer,
		WebHostHelper<ImplClass>::sIsPlatformAPIVersion
	};

    template<class ImplClass>
    WebHostHelper<ImplClass>::WebHostHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
	WebHostHelper<ImplClass>::~WebHostHelper()
	{
        m_pVTable = 0;
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sUpdateLocation( struct WebHost* pWebHost, const unsigned char* urlBytes )
	{
		ImplClass* pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->updateLocation(urlBytes);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sLoadEnd( struct WebHost* pWebHost, unsigned char success )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
        bool const bSuccess = success != 0;
		pThis->loadEnd(bSuccess);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sSetTitle( struct WebHost* pWebHost, const uint16_t* pUTF16Title, unsigned long numTitleCodeUnits )
	{
		ImplClass* pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->setTitle(pUTF16Title, numTitleCodeUnits);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sSetStatusText(	WebHost* pWebHost, const uint16_t* pUTF16Status, unsigned long numStatusCodeUnits )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->setStatusText(pUTF16Status, numStatusCodeUnits);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
 	WebString* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetUserAgentText( WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return 0;
 		return pThis->getUserAgentText();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sUnCaughtJSException( struct WebHost* pWebHost, struct WebScriptProxyVariant* pExceptionVariant )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->uncaughtJSException(pExceptionVariant);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sRunJSAlert( struct WebHost* pWebHost, const char* pUTF8Message )	
	{
		ImplClass* pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->runJSAlert( pUTF8Message );
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	unsigned char WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sRunJSConfirm(	struct WebHost* pWebHost, const char* pUTF8Message )	
	{	
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return 0;
		unsigned char const result = pThis->runJSConfirm( pUTF8Message ) ? 1 : 0;
		return result;
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	unsigned char WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sRunJSPrompt( struct WebHost* pWebHost, const char* pUTF8Prompt, const char* pUTF8DefaultValue, char** ppUTF8Result )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return 0;
		unsigned char const result = pThis->runJSPrompt( pUTF8Prompt, pUTF8DefaultValue, ppUTF8Result ) ? 1 : 0;
		return result;
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebResourceHandle* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sLoadResource( struct WebHost* pWebHost, struct WebResourceRequest* pURLRequest, struct WebResourceHandleClient* pClient )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return 0;
		WebResourceHandle* const result = pThis->loadResource(pURLRequest, pClient);
		return result;
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sLoadResourceSynchronously(	struct WebHost* pWebHost, struct WebResourceRequest* pURLRequest, struct WebResourceHandleClient* pClient )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->loadResourceSynchronously(pURLRequest, pClient);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sHandleOnLoadEvents( struct WebHost* pWebHost )
	{
		ImplClass* pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->handleOnLoadEvents();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sHandleOnDocumentCreated( struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->handleOnDocumentCreated();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebViewApollo* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sCreateNewWindow( struct WebHost* pWebHost, struct WebHostCreateWindowArgs* windowArgs )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return 0;
		return pThis->createNewWindow(windowArgs);
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sCloseWindow( struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->closeWindow();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sBeginModal( struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->beginModal();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sEndModal(	struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return;
		pThis->endModal();
	}
	template < class ImplClass >
	WEBKIT_APOLLO_PROTO1
	int WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sPDFCapability( struct WebHost* pWebHost )
	{
		ImplClass* pThis = getThis( pWebHost );
		if ( pThis->isUnusable() ) { return 2173; }
		return pThis->pdfCapability( );
	}
	
	template < class ImplClass >
	WEBKIT_APOLLO_PROTO1
		int WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sLoadPDFPlugin( struct WebHost* pWebHost, void **pPDFPluginObject )
	{
		ImplClass* pThis = getThis( pWebHost );
		if ( pThis->isUnusable() )
		{
			if ( pPDFPluginObject != NULL )
				*pPDFPluginObject = NULL;
			return 2173;
		}
		return pThis->loadPDFPlugin( pPDFPluginObject );
	}
	
	template < class ImplClass >
	WEBKIT_APOLLO_PROTO1
		int WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sUnloadPDFPlugin(	struct WebHost* pWebHost, void *pdfPluginObject )
	{
		ImplClass* pThis = getThis( pWebHost );
		if ( pThis->isUnusable() ) { return 2173; }
		return pThis->unloadPDFPlugin( pdfPluginObject );
	}

	template < class ImplClass >
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sHandlePDFError(	struct WebHost* pWebHost, int pdfErrorNum )
	{
		ImplClass* pThis = getThis( pWebHost );
		if ( pThis->isUnusable() ) { return; }
		pThis->handlePDFError( pdfErrorNum );
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
    uint16_t* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sOpenFileChooser( struct WebHost* pWebHost, unsigned long* pFilePathLength )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
            return 0;
		return pThis->openFileChooser(pFilePathLength);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetEventObject( struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getEventObject();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetRootPackage( struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getRootPackage();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetWKMethods( struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getWKMethods();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sMakeDoor( struct WebHost* pWebHost, struct WebScriptProxyVariant* pProxy )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->makeDoor(pProxy);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetHtmlControl( struct WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getHtmlControl();
	}

	template < class ImplClass >
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sSetMouseCursor( struct WebHost* pWebHost, WebCursorType cursorType )
	{
		ImplClass* pThis = getThis( pWebHost );
		if ( pThis->isUnusable() ) { return; }
		pThis->setMouseCursor( cursorType );
	}

	template < class ImplClass >
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sSetMouseCapture( struct WebHost* pWebHost )
	{
		ImplClass* pThis = getThis( pWebHost );
		if ( pThis->isUnusable() ) { return; }
		pThis->setMouseCapture( );
	}

	template < class ImplClass >
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sReleaseMouseCapture( struct WebHost* pWebHost )
	{
		ImplClass* pThis = getThis( pWebHost );
		if ( pThis->isUnusable() ) { return; }
		pThis->releaseMouseCapture( );
	}

    template < class ImplClass >
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sCompositionSelectionChanged( struct WebHost* pWebHost, int start, int end )
    {
        ImplClass* pThis = getThis(pWebHost);
        if (pThis->isUnusable()) return;
        pThis->compositionSelectionChanged(start, end);
    }

    template < class ImplClass >
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sCompositionAbandoned( struct WebHost* pWebHost )
    {
        ImplClass* pThis = getThis(pWebHost);
        if (pThis->isUnusable()) return;
        pThis->compositionAbandoned();
    }

    template < class ImplClass >
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sSetInputMethodState( struct WebHost* pWebHost, unsigned char enable )
    {
        ImplClass* pThis = getThis(pWebHost);
        if (pThis->isUnusable()) return;
        pThis->setInputMethodState(enable != 0);
    }

    template < class ImplClass >
    WEBKIT_APOLLO_PROTO1
    bool WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sGetInputMethodState( struct WebHost* pWebHost )
    {
        ImplClass* pThis = getThis(pWebHost);
        if (pThis->isUnusable()) 
			return NULL;
        return pThis->getInputMethodState();
    }

    template < class ImplClass >
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sActivateIMEForPlugin( struct WebHost* pWebHost )
    {
        ImplClass* pThis = getThis(pWebHost);
        if (pThis->isUnusable()) return;
        pThis->activateIMEForPlugin();
    }

    template < class ImplClass >
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sDeactivateIMEForPlugin( struct WebHost* pWebHost )
    {
        ImplClass* pThis = getThis(pWebHost);
        if (pThis->isUnusable()) return;
        pThis->deactivateIMEForPlugin();
    }

    template < class ImplClass >
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebHostHelper< ImplClass >::sSelectionChanged( struct WebHost* pWebHost )
    {
        ImplClass* pThis = getThis(pWebHost);
        if (pThis->isUnusable()) return;
        pThis->selectionChanged();
    }

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const uint16_t* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetAppResourcePath( WebHost* pWebHost, unsigned long* pAppResourcePathLength )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getAppResourcePath( pAppResourcePathLength );
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const uint16_t* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetAppStoragePath( WebHost* pWebHost, unsigned long* pAppStoragePathLength )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getAppStoragePath( pAppStoragePathLength );
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebPopupWindow* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sCreatePopupWindow( WebHost* pWebHost, struct WebPopupWindowClient* pWebPopupWindowClient, float computedFontSize )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->createPopupWindow( pWebPopupWindowClient, computedFontSize );
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebBitmap* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sCreateBitmap( WebHost* pWebHost, unsigned long width, unsigned long height )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return static_cast<WebBitmap*>(pThis->createBitmap(width, height));
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebBitmap* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sCreateBitmapFromBitmapDataObject( WebHost* pWebHost, WebScriptProxyVariant* bitmapDataObject )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return static_cast<WebBitmap*>(pThis->createBitmapFromBitmapDataObject(bitmapDataObject));
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetPlatformInterpreterContext(WebHost* pWebHost )
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return 0;
		return pThis->getPlatformInterpreterContext();
	}
    
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebNavigationPolicyAction WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sDispatchDecidePolicyForNavigationAction(WebHost* pWebHost, WebString* url, WebNavigationTypeApollo navType, WebResourceRequest* resourceRequest)
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return WebNavigationPolicyActionIgnore;
		return pThis->dispatchDecidePolicyForNavigationAction(url, navType, resourceRequest);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebNavigationPolicyAction WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sDispatchDecidePolicyForNewWindowAction(WebHost* pWebHost, unsigned const char* urlBytes, unsigned long numURLBytes, WebNavigationTypeApollo navType, WebResourceRequest* resourceRequest, const uint16_t* frameNameUTF16, unsigned long numFrameNameUTF16CodeUnits)
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return WebNavigationPolicyActionIgnore;
		return pThis->dispatchDecidePolicyForNewWindowAction(urlBytes, numURLBytes, navType, resourceRequest, frameNameUTF16, numFrameNameUTF16CodeUnits);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebString* WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sGetLanguage(WebHost* pWebHost)
	{
		ImplClass* const pThis = getThis(pWebHost);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getLanguage();
	}

    

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sCanShowPlugins(WebHost* pWebHost, bool* canShowPlugins, bool* canShowWindowedPlugins)
	{
		ImplClass* const pThis = getThis(pWebHost);
        if (pThis->isUnusable()) {
            *canShowPlugins = false;
            *canShowWindowedPlugins = false;
			return;
        }
		return pThis->canShowPlugins(canShowPlugins, canShowWindowedPlugins);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	bool WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sEnterPlayer(struct WebHost* pWebHost, WebHostEnterPlayerCallback callback, void* arg )
	{
		ImplClass* const pThis = getThis(pWebHost);
        if (pThis->isUnusable()) 
        	return false;
        
		return pThis->enterPlayer(callback,arg);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	bool WEBKIT_APOLLO_PROTO2 WebHostHelper<ImplClass>::sIsPlatformAPIVersion(struct WebHost* pWebHost, uint32_t airVersionToCheck )
	{
		ImplClass* const pThis = getThis(pWebHost);
        if (pThis->isUnusable()) 
        	return false;
        
		return pThis->isPlatformAPIVersion(airVersionToCheck);
	}
}

#endif
