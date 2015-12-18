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
#ifndef WebViewImpl_h
#define WebViewImpl_h

#include <WebKitApollo/WebKit.h>
#include <WebKitApollo/helpers/WebViewHelper.h>
#include <memory>

#include <wtf/RefPtr.h>
#include <wtf/HashSet.h>
#include "WebFocusManager.h"
#include <Timer.h>
#include <FloatRect.h>
#include <ScrollTypes.h>

namespace WebCore
{
    class Editor;
    class KeyboardEvent;
    class Page;
	class Frame;
    class FrameApollo;
    class FrameView;
    class PlatformKeyboardEvent;
    class Widget;
    class PluginView;
    class Node;
    class RenderObject;
    class RenderWidget;
}

namespace WebKitApollo
{

    class FrameScrollViewImpl;
    class WebPluginWidgetImpl;
    class WebWidgetImpl;
    class WebEditorClient;

	class WebDebugListenerImpl;
#if ENABLE(APOLLO_PROFILER)
    class WebSamplerImpl;
#endif
    struct JSCInitializeThreadingHelper;

    WEBKIT_APOLLO_PROTO1 WebViewApollo* WEBKIT_APOLLO_PROTO1 webViewCreate(WebHost* pWebHost, WebWindow* pTargetWindow, WebError* pErrorInfo);

    class WebFrameImpl;

    class WebViewImpl : public WebViewHelper< WebViewImpl >
    {
        friend class WebViewHelper< WebViewImpl >;
    public:
        WebViewImpl(WebHost* pHost, WebWindow* pTargetWindow);
        virtual ~WebViewImpl();
        
        WebCore::Page* page();
        WebCore::Frame* mainFrame();
		WebCore::Frame* focusedOrMainFrame();
        
        void putFrameOnLifeSupport(WTF::PassRefPtr<WebCore::Frame>);
        
        void addPluginImpl(WebCore::Widget* const);
        void removePluginImpl(WebCore::Widget* const);

		WebHost *webHost() { return m_pHost; }
        
        void selectionChanged();
        void activateIMEForPlugin();
        void deactivateIMEForPlugin();
		bool getInputMethodState();
       
        bool doDefaultKeyPressActionForEditor(WebCore::Editor* const editor, WebCore::KeyboardEvent* keyEvent);
        
        WebWindow* getTargetWindow(){ return m_pTargetWindow; }

    private:
        void destroy();
        WebFrameApollo* mainWebFrame();
        void layout();
        void setPageGroupName( const uint16_t* groupName );
        WebString* getPageGroupName();
        void paint(const WebIntRect* const pPaintRect);
        WebCore::Frame* focusedFrame();
               
	void platformSelectionChanged();
        void onContextMenuItemSelect( int const actionID, const uint16_t *title );
        bool onContextMenu( int const localX
                        , int const localY
                        , int const screenX
                        , int const screenY
                        , bool const shiftKey
                        , bool const ctrlKey
                        , bool const altKey
                        , bool const metaKey);

        bool onMouseDown( int const localX
                        , int const localY
                        , int const screenX
                        , int const screenY
                        , WebMouseButton const mouseButton
                        , unsigned int const clickCount
                        , bool const shiftKey
                        , bool const ctrlKey
                        , bool const altKey
                        , bool const metaKey);
        bool onMouseUp( int const localX
                      , int const localY
                      , int const screenX
                      , int const screenY
                      , WebMouseButton const mouseButton
                      , unsigned int const clickCount
                      , bool const shiftKey
                      , bool const ctrlKey
                      , bool const altKey
                      , bool const metaKey);
        bool onMouseMove( int const localX
                        , int const localY
                        , int const screenX
                        , int const screenY
                        , bool const shiftKey
                        , bool const ctrlKey
                        , bool const altKey
                        , bool const metaKey
                        , bool const buttonDown);
        bool onMouseWheel( int const localX
                         , int const localY
                         , int const screenX
                         , int const screenY
                         , int const deltaX
                         , int const deltaY
                         , bool const shiftKey
                         , bool const ctrlKey
                         , bool const altKey
                         , bool const metaKey);

        // focus in/out
        void onFocusIn(const uint16_t *direction, int length);
        void onFocusOut();

        // window activation/deactivation
        void onWindowActivate();
        void onWindowDeactivate();

        // treat the focus webkit behaviour
        void processFocusIn();
        void processFocusOut();
        void setFocusAndFocusedFrame();
        
        bool hasFocusableContent();

        bool onKeyDown( unsigned int const winCharCode
                      , unsigned int const winKeyCode
                      , bool const isShiftKey
                      , bool const isCtrlKey
                      , bool const isAltKey
                      , bool const isMetaKey
                      , bool const isAutoRepeat);
        bool onKeyUp( unsigned int const winCharCode
					, unsigned int const winKeyCode
					, bool const isShiftKey
					, bool const isCtrlKey
					, bool const isAltKey
                    , bool const isMetaKey);
        bool onKeyPress( unsigned int const winCharCode
					, bool const isShiftKey
					, bool const isCtrlKey
					, bool const isAltKey
                    , bool const isMetaKey
                    , bool const isAutoRepeat);

