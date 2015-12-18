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
#ifndef WebViewHelper_h
#define WebViewHelper_h

#include <WebKitApollo/WebKit.h>

#include <wtf/Assertions.h>

namespace WebKitApollo
{
    template <class ImplClass>
    class WebViewHelper : private WebViewApollo {
 
    public:
    
        inline WebViewApollo* getWebView() { return this; }

        static inline ImplClass* getImpl( WebViewApollo* const pWebView )
        {
            ASSERT( pWebView );
            ASSERT( pWebView->m_pVTable == &s_VTable );
            return getThis( pWebView );
        }
    protected:
        WebViewHelper();
        virtual ~WebViewHelper();

    private:
        static WebViewVTable const s_VTable;
        static ImplClass* getThis( WebViewApollo* const pWebView ) { return static_cast<ImplClass*>( pWebView ); }
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDestroy( struct WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 WebFrameApollo* WEBKIT_APOLLO_PROTO2 sMainFrame( struct WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sLayout( struct WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sPaint( struct WebViewApollo* pWebView, const struct WebIntRect* pPaintRect );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnContextMenuItemSelect( WebViewApollo* pWebView , int const actionID, const uint16_t *title);
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnContextMenu( WebViewApollo* pWebView
                                                                          , int const localX
                                                                          , int const localY
                                                                          , int const screenX
                                                                          , int const screenY
                                                                          , unsigned char const shiftKey
                                                                          , unsigned char const ctrlKey
                                                                          , unsigned char const altKey
                                                                          , unsigned char const metaKey );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnMouseDown( WebViewApollo* pWebView
                                                                          , int const localX
                                                                          , int const localY
                                                                          , int const screenX
                                                                          , int const screenY
                                                                          , WebMouseButton const mouseButton
                                                                          , unsigned int const clickCount
                                                                          , unsigned char const shiftKey
                                                                          , unsigned char const ctrlKey
                                                                          , unsigned char const altKey
                                                                           , unsigned char const metaKey );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnMouseUp( WebViewApollo* pWebView
                                                                        , int const localX
                                                                        , int const localY
                                                                        , int const screenX
                                                                        , int const screenY
                                                                        , WebMouseButton const mouseButton
                                                                        , unsigned int const clickCount
                                                                        , unsigned char const shiftKey
                                                                        , unsigned char const ctrlKey
                                                                        , unsigned char const altKey
                                                                        , unsigned char const metaKey );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnMouseMove( WebViewApollo* pWebView
                                                                          , int const localX
                                                                          , int const localY
                                                                          , int const screenX
                                                                          , int const screenY
                                                                          , unsigned char const shiftKey
                                                                          , unsigned char const ctrlKey
                                                                          , unsigned char const altKey
                                                                          , unsigned char const metaKey
                                                                          , unsigned char const buttonDown);

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnMouseWheel( WebViewApollo* pWebView
                                                                           , int const localX
                                                                           , int const localY
                                                                           , int const screenX
                                                                           , int const screenY
                                                                           , int const deltaX
                                                                           , int const deltaY
                                                                           , unsigned char const shiftKey
                                                                           , unsigned char const ctrlKey
                                                                           , unsigned char const altKey
                                                                           , unsigned char const metaKey );

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnFocusIn( WebViewApollo* pWebView, const uint16_t *direction, int length );
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnFocusOut( WebViewApollo* pWebView );

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnWindowActivate( WebViewApollo* pWebView );
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnWindowDeactivate( WebViewApollo* pWebView );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnKeyDown( WebViewApollo* pWebView
                                                                                , unsigned int const winCharCode
                                                                                , unsigned int const winKeyCode
                                                                                , unsigned char const isShiftKey
                                                                                , unsigned char const isCtrlKey
                                                                                , unsigned char const isAltKey
                                                                                , unsigned char const isMetaKey
                                                                                , unsigned char const isAutoRepeat );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnKeyUp( WebViewApollo* pWebView
                                                                                , unsigned int const winCharCode
                                                                                , unsigned int const winKeyCode
                                                                                , unsigned char const isShiftKey
                                                                                , unsigned char const isCtrlKey
                                                                                , unsigned char const isAltKey
                                                                                , unsigned char const isMetaKey );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnKeyPress( WebViewApollo* pWebView
                                                                                    , unsigned int const winCharCode
                                                                                    , unsigned char const isShiftKey
                                                                                    , unsigned char const isCtrlKey
                                                                                    , unsigned char const isAltKey
                                                                                    , unsigned char const metaKey
                                                                                    , unsigned char const isAutoRepeat );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sInsertText(WebViewApollo* webView
                                                                             , const uint16_t* utf16Text
                                                                             , unsigned long numUTF16CodeUnits);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetComposition( WebViewApollo* webView
                                                                             , const uint16_t* utf16Text
                                                                             , unsigned long numUTF16CodeUnits
                                                                             , WebViewCompositionUnderlineRange *underlineRanges
                                                                             , unsigned long numUnderlineRanges
                                                                             , int startSelection
                                                                             , int endSelection );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sConfirmComposition( WebViewApollo* webView
                                                                                    , const uint16_t* utf16Text
                                                                                    , unsigned long numUTF16CodeUnits );

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sConfirmCurrentComposition( WebViewApollo* webView , unsigned char preserveSelection );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sGetSelectedRange( WebViewApollo* webView
                                                                                        , int *startSelection
                                                                                        , int *endSelection
                                                                                        , unsigned char *isEditable );

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetSelectedRange( WebViewApollo* webView
                                                                                        , int startSelection
                                                                                        , int endSelection );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sGetCompositionRange( WebViewApollo* webView
                                                                                           , int *startComposition
                                                                                           , int *endComposition );

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetFirstRectForCharacterRange( WebViewApollo* webView
                                                                                           , int startRange
                                                                                           , int endRange
                                                                                           , WebIntRect* rect );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sGetTextForCharacterRange( WebViewApollo* webView
                                                                                           , int startRange
                                                                                           , int endRange
                                                                                           , uint16_t* text );


        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnDragEnter( WebViewApollo* pWebView
                                                                          , int const localX
                                                                          , int const localY
                                                                          , int const screenX
                                                                          , int const screenY
                                                                          , unsigned char const shiftKey
                                                                          , unsigned char const ctrlKey
                                                                          , unsigned char const altKey
                                                                          , unsigned char const metaKey
                                                                          , WebScriptProxyVariant* dataTransfer );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnDragOver( WebViewApollo* pWebView
                                                                                  , int const localX
                                                                                  , int const localY
                                                                                  , int const screenX
                                                                                  , int const screenY
                                                                                  , unsigned char const shiftKey
                                                                                  , unsigned char const ctrlKey
                                                                                  , unsigned char const altKey
                                                                                  , unsigned char const metaKey
                                                                                  , WebScriptProxyVariant* dataTransfer );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnDragDrop( WebViewApollo* pWebView
                                                                                  , int const localX
                                                                                  , int const localY
                                                                                  , int const screenX
                                                                                  , int const screenY
                                                                                  , unsigned char const shiftKey
                                                                                  , unsigned char const ctrlKey
                                                                                  , unsigned char const altKey
                                                                                  , unsigned char const metaKey
                                                                                  , WebScriptProxyVariant* dataTransfer );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnDragExit( WebViewApollo* pWebView
                                                                         , int const localX
                                                                         , int const localY
                                                                         , int const screenX
                                                                         , int const screenY
                                                                         , unsigned char const shiftKey
                                                                         , unsigned char const ctrlKey
                                                                         , unsigned char const altKey
                                                                         , unsigned char const metaKey
                                                                         , WebScriptProxyVariant* dataTransfer );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnDragStart( WebViewApollo* pWebView
                                                                                   , int const localX
                                                                                   , int const localY
                                                                                   , int const screenX
                                                                                   , int const screenY
                                                                                   , unsigned char const shiftKey
                                                                                   , unsigned char const ctrlKey
                                                                                   , unsigned char const altKey
                                                                                   , unsigned char const metaKey );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnDragUpdate( WebViewApollo* pWebView
                                                                                   , int const localX
                                                                                   , int const localY
                                                                                   , int const screenX
                                                                                   , int const screenY
                                                                                   , unsigned char const shiftKey
                                                                                   , unsigned char const ctrlKey
                                                                                   , unsigned char const altKey
                                                                                   , unsigned char const metaKey );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnDragComplete( WebViewApollo* pWebView
                                                                                      , int const localX
                                                                                      , int const localY
                                                                                      , int const screenX
                                                                                      , int const screenY
                                                                                      , unsigned char const shiftKey
                                                                                      , unsigned char const ctrlKey
                                                                                      , unsigned char const altKey
                                                                                      , unsigned char const metaKey
                                                                                      , const uint16_t *dropAction );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnCut( WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnCopy( WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnPaste( WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sOnSelectAll( WebViewApollo* pWebView );

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSendResizeEvent( WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSendScrollEvent( WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sConvertToRootContent( WebViewApollo* pWebView );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetPaintsDefaultBackground( WebViewApollo* pWebView, unsigned char newValue );
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetContextMenuItems( WebViewApollo* pWebView, WebMenuItem*** items, int *nItems );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetContextMenuItemInfo( WebViewApollo* pWebView, WebMenuItem* item, short **title, unsigned int *titleCharLen, int *type, unsigned char *checked, unsigned char *enabled, int *action, WebMenuItem ***subMenuItems, int *numSubMenuItems );

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sHasFocusableContent( WebViewApollo* pWebView );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetPageGroupName(struct WebViewApollo* pWebFrame, const uint16_t* groupName );
		static WEBKIT_APOLLO_PROTO1 WebString* WEBKIT_APOLLO_PROTO2 sGetPageGroupName(struct WebViewApollo* pWebFrame );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetTextEncodingOverride( WebViewApollo* webView
																					  , const uint16_t* utf16TextEncoding
																					  , unsigned long numUTF16CodeUnits);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetTextEncodingOverride( WebViewApollo* webView
																					  , uint16_t** utf16TextEncoding);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetTextEncodingFallback( WebViewApollo* webView
																					  , const uint16_t* utf16TextEncoding
																					  , unsigned long numUTF16CodeUnits);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetTextEncodingFallback( WebViewApollo* webView
																					  , uint16_t** utf16TextEncoding);
    };

    template <class ImplClass>
    WebViewVTable const WebViewHelper<ImplClass>::s_VTable =
    {
        sizeof( WebViewVTable ),
        WebViewHelper<ImplClass>::sDestroy,
        WebViewHelper<ImplClass>::sMainFrame,
        WebViewHelper<ImplClass>::sLayout,
        WebViewHelper<ImplClass>::sPaint,
        WebViewHelper<ImplClass>::sOnContextMenuItemSelect,
        WebViewHelper<ImplClass>::sOnContextMenu,
        WebViewHelper<ImplClass>::sOnMouseDown,
        WebViewHelper<ImplClass>::sOnMouseUp,
        WebViewHelper<ImplClass>::sOnMouseMove,
        WebViewHelper<ImplClass>::sOnMouseWheel,
        WebViewHelper<ImplClass>::sOnFocusIn,
        WebViewHelper<ImplClass>::sOnFocusOut,
        WebViewHelper<ImplClass>::sOnWindowActivate,
        WebViewHelper<ImplClass>::sOnWindowDeactivate,
        WebViewHelper<ImplClass>::sOnKeyDown,
        WebViewHelper<ImplClass>::sOnKeyUp,
        WebViewHelper<ImplClass>::sOnKeyPress,
        WebViewHelper<ImplClass>::sInsertText,
        WebViewHelper<ImplClass>::sSetComposition,
        WebViewHelper<ImplClass>::sConfirmComposition,
        WebViewHelper<ImplClass>::sConfirmCurrentComposition,
        WebViewHelper<ImplClass>::sGetSelectedRange,
        WebViewHelper<ImplClass>::sSetSelectedRange,
        WebViewHelper<ImplClass>::sGetCompositionRange,
        WebViewHelper<ImplClass>::sGetFirstRectForCharacterRange,
        WebViewHelper<ImplClass>::sGetTextForCharacterRange,
        WebViewHelper<ImplClass>::sOnDragEnter,
        WebViewHelper<ImplClass>::sOnDragOver,
        WebViewHelper<ImplClass>::sOnDragDrop,
        WebViewHelper<ImplClass>::sOnDragExit,
        WebViewHelper<ImplClass>::sOnDragStart,
        WebViewHelper<ImplClass>::sOnDragUpdate,
        WebViewHelper<ImplClass>::sOnDragComplete,
        WebViewHelper<ImplClass>::sOnCut,
        WebViewHelper<ImplClass>::sOnCopy,
        WebViewHelper<ImplClass>::sOnPaste,
        WebViewHelper<ImplClass>::sOnSelectAll,
        WebViewHelper<ImplClass>::sSendResizeEvent,
        WebViewHelper<ImplClass>::sSendScrollEvent,
        WebViewHelper<ImplClass>::sConvertToRootContent,
        WebViewHelper<ImplClass>::sSetPaintsDefaultBackground,
        WebViewHelper<ImplClass>::sGetContextMenuItems,
        WebViewHelper<ImplClass>::sGetContextMenuItemInfo,
        WebViewHelper<ImplClass>::sHasFocusableContent,
        WebViewHelper<ImplClass>::sSetPageGroupName,
        WebViewHelper<ImplClass>::sGetPageGroupName,
        WebViewHelper<ImplClass>::sSetTextEncodingOverride,
        WebViewHelper<ImplClass>::sGetTextEncodingOverride,
        WebViewHelper<ImplClass>::sSetTextEncodingFallback,
        WebViewHelper<ImplClass>::sGetTextEncodingFallback
    };
    
    template <class ImplClass>
    WebViewHelper<ImplClass>::WebViewHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebViewHelper<ImplClass>::~WebViewHelper()
    {
        m_pVTable = 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sDestroy(struct WebViewApollo* pWebView)
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->destroy();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    WebFrameApollo* WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sMainFrame(struct WebViewApollo* pWebView)
    {
        ImplClass* const pThis = getThis(pWebView);
        return pThis->mainWebFrame();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sLayout(struct WebViewApollo* pWebView)
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->layout();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sPaint(struct WebViewApollo* pWebView, const struct WebIntRect* pPaintRect)
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->paint(pPaintRect);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnContextMenuItemSelect( WebViewApollo* pWebView, int const actionID, const uint16_t *title )
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->onContextMenuItemSelect(actionID, title);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnContextMenu( WebViewApollo* pWebView
                                                                    , int const localX
                                                                    , int const localY
                                                                    , int const screenX
                                                                    , int const screenY
                                                                    , unsigned char const shiftKey
                                                                    , unsigned char const ctrlKey
                                                                    , unsigned char const altKey
                                                                    , unsigned char const metaKey)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bHandled = pThis->onContextMenu(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnMouseDown( WebViewApollo* pWebView
                                                                    , int const localX
                                                                    , int const localY
                                                                    , int const screenX
                                                                    , int const screenY
                                                                    , WebMouseButton const mouseButton
                                                                    , unsigned int const clickCount
                                                                    , unsigned char const shiftKey
                                                                    , unsigned char const ctrlKey
                                                                    , unsigned char const altKey
                                                                    , unsigned char const metaKey)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bHandled = pThis->onMouseDown(localX, localY, screenX, screenY, mouseButton, clickCount, bShiftKey, bCtrlKey, bAltKey, bMetaKey);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnMouseUp( WebViewApollo* pWebView
                                                                  , int const localX
                                                                  , int const localY
                                                                  , int const screenX
                                                                  , int const screenY
                                                                  , WebMouseButton const mouseButton
                                                                  , unsigned int const clickCount
                                                                  , unsigned char const shiftKey
                                                                  , unsigned char const ctrlKey
                                                                  , unsigned char const altKey
                                                                  , unsigned char const metaKey)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bHandled = pThis->onMouseUp(localX, localY, screenX, screenY, mouseButton, clickCount, bShiftKey, bCtrlKey, bAltKey, bMetaKey);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnMouseMove( WebViewApollo* pWebView
                                                                    , int const localX
                                                                    , int const localY
                                                                    , int const screenX
                                                                    , int const screenY
                                                                    , unsigned char const shiftKey
                                                                    , unsigned char const ctrlKey
                                                                    , unsigned char const altKey
                                                                    , unsigned char const metaKey
                                                                    , unsigned char const buttonDown)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bButtonDown = buttonDown != 0;
        bool const bHandled = pThis->onMouseMove(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey, bButtonDown);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnMouseWheel( WebViewApollo* pWebView
                                                                     , int const localX
                                                                     , int const localY
                                                                     , int const screenX
                                                                     , int const screenY
                                                                     , int const deltaX
                                                                     , int const deltaY
                                                                     , unsigned char const shiftKey
                                                                     , unsigned char const ctrlKey
                                                                     , unsigned char const altKey
                                                                     , unsigned char const metaKey)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bHandled =  pThis->onMouseWheel(localX, localY, screenX, screenY, deltaX, deltaY, bShiftKey, bCtrlKey, bAltKey, bMetaKey);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnFocusIn(WebViewApollo* pWebView, const uint16_t *direction, int length)
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->onFocusIn(direction, length);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnFocusOut(WebViewApollo* pWebView)
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->onFocusOut();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnWindowActivate(WebViewApollo* pWebView)
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->onWindowActivate();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnWindowDeactivate(WebViewApollo* pWebView)
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->onWindowDeactivate();
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnKeyDown( WebViewApollo* pWebView
                                                                           , unsigned int const winCharCode
                                                                           , unsigned int const winKeyCode
                                                                           , unsigned char const isShiftKey
                                                                           , unsigned char const isCtrlKey
                                                                           , unsigned char const isAltKey
                                                                           , unsigned char const isMetaKey
                                                                           , unsigned char const isAutoRepeat )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = isShiftKey != 0;
        bool const bCtrlKey = isCtrlKey != 0;
        bool const bAltKey = isAltKey != 0;
        bool const bMetaKey = isMetaKey != 0;
        bool const bIsAutoRepeat = isAutoRepeat != 0;
        bool const bHandled = pThis->onKeyDown(winCharCode, winKeyCode, bShiftKey, bCtrlKey, bAltKey, bMetaKey, bIsAutoRepeat);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnKeyUp( WebViewApollo* pWebView
                                                                         , unsigned int const winCharCode
                                                                         , unsigned int const winKeyCode
                                                                         , unsigned char const isShiftKey
                                                                         , unsigned char const isCtrlKey
                                                                         , unsigned char const isAltKey
                                                                         , unsigned char const isMetaKey )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = isShiftKey != 0;
        bool const bCtrlKey = isCtrlKey != 0;
        bool const bAltKey = isAltKey != 0;
        bool const bMetaKey = isMetaKey != 0;
        bool const bHandled = pThis->onKeyUp(winCharCode, winKeyCode, bShiftKey, bCtrlKey, bAltKey, bMetaKey);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnKeyPress( WebViewApollo* webView
                                                                             , unsigned int const winCharCode
                                                                             , unsigned char const isShiftKey
                                                                             , unsigned char const isCtrlKey
                                                                             , unsigned char const isAltKey
                                                                             , unsigned char const isMetaKey
                                                                             , unsigned char const isAutoRepeat )
    {
        ImplClass* const pThis = getThis(webView);
        bool const bShiftKey = isShiftKey != 0;
        bool const bCtrlKey = isCtrlKey != 0;
        bool const bAltKey = isAltKey != 0;
        bool const bMetaKey = isMetaKey != 0;
        bool const bIsAutoRepeat = isAutoRepeat != 0;
        bool bHandled = pThis->onKeyPress(winCharCode, bShiftKey, bCtrlKey, bAltKey, bMetaKey, bIsAutoRepeat);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sInsertText(WebViewApollo* webView
        , const uint16_t* utf16Text
        , unsigned long numUTF16CodeUnits)
    {
        ImplClass* const pThis = getThis(webView);
        bool const handled = pThis->insertText(utf16Text, numUTF16CodeUnits);
        return handled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSetComposition( WebViewApollo* webView
        , const uint16_t* utf16Text
        , unsigned long numUTF16CodeUnits
        , WebViewCompositionUnderlineRange *underlineRanges
        , unsigned long numUnderlineRanges
        , int startSelection
        , int endSelection )
    {
        ImplClass* const pThis = getThis(webView);
        pThis->setComposition(utf16Text, numUTF16CodeUnits, underlineRanges, numUnderlineRanges, startSelection, endSelection);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sConfirmComposition( WebViewApollo* webView
        , const uint16_t* utf16Text
        , unsigned long numUTF16CodeUnits )
    {
        ImplClass* const pThis = getThis(webView);
        pThis->confirmComposition(utf16Text, numUTF16CodeUnits);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sConfirmCurrentComposition( WebViewApollo* webView, unsigned char preserveSelection )
    {
        ImplClass* const pThis = getThis(webView);
        pThis->confirmComposition(preserveSelection);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetSelectedRange( WebViewApollo* webView
        , int* startSelection
        , int* endSelection
        , unsigned char* isEditableUnsignedChar )
    {
        ImplClass* const pThis = getThis(webView);
        bool isEditable;
        bool result = pThis->getSelectedRange(*startSelection, *endSelection, isEditable);
        *isEditableUnsignedChar = isEditable;
        return result;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSetSelectedRange( WebViewApollo* webView
        , int startSelection
        , int endSelection )
    {
        ImplClass* const pThis = getThis(webView);
        pThis->setSelectedRange(startSelection, endSelection);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetCompositionRange( WebViewApollo* webView
        , int *startComposition
        , int *endComposition )
    {
        ImplClass* const pThis = getThis(webView);
        return pThis->getCompositionRange(*startComposition, *endComposition);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetFirstRectForCharacterRange( WebViewApollo* webView
        , int startRange
        , int endRange
        , WebIntRect* rect )
    {
        ImplClass* const pThis = getThis(webView);
        *rect = pThis->getFirstRectForCharacterRange(startRange, endRange);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetTextForCharacterRange( WebViewApollo* webView
        , int startRange
        , int endRange
        , uint16_t* text )
    {
        ImplClass* const pThis = getThis(webView);
        return pThis->getTextForCharacterRange(startRange, endRange, text);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnDragEnter( WebViewApollo* pWebView
                                                                    , int localX
                                                                    , int localY
                                                                    , int screenX
                                                                    , int screenY
                                                                    , unsigned char const shiftKey
                                                                    , unsigned char const ctrlKey
                                                                    , unsigned char const altKey
                                                                    , unsigned char const metaKey
                                                                    , WebScriptProxyVariant* dataTransfer )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bHandled = pThis->onDragEnter(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey, dataTransfer);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnDragOver( WebViewApollo* pWebView
                                                                            , int localX
                                                                            , int localY
                                                                            , int screenX
                                                                            , int screenY
                                                                            , unsigned char const shiftKey
                                                                            , unsigned char const ctrlKey
                                                                            , unsigned char const altKey
                                                                            , unsigned char const metaKey
                                                                            , WebScriptProxyVariant* dataTransfer )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bHandled = pThis->onDragOver(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey, dataTransfer);
        return bHandled ? 1 :0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnDragDrop( WebViewApollo* pWebView
                                                                            , int localX
                                                                            , int localY
                                                                            , int screenX
                                                                            , int screenY
                                                                            , unsigned char const shiftKey
                                                                            , unsigned char const ctrlKey
                                                                            , unsigned char const altKey
                                                                            , unsigned char const metaKey
                                                                            , WebScriptProxyVariant* dataTransfer )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        bool const bHandled = pThis->onDragDrop(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey, dataTransfer);
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnDragExit( WebViewApollo* pWebView
                                                                   , int localX
                                                                   , int localY
                                                                   , int screenX
                                                                   , int screenY
                                                                   , unsigned char const shiftKey
                                                                   , unsigned char const ctrlKey
                                                                   , unsigned char const altKey
                                                                   , unsigned char const metaKey
                                                                   , WebScriptProxyVariant* dataTransfer )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        pThis->onDragExit(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey, dataTransfer);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnDragStart( WebViewApollo* pWebView
                                                                             , int localX
                                                                             , int localY
                                                                             , int screenX
                                                                             , int screenY
                                                                             , unsigned char const shiftKey
                                                                             , unsigned char const ctrlKey
                                                                             , unsigned char const altKey
                                                                             , unsigned char const metaKey)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        pThis->onDragStart(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnDragUpdate( WebViewApollo* pWebView
                                                                             , int localX
                                                                             , int localY
                                                                             , int screenX
                                                                             , int screenY
                                                                             , unsigned char shiftKey
                                                                             , unsigned char ctrlKey
                                                                             , unsigned char altKey
                                                                             , unsigned char metaKey)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool bShiftKey = shiftKey != 0;
        bool bCtrlKey = ctrlKey != 0;
        bool bAltKey = altKey != 0;
        bool bMetaKey = metaKey != 0;
        pThis->onDragUpdate(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnDragComplete( WebViewApollo* pWebView
                                                                       , int localX
                                                                       , int localY
                                                                       , int screenX
                                                                       , int screenY
                                                                       , unsigned char const shiftKey
                                                                       , unsigned char const ctrlKey
                                                                       , unsigned char const altKey
                                                                       , unsigned char const metaKey
                                                                       , const uint16_t *dropAction)
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bShiftKey = shiftKey != 0;
        bool const bCtrlKey = ctrlKey != 0;
        bool const bAltKey = altKey != 0;
        bool const bMetaKey = metaKey != 0;
        pThis->onDragComplete(localX, localY, screenX, screenY, bShiftKey, bCtrlKey, bAltKey, bMetaKey, dropAction);
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnCut( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        unsigned char const bHandled = pThis->onCut();
        return bHandled ? 1 : 0;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnCopy( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bHandled = pThis->onCopy();
        return bHandled ? 1 : 0;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnPaste( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bHandled = pThis->onPaste();
        return bHandled ? 1 : 0;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sOnSelectAll( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        bool const bHandled = pThis->onSelectAll();
        return bHandled ? 1 : 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSendResizeEvent( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->sendResizeEvent();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSendScrollEvent( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->sendScrollEvent();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sConvertToRootContent( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->convertToRootContent();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSetPaintsDefaultBackground( WebViewApollo* pWebView, unsigned char newValue )
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->setShouldPaintDefaultBackground(newValue);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetContextMenuItems( WebViewApollo* pWebView, WebMenuItem*** items, int *nItems )
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->getContextMenuItems(items, nItems);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetContextMenuItemInfo( WebViewApollo* pWebView, WebMenuItem* item, short **title, unsigned int *titleCharLen, int *type, unsigned char *checked, unsigned char *enabled, int *action, WebMenuItem ***subMenuItems, int *numSubMenuItems )
    {
        ImplClass* const pThis = getThis(pWebView);
        pThis->getContextMenuItemInfo(item, title, titleCharLen, type, checked, enabled, action, subMenuItems, numSubMenuItems);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sHasFocusableContent( WebViewApollo* pWebView )
    {
        ImplClass* const pThis = getThis(pWebView);
        return pThis->hasFocusableContent();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSetPageGroupName(struct WebViewApollo* pWebFrame,
	    const uint16_t* groupName)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->setPageGroupName(groupName);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    WebString* WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetPageGroupName(struct WebViewApollo* pWebFrame)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        return pThis->getPageGroupName();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSetTextEncodingOverride( WebViewApollo* webView
																				, const uint16_t* utf16TextEncoding
																				, unsigned long numUTF16CodeUnits)
    {
        ImplClass* const pThis = getThis(webView);
        pThis->setTextEncodingOverride(utf16TextEncoding, numUTF16CodeUnits);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetTextEncodingOverride( WebViewApollo* webView
																				, uint16_t** utf16TextEncoding)
    {
        ImplClass* const pThis = getThis(webView);
        pThis->getTextEncodingOverride(utf16TextEncoding);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sSetTextEncodingFallback( WebViewApollo* webView
																				, const uint16_t* utf16TextEncoding
																				, unsigned long numUTF16CodeUnits)
    {
        ImplClass* const pThis = getThis(webView);
        pThis->setTextEncodingFallback(utf16TextEncoding, numUTF16CodeUnits);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebViewHelper<ImplClass>::sGetTextEncodingFallback( WebViewApollo* webView
																				, uint16_t** utf16TextEncoding)
    {
        ImplClass* const pThis = getThis(webView);
        pThis->getTextEncodingFallback(utf16TextEncoding);
    }

}
#endif
