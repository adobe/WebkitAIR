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
#ifndef WebView_h
#define WebView_h

#include <WebKitApollo/WebConfig.h>

struct WebViewApollo;
struct WebHost;
struct WebError;
struct WebWindow;
struct WebFrameApollo;
struct WebIntRect;
class WebMenuItem;
struct WebString;

enum WebMouseButton {
    WebMouseButtonLeft = 0,
    WebMouseButtonMiddle,
    WebMouseButtonRight,

    /*-----------------------------------------
      Add new enum values above this comment!!
    -------------------------------------------*/
    WebMouseButtonLast
};

/* on mac bool is defined in stdbool.h as: #define bool bool */
#ifdef bool
#undef bool
#endif

#define bool DO_NO_USE_BOOLS_IN_THIS_FILE

typedef WEBKIT_APOLLO_PROTO1 WebViewApollo* ( WEBKIT_APOLLO_PROTO2 *WebViewCreateFunction )( WebHost* pWebHost
                                                                                     , WebWindow* pTargetWindow
                                                                                     , WebError* pErrorInfo );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewDestroyFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 WebFrameApollo* ( WEBKIT_APOLLO_PROTO2 *WebViewMainFrameFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewLayoutFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewPaintFunction )( WebViewApollo* pWebView
                                                                                , const WebIntRect* pPaintRect );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnContextMenuItemSelectFunction )( WebViewApollo* pWebView, int const actionID, const uint16_t *title);
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnContextMenuFunction )( WebViewApollo* pWebView
                                                                                      , int const localX
                                                                                      , int const localY
                                                                                      , int const screenX
                                                                                      , int const screenY
                                          											  , unsigned char const shiftKey
                                                                                      , unsigned char const ctrlKey
                                                                                      , unsigned char const altKey
                                                                                      , unsigned char const metaKey );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnMouseDownFunction )( WebViewApollo* pWebView
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
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnMouseUpFunction )( WebViewApollo* pWebView
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
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnMouseMoveFunction )( WebViewApollo* pWebView
                                                                                      , int const localX
                                                                                      , int const localY
                                                                                      , int const screenX
                                                                                      , int const screenY
                                                                                      , unsigned char const shiftKey
                                                                                      , unsigned char const ctrlKey
                                                                                      , unsigned char const altKey
                                                                                      , unsigned char const metaKey
                                                                                      , unsigned char const buttonDown);
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnMouseWheelFunction )( WebViewApollo* pWebView
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
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnFocusInFunction )( WebViewApollo* pWebView, const uint16_t *direction, int length );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnFocusOutFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnWindowActivateFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnWindowDeactivateFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnKeyDownFunction )( WebViewApollo* pWebView
                                                                                             , unsigned int const winCharCode
                                                                                             , unsigned int const winKeyCode
                                                                                             , unsigned char const isShiftKey
                                                                                             , unsigned char const isCtrlKey
                                                                                             , unsigned char const isAltKey
                                                                                             , unsigned char const metaKey
                                                                                             , unsigned char const isAutoRepeat );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnKeyUpFunction )( WebViewApollo* pWebView
                                                                                           , unsigned int const winCharCode
                                                                                           , unsigned int const winKeyCode
                                                                                           , unsigned char const isShiftKey
                                                                                           , unsigned char const isCtrlKey
                                                                                           , unsigned char const isAltKey
                                                                                           , unsigned char const metaKey );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnKeyPressFunction )( WebViewApollo* pWebView
                                                                                             , unsigned int const winCharCode
                                                                                             , unsigned char const isShiftKey
                                                                                             , unsigned char const isCtrlKey
                                                                                             , unsigned char const isAltKey
                                                                                             , unsigned char const metaKey
                                                                                             , unsigned char const isAutoRepeat );

typedef WEBKIT_APOLLO_PROTO1 unsigned char (WEBKIT_APOLLO_PROTO2* WebViewInsertTextFunction)(WebViewApollo* webView
                                                                                       , const uint16_t* utf16Text
                                                                                       , unsigned long numUTF16CodeUnits);

typedef struct
{
    int start;
    int end;
    struct { uint8_t red, green, blue, alpha; } color;
    unsigned char thick;
} WebViewCompositionUnderlineRange;

typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewSetCompositionFunction)( WebViewApollo* webView
                                                                                       , const uint16_t* utf16Text
                                                                                       , unsigned long numUTF16CodeUnits
                                                                                       , WebViewCompositionUnderlineRange* underlineRanges
                                                                                       , unsigned long numUnderlineRanges
                                                                                       , int startSelection
                                                                                       , int endSelection );

typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewConfirmCompositionFunction)( WebViewApollo* webView
                                                                                           , const uint16_t* utf16Text
                                                                                           , unsigned long numUTF16CodeUnits );

typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewConfirmCurrentCompositionFunction)( WebViewApollo* webView, unsigned char preserveSelection );

typedef WEBKIT_APOLLO_PROTO1 unsigned char (WEBKIT_APOLLO_PROTO2* WebViewGetSelectedRangeFunction)( WebViewApollo* webView
                                                                                         , int *startSelection
                                                                                         , int *endSelection
                                                                                         , unsigned char *isEditable );

typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewSetSelectedRangeFunction)( WebViewApollo* webView
                                                                                         , int startSelection
                                                                                         , int endSelection );

typedef WEBKIT_APOLLO_PROTO1 unsigned char (WEBKIT_APOLLO_PROTO2* WebViewGetCompositionRangeFunction)( WebViewApollo* webView
                                                                                            , int *startComposition
                                                                                            , int *endComposition );

typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewGetFirstRectForCharacterRangeFunction)( WebViewApollo* webView
                                                                                                            , int startRange
                                                                                                            , int endRange
                                                                                                            , WebIntRect *rect );

typedef WEBKIT_APOLLO_PROTO1 unsigned char (WEBKIT_APOLLO_PROTO2* WebViewGetTextForCharacterRangeFunction)( WebViewApollo* webView
                                                                                                            , int startRange
                                                                                                            , int endRange
                                                                                                            , uint16_t *text );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnDragEnterFunction )( WebViewApollo* pWebView
                                                                                      , int const localX
                                                                                      , int const localY
                                                                                      , int const screenX
                                                                                      , int const screenY
                                                                                      , unsigned char const shiftKey
                                                                                      , unsigned char const ctrlKey
                                                                                      , unsigned char const altKey
                                                                                      , unsigned char const metaKey
                                                                                      , WebScriptProxyVariant *dataTransfer );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnDragOverFunction )( WebViewApollo* pWebView
                                                                                              , int const localX
                                                                                              , int const localY
                                                                                              , int const screenX
                                                                                              , int const screenY
                                                                                              , unsigned char const shiftKey
                                                                                              , unsigned char const ctrlKey
                                                                                              , unsigned char const altKey
                                                                                              , unsigned char const metaKey
                                                                                              , WebScriptProxyVariant *dataTransfer );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnDragDropFunction )( WebViewApollo* pWebView
                                                                                              , int const localX
                                                                                              , int const localY
                                                                                              , int const screenX
                                                                                              , int const screenY
                                                                                              , unsigned char const shiftKey
                                                                                              , unsigned char const ctrlKey
                                                                                              , unsigned char const altKey
                                                                                              , unsigned char const metaKey
                                                                                              , WebScriptProxyVariant *dataTransfer );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnDragExitFunction )( WebViewApollo* pWebView
                                                                                     , int const localX
                                                                                     , int const localY
                                                                                     , int const screenX
                                                                                     , int const screenY
                                                                                     , unsigned char const shiftKey
                                                                                     , unsigned char const ctrlKey
                                                                                     , unsigned char const altKey
                                                                                     , unsigned char const metaKey
                                                                                     , WebScriptProxyVariant *dataTransfer );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnDragStartFunction )( WebViewApollo* pWebView
                                                                                               , int const localX
                                                                                               , int const localY
                                                                                               , int const screenX
                                                                                               , int const screenY
                                                                                               , unsigned char const shiftKey
                                                                                               , unsigned char const ctrlKey
                                                                                               , unsigned char const altKey
                                                                                               , unsigned char const metaKey );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnDragUpdateFunction )( WebViewApollo* pWebView
                                                                                               , int const localX
                                                                                               , int const localY
                                                                                               , int const screenX
                                                                                               , int const screenY
                                                                                               , unsigned char const shiftKey
                                                                                               , unsigned char const ctrlKey
                                                                                               , unsigned char const altKey
                                                                                               , unsigned char const metaKey );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewOnDragCompleteFunction )( WebViewApollo* pWebView
                                                                                         , int const localX
                                                                                         , int const localY
                                                                                         , int const screenX
                                                                                         , int const screenY
                                                                                         , unsigned char const shiftKey
                                                                                         , unsigned char const ctrlKey
                                                                                         , unsigned char const altKey
                                                                                         , unsigned char const metaKey
                                                                                         , const uint16_t *dropEffect );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnCutFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnCopyFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnPasteFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewOnSelectAllFunction )( WebViewApollo* pWebView );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewSendResizeEventFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewSendScrollEventFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewConvertToRootContentFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewSetShouldPaintDefaultBackgroundFunction )(	WebViewApollo* pWebView, unsigned char newValue );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewGetContextMenuItemsFunction )( WebViewApollo* pWebView, WebMenuItem*** items, int *nItems );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewGetContextMenuItemInfoFunction )( WebViewApollo* pWebView, WebMenuItem* item, short **title, unsigned int *titleCharLen, int *type, unsigned char *checked, unsigned char *enabled, int *action, WebMenuItem ***subMenuItems, int *numSubMenuItems );


typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebViewHasFocusableContentFunction )( WebViewApollo* pWebView );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebViewSetPageGroupName )( struct WebViewApollo* pWebFrame, const uint16_t* groupName );
typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebViewGetPageGroupName )( struct WebViewApollo* pWebFrame );
typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewSetTextEncodingOverrideFunction)( WebViewApollo* webView
																								, const uint16_t* utf16Text
																								, unsigned long numUTF16CodeUnits);
typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewGetTextEncodingOverrideFunction)( WebViewApollo* webView
																								, uint16_t** utf16Text);
typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewSetTextEncodingFallbackFunction)( WebViewApollo* webView
																								, const uint16_t* utf16Text
																								, unsigned long numUTF16CodeUnits);
typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2* WebViewGetTextEncodingFallbackFunction)( WebViewApollo* webView
																								, uint16_t** utf16Texts);


struct WebViewVTable {
    unsigned long m_vTableSize;
    WebViewDestroyFunction destroy;
    WebViewMainFrameFunction mainFrame;
    WebViewLayoutFunction layout;
    WebViewPaintFunction paint;
    WebViewOnContextMenuItemSelectFunction onContextMenuItemSelect;
    WebViewOnContextMenuFunction onContextMenu;
    WebViewOnMouseDownFunction onMouseDown;
    WebViewOnMouseUpFunction onMouseUp;
    WebViewOnMouseMoveFunction onMouseMove;
    WebViewOnMouseWheelFunction onMouseWheel;
    WebViewOnFocusInFunction onFocusIn;
    WebViewOnFocusOutFunction onFocusOut;
    WebViewOnWindowActivateFunction onWindowActivate;
    WebViewOnWindowDeactivateFunction onWindowDeactivate;
    WebViewOnKeyDownFunction onKeyDown;
    WebViewOnKeyUpFunction onKeyUp;
    WebViewOnKeyPressFunction onKeyPress;
    WebViewInsertTextFunction insertText;
    WebViewSetCompositionFunction setComposition;
    WebViewConfirmCompositionFunction confirmComposition;
    WebViewConfirmCurrentCompositionFunction confirmCurrentComposition;
    WebViewGetSelectedRangeFunction getSelectedRange;
    WebViewSetSelectedRangeFunction setSelectedRange;
    WebViewGetCompositionRangeFunction getCompositionRange;
    WebViewGetFirstRectForCharacterRangeFunction getFirstRectForCharacterRange;
	WebViewGetTextForCharacterRangeFunction getTextForCharacterRange;
    WebViewOnDragEnterFunction onDragEnter;
    WebViewOnDragOverFunction onDragOver;
    WebViewOnDragDropFunction onDragDrop;
    WebViewOnDragExitFunction onDragExit;
    WebViewOnDragStartFunction onDragStart;
    WebViewOnDragUpdateFunction onDragUpdate;
    WebViewOnDragCompleteFunction onDragComplete;
    WebViewOnCutFunction onCut;
    WebViewOnCopyFunction onCopy;
    WebViewOnPasteFunction onPaste;
    WebViewOnSelectAllFunction onSelectAll;
    WebViewSendResizeEventFunction sendResizeEvent;
    WebViewSendScrollEventFunction sendScrollEvent;
    WebViewConvertToRootContentFunction convertToRootContent;
	WebViewSetShouldPaintDefaultBackgroundFunction setShouldPaintDefaultBackground;
    WebViewGetContextMenuItemsFunction getContextMenuItems;
    WebViewGetContextMenuItemInfoFunction getContextMenuItemInfo;
    WebViewHasFocusableContentFunction hasFocusableContent;
    WebViewSetPageGroupName setPageGroupName;
    WebViewGetPageGroupName getPageGroupName;
	WebViewSetTextEncodingOverrideFunction setTextEncodingOverride;
	WebViewGetTextEncodingOverrideFunction getTextEncodingOverride;
	WebViewSetTextEncodingFallbackFunction setTextEncodingFallback;
	WebViewGetTextEncodingFallbackFunction getTextEncodingFallback;
};

struct WebViewApollo {
    const WebViewVTable* m_pVTable;
};

#undef bool

#endif