        bool insertText(const uint16_t* utf16Text, unsigned long numUTF16CodeUnits);
        void setComposition(const uint16_t* utf16Text
            , unsigned long numUTF16CodeUnits
            , WebViewCompositionUnderlineRange* underlineRanges
            , unsigned long numUnderlineRanges
            , int startSelection
            , int endSelection);
        void confirmComposition(const uint16_t* utf16Text, unsigned long numUTF16CodeUnits);
        void confirmComposition(bool preserveSelection);
        bool getSelectedRange(int& startSelection, int& endSelection, bool& isEditable);
        void setSelectedRange(int startSelection, int endSelection);
        bool getCompositionRange(int& startComposition, int& endComposition);
        WebIntRect getFirstRectForCharacterRange(int startRange, int endRange);
        bool getTextForCharacterRange(int startRange, int endRange, uint16_t* text);
        bool onDragEnter( int localX
                        , int localY
                        , int screenX
                        , int screenY
                        , bool shiftKey
                        , bool ctrlKey
                        , bool altKey
                        , bool metaKey
                        , WebScriptProxyVariant* dataTransfer);
        bool onDragOver( int localX
                        , int localY
                        , int screenX
                        , int screenY
                        , bool shiftKey
                        , bool ctrlKey
                        , bool altKey
                        , bool metaKey
                        , WebScriptProxyVariant* dataTransfer);
        bool onDragDrop( int localX
                        , int localY
                        , int screenX
                        , int screenY
                        , bool shiftKey
                        , bool ctrlKey
                        , bool altKey
                        , bool metaKey
                        , WebScriptProxyVariant* dataTransfer);
        void onDragExit( int localX
                        , int localY
                        , int screenX
                        , int screenY
                        , bool shiftKey
                        , bool ctrlKey
                        , bool altKey
                        , bool metaKey
                        , WebScriptProxyVariant* dataTransfer);
        void onDragStart( int localX
                        , int localY
                        , int screenX
                        , int screenY
                        , bool shiftKey
                        , bool ctrlKey
                        , bool altKey
                        , bool metaKey);
        void onDragUpdate( int localX
                         , int localY
                         , int screenX
                         , int screenY
                         , bool shiftKey
                         , bool ctrlKey
                         , bool altKey
                         , bool metaKey);
        void onDragComplete( int localX
                           , int localY
                           , int screenX
                           , int screenY
                           , bool shiftKey
                           , bool ctrlKey
                           , bool altKey
                           , bool metaKey
                           , const uint16_t *operation);
        bool onCut();
        bool onCopy();
        bool onPaste();
        bool onSelectAll();
        void sendResizeEvent();
        void sendScrollEvent();
        void convertToRootContent();
        void setShouldPaintDefaultBackground(bool newValue);
        void updatePlugins(WebCore::Timer<WebViewImpl>*);
        void setTextEncodingOverride(const uint16_t* utf16TextEncoding, unsigned long numUTF16CodeUnits);
        void getTextEncodingOverride(uint16_t** utf16TextEncoding);
        void setTextEncodingFallback(const uint16_t* utf16TextEncoding, unsigned long numUTF16CodeUnits);
        void getTextEncodingFallback(uint16_t** utf16TextEncodings);

        void getContextMenuItems(WebMenuItem*** items, int *nItems);
        void getContextMenuItemInfo(WebMenuItem* item, short **title, unsigned int *titleCharLen, int *type, unsigned char *checked, unsigned char *enabled, int *action, WebMenuItem ***subMenuItems, int *numSubMenuItems);

        void initSettings();

        void onLifeSupportTimer(WebCore::Timer<WebViewImpl>*);
        void onRemoveFocusTimer(WebCore::Timer<WebViewImpl>*);

        WebEditorClient* webEditorClient() const;

        WebCore::FrameView* focusedOrMainFrameView() const;
        WebCore::PluginView* pluginViewForFocusedNode();
        
        bool doDefaultKeyPressAction(const WebCore::PlatformKeyboardEvent&);
        
        // undo/redo
        bool isUndoCommand(WebCore::PlatformKeyboardEvent& keyEvent) const;
        bool isRedoCommand(WebCore::PlatformKeyboardEvent& keyEvent) const;
        
        WebViewImpl(const WebViewImpl& other);
        WebViewImpl& operator=(const WebViewImpl& other);
    
        bool m_inDestructor;
        WebHost* const m_pHost;
        WebWindow* m_pTargetWindow;
        std::auto_ptr<WebCore::Page> m_pPage;
        WTF::HashSet<WTF::RefPtr<WebCore::Frame> > m_framesOnLifeSupport;
        WebCore::Timer<WebViewImpl> m_lifeSupportTimer;
        WebCore::Timer<WebViewImpl> m_removeFocusTimer;
        
        bool m_canTransformToRootContent;
        
        //Set of all plugin Impl's in this view.
        WTF::HashSet<WebCore::Widget*> m_pluginImpls;

        bool m_hasHostFocus;
        bool m_isIMEForPluginActive;

		WTF::RefPtr<WebDebugListenerImpl> m_debugListener;
#if ENABLE(APOLLO_PROFILER)
        WTF::RefPtr<WebSamplerImpl> m_sampler;
#endif

        WebCore::Timer<WebViewImpl> m_updatePluginsTimer;
    };
}


#endif
