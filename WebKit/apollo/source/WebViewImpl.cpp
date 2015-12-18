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
#include "WebViewImpl.h"

#include "WebFrameImpl.h"

#include "PlatformMouseEvent.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformWheelEvent.h"

#include <Page.h>
#include <FocusController.h>
#include <FrameLoader.h>
#include <Cache.h>

#include "ContextMenuItemApollo.h"
#include "DragController.h"
#include "DragData.h"
#include "Editor.h"
#include "EventHandler.h"
#include "Frame.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "TextIterator.h"
#include "WebChromeClient.h"
#include "WebContextMenuClient.h"
#include "WebDragClient.h"
#include "WebEditorClient.h"
#include <ClipboardApollo.h>
#include <ClipboardApolloHelper.h>
#include <Settings.h>
#include <apollo/proxy_obj.h>
#include "ContextMenu.h"
#include <WebKitApollo/WebKeyboardCodes.h>
#include <wtf/InstanceCounter.h>
#include <DocumentLoader.h>
#include <InitializeThreading.h>
#include "WebDebugListenerImpl.h"
#include "KeyboardEvent.h"
#include "EventHandler.h"
#include "WebMainFrameView.h"
#include "JSLock.h"
#include "ScriptController.h"
#include "WebSamplerImpl.h"

#include "RenderObject.h"
#include "RenderWidget.h"
#include "PluginView.h"
#include "Widget.h"

#include "ContextMenuController.h"
#include "ScriptController.h"
#include "DOMWrapperWorld.h"

#if PLATFORM(APOLLO_DARWIN) || PLATFORM(APOLLO_UNIX)
#include <sys/time.h>
#elif PLATFORM(APOLLO_WINDOWS)
#include <windows.h>
#endif

namespace WebKitApollo
{
extern WebKitAPIHostFunctions* g_HostFunctions;

WebViewImpl::WebViewImpl(WebHost* pHost, WebWindow* pTargetWindow)
    : m_inDestructor(false)
    , m_pHost(pHost)
    , m_pTargetWindow(pTargetWindow)
    , m_pPage(0)
    , m_lifeSupportTimer(this, &WebViewImpl::onLifeSupportTimer)
    , m_removeFocusTimer(this, &WebViewImpl::onRemoveFocusTimer)
    , m_canTransformToRootContent(true)
    , m_hasHostFocus(false)
    , m_isIMEForPluginActive(false)
    , m_updatePluginsTimer(this, &WebViewImpl::updatePlugins)
{
    // initializeThreading must be called before the Page ctor is called
    JSC::initializeThreading();
    m_pPage = std::auto_ptr<WebCore::Page>(
        new WebCore::Page(
            new WebChromeClient(pTargetWindow, pHost), 
            new WebContextMenuClient(), 
            new WebEditorClient(this), 
            new WebDragClient(),
            0, /*inspector client*/
            0, /*plugin halter client*/
            0 /*gelocation client*/
        )
    );
    initSettings();
    WebFrameImpl::construct(this, m_pHost, m_pPage.get());
    m_pPage->mainFrame()->tree()->setName(WebCore::String());
    m_pPage->mainFrame()->init();

	// HTMLDebugListenerImpl::getInstance() returns 0 if the debugger is disabled
	m_debugListener = WebDebugListenerImpl::getInstance();
	if (m_debugListener)
		m_pPage->setDebugger( m_debugListener->getDebugger() );

    WebCore::SecurityOrigin::setLocalLoadPolicy(WebCore::SecurityOrigin::AllowLocalLoadsForLocalOnly);
    
#if ENABLE(APOLLO_PROFILER)
    m_sampler = WebSamplerImpl::getInstance();
#endif
}

WebViewImpl::~WebViewImpl()
{
	m_pPage->setDebugger(0);
	m_debugListener = 0;

#if ENABLE(APOLLO_PROFILER)
    m_sampler = 0;
#endif
    
    m_lifeSupportTimer.stop();
    m_removeFocusTimer.stop();

    // clear the cache otherwise there will be CachedResources leaks on a debug build
    WebCore::Cache* cache = WebCore::cache();
    if (cache && !cache->disabled())
        cache->clear();

    WebCore::Frame* const pFrame = mainFrame();
    m_inDestructor = true;
    ASSERT(pFrame);
    WebCore::FrameLoader* const pLoader = pFrame->loader();
    ASSERT(pLoader);
    pLoader->detachFromParent();
    m_pPage = std::auto_ptr<WebCore::Page>();
}

WebCore::Page* WebViewImpl::page()
{
    ASSERT(m_pPage.get());
    return m_pPage.get();
}

WebCore::Frame* WebViewImpl::mainFrame()
{
    ASSERT(m_pPage.get());
    ASSERT(!m_inDestructor);
    WebCore::Frame* const pMainFrame = m_pPage->mainFrame();
    ASSERT(pMainFrame);
    return pMainFrame;
}

WebCore::Frame* WebViewImpl::focusedOrMainFrame()
{
    ASSERT(m_pPage.get());
    ASSERT(!m_inDestructor);
	WebCore::Frame* const pMainFrame = m_pPage->focusController()->focusedOrMainFrame();
    ASSERT(pMainFrame);
    return pMainFrame;
}
 
WebCore::PluginView* WebViewImpl::pluginViewForFocusedNode()
{
    WebCore::Frame const *focusedFrame = focusedOrMainFrame();
    if(!focusedFrame)
        return 0;
    
     WebCore::Node* focusedNode = focusedFrame->document()->focusedNode();

    if (!focusedNode)
        return 0;
    
    WebCore::RenderObject*  renderer = focusedNode->renderer();
    if (!renderer || !renderer->isWidget())
        return 0;
    
    WebCore::Widget*  widget = static_cast<WebCore::RenderWidget*>(renderer)->widget();
    if (!widget || !widget->isPluginView())
        return 0;
    
    return static_cast<WebCore::PluginView*>(widget);
}

void WebViewImpl::putFrameOnLifeSupport(WTF::PassRefPtr<WebCore::Frame> frameArg)
{
    ASSERT(frameArg);
    WTF::RefPtr<WebCore::Frame> const frame(frameArg);
    m_framesOnLifeSupport.add(frame);
    if ((!m_inDestructor) && (!m_lifeSupportTimer.isActive()))
        m_lifeSupportTimer.startOneShot(0);
}

void WebViewImpl::onLifeSupportTimer(WebCore::Timer<WebViewImpl>* timer)
{
    ASSERT(m_framesOnLifeSupport.size());
    ASSERT(timer == &m_lifeSupportTimer);
    m_framesOnLifeSupport.clear();
}


void WebViewImpl::onRemoveFocusTimer(WebCore::Timer<WebViewImpl>* timer)
{
    ASSERT(timer == &m_removeFocusTimer);
    processFocusOut();
}

WebFrameApollo* WebViewImpl::mainWebFrame()
{
    m_canTransformToRootContent = false;
    WebCore::Frame* const pMainFrame = mainFrame();
    WebFrameApollo* result = NULL;
    if (pMainFrame)
    {
        WebFrameImpl* const pMainWebFrameImpl = WebFrameImpl::kit(pMainFrame);
        ASSERT(pMainWebFrameImpl);
        result = pMainWebFrameImpl->getWebFrame();
        ASSERT(result);
    }
    return result;
}

WebCore::Frame* WebViewImpl::focusedFrame()
{
    WebCore::FocusController* controller = m_pPage->focusController();
    return controller->focusedFrame();
}

void cancelMousePressedRecursive( WebCore::Frame* frame );
void cancelMousePressedRecursive( WebCore::Frame* frame )
{
    ASSERT(frame);
    ASSERT(frame->eventHandler());
    frame->eventHandler()->setMousePressed( false );
    WebCore::FrameTree* tree = frame->tree();
    ASSERT(tree);
    unsigned childCount = tree->childCount();
    unsigned index = 0;
    for( ;index<childCount; index++) {
        WebCore::Frame* child = tree->child(index);
        ASSERT(child);
        cancelMousePressedRecursive(child);
    }
    
}

namespace {
    // handles widgetImpl being null.
    static inline bool widgetIsSameOrDescendantOf(WebCore::FrameView* const fv, WebCore::Widget* const widget)
    {
        ASSERT(fv);
        WebCore::Widget* const fvAsWidget = fv;
        WebCore::Widget* currentWidget = widget;

        while ((currentWidget) && (fvAsWidget != currentWidget))
            currentWidget = static_cast<WebCore::Widget*>(currentWidget->parent());
            
        return fvAsWidget == currentWidget;
    }
}

void WebViewImpl::destroy()
{
    m_canTransformToRootContent = false;
    delete this;
}

void WebViewImpl::layout()
{
    m_canTransformToRootContent = false;
    WebCore::Frame* const theMainFrame = mainFrame();
    if (theMainFrame && theMainFrame->view())
        theMainFrame->view()->layout();
}

static double currentTime()
{
#if PLATFORM(APOLLO_DARWIN) || PLATFORM(APOLLO_UNIX)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + 0.000001 * tv.tv_usec;
#elif PLATFORM(APOLLO_WINDOWS)
    return 0.001 * GetTickCount();
#else
    #error "No implementation of currentTime()."
#endif
}

bool WebViewImpl::onMouseDown( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , WebMouseButton const mouseButton
                             , unsigned int const clickCount
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey)
{
    

    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);
    ASSERT(static_cast<unsigned long>(WebCore::LeftButton) == static_cast<unsigned long>(WebMouseButtonLeft));
    ASSERT(static_cast<unsigned long>(WebCore::MiddleButton) == static_cast<unsigned long>(WebMouseButtonMiddle));
    ASSERT(static_cast<unsigned long>(WebCore::RightButton) == static_cast<unsigned long>(WebMouseButtonRight));
    WebCore::MouseButton webCoreMouseButton = static_cast<WebCore::MouseButton>(mouseButton);
    WebCore::PlatformMouseEvent mouseEvent( localPoint
                                          , screenPoint
                                          , webCoreMouseButton
                                          , WebCore::MouseEventPressed
                                          , static_cast< int >( clickCount )
                                          , shiftKey
                                          , ctrlKey
                                          , altKey
                                          , metaKey
                                          , currentTime());

    WebCore::Frame* const pMainFrame = mainFrame();
    if (pMainFrame)
        return pMainFrame->eventHandler()->handleMousePressEvent(mouseEvent);

    return false;
}

void WebViewImpl::onContextMenuItemSelect( int const actionID, const uint16_t *title )
{
    WebCore::ContextMenuItem item(WebCore::ActionType, static_cast<WebCore::ContextMenuAction>(actionID), WebCore::String((const UChar *) title));

    m_pPage->contextMenuController()->contextMenuItemSelected(&item);
}

bool WebViewImpl::onContextMenu( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey)
{
    
    WebCore::IntPoint localPoint(localX, localY);
    WebCore::IntPoint screenPoint(screenX, screenY);

    WebCore::PlatformMouseEvent mouseEvent( localPoint
                                          , screenPoint
                                          , WebCore::RightButton
                                          , WebCore::MouseEventReleased
                                          , 0
                                          , shiftKey
                                          , ctrlKey
                                          , altKey
                                          , metaKey
                                          , static_cast<double>(time(0)));

    m_pPage->contextMenuController()->clearContextMenu();

    WebCore::Frame* const pMainFrame = focusedOrMainFrame();
    if (pMainFrame)
        return pMainFrame->eventHandler()->sendContextMenuEvent(mouseEvent);

    return false;
}

bool WebViewImpl::onMouseUp( int const localX
                           , int const localY
                           , int const screenX
                           , int const screenY
                           , WebMouseButton const mouseButton
                           , unsigned int const clickCount
                           , bool const shiftKey
                           , bool const ctrlKey
                           , bool const altKey
                           , bool const metaKey)
{
    
    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);
    ASSERT(static_cast<unsigned long>(WebCore::LeftButton) == static_cast<unsigned long>(WebMouseButtonLeft));
    ASSERT(static_cast<unsigned long>(WebCore::MiddleButton) == static_cast<unsigned long>(WebMouseButtonMiddle));
    ASSERT(static_cast<unsigned long>(WebCore::RightButton) == static_cast<unsigned long>(WebMouseButtonRight));
    WebCore::MouseButton webCoreMouseButton = static_cast<WebCore::MouseButton>(mouseButton);
    WebCore::PlatformMouseEvent mouseEvent( localPoint
                                          , screenPoint
                                          , webCoreMouseButton
                                          , WebCore::MouseEventReleased
                                          , static_cast< int >( clickCount )
                                          , shiftKey
                                          , ctrlKey
                                          , altKey
                                          , metaKey
                                          , currentTime());

    WebCore::Frame* const pMainFrame = mainFrame();
    if (pMainFrame)
        return pMainFrame->eventHandler()->handleMouseReleaseEvent(mouseEvent);

    return false;
}

bool WebViewImpl::onMouseMove( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey
                             , bool const buttonDown)
{
    
    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);
    WebCore::MouseButton webCoreMouseButton = static_cast<WebCore::MouseButton>(buttonDown ? WebCore::LeftButton : WebCore::NoButton);
    WebCore::PlatformMouseEvent mouseEvent( localPoint
                                          , screenPoint
                                          , webCoreMouseButton
                                          , WebCore::MouseEventMoved
                                          , 0
                                          , shiftKey
                                          , ctrlKey
                                          , altKey
                                          , metaKey
                                          , currentTime());
    WebCore::Frame* const pMainFrame = mainFrame();
    if (pMainFrame)
        return pMainFrame->eventHandler()->handleMouseMoveEvent(mouseEvent);

    return false;
}

bool WebViewImpl::onMouseWheel( int const localX
                              , int const localY
                              , int const screenX
                              , int const screenY
                              , int const deltaX
                              , int const deltaY
                              , bool const shiftKey
                              , bool const ctrlKey
                              , bool const altKey
                              , bool const metaKey)
{
    
    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);
    bool isAccepted = false;
    bool isContinuous = false;
    WebCore::PlatformWheelEvent wheelEvent( localPoint
                                          , screenPoint
                                          , static_cast< float >( deltaX )
                                          , static_cast< float >( deltaY )
                                          , isAccepted
                                          , shiftKey
                                          , ctrlKey
                                          , altKey
                                          , metaKey
                                          , isContinuous );

    WebCore::Frame* const pMainFrame = mainFrame();
    if (pMainFrame)
        return pMainFrame->eventHandler()->handleWheelEvent(wheelEvent);

    return false;
}


// treat the focus webkit behaviour
void WebViewImpl::setFocusAndFocusedFrame()
{
    ASSERT(m_pPage.get());

    WebCore::FocusController *controller = m_pPage->focusController();
    ASSERT(controller);

    controller->setFocused(true);

    // try to mimic the behaviour of focusDocumentView (EventHandler::handleMousePressEvent)
    // and set a focused frame.
    if (!controller->focusedFrame()) {
        controller->setFocusedFrame(m_pPage->mainFrame());
    }
}

void WebViewImpl::processFocusIn()
{
    ASSERT(m_pPage.get());

    WebCore::FocusController *controller = m_pPage->focusController();
    ASSERT(controller);

    controller->setActive(true);
    setFocusAndFocusedFrame();
}

void WebViewImpl::processFocusOut()
{
    // We need special handling if the focused node is a plugin.
    WebCore::PluginView *view = pluginViewForFocusedNode();
    if (view)
        view->setFocus(false);
    
    WebCore::FocusController* controller = m_pPage->focusController();
    controller->setFocused(false);
    controller->setActive(false);
}

void WebViewImpl::onFocusIn(const uint16_t *direction, int length)
{
    m_canTransformToRootContent = false;
    m_removeFocusTimer.stop();

    ASSERT(m_pPage.get());
    ASSERT(m_pPage->focusController());

    m_hasHostFocus = true;

    // m_isIMEForPluginActive is set either by a SWF-in-HTML plugin being focus/unfocused
    // or when an HTML input is focused / blured
    if (m_isIMEForPluginActive)
        webHost()->m_pVTable->activateIMEForPlugin(webHost());

    WebCore::FocusController* controller = m_pPage->focusController();
    controller->setActive(true);

	WebCore::Frame* frame = controller->focusedOrMainFrame();

    static const WebCore::String s_bottom("bottom");
    static const WebCore::String s_top("top");
    
    if(WTF::Unicode::umemcasecmp(s_bottom.characters(), (const UChar*)direction, s_bottom.length()) == 0)
    {
        controller->setFocusedNode(0, frame);
        controller->setFocusedFrame(0);
        controller->setFocused(true);
        controller->advanceFocus(WebCore::FocusDirectionBackward, 0);
    }
   
    else if(WTF::Unicode::umemcasecmp(s_top.characters(), (const UChar*)direction, s_top.length()) == 0)
    {
        controller->setFocusedNode(0, frame);
        controller->setFocusedFrame(0);
        controller->setFocused(true);
        controller->advanceFocus(WebCore::FocusDirectionForward, 0);
    }
    else if (frame) {
        setFocusAndFocusedFrame();
    }

    webHost()->m_pVTable->selectionChanged(webHost());
}

bool WebViewImpl::hasFocusableContent()
{
    WebCore::FocusController* controller = m_pPage->focusController();
    
    WebCore::Node* forwardNode = controller->getNextFocusNode(WebCore::FocusDirectionForward, 0);
    WebCore::Node* backwardNode = controller->getNextFocusNode(WebCore::FocusDirectionBackward, 0);

    return forwardNode || backwardNode;
}

void WebViewImpl::onFocusOut()
{
    m_canTransformToRootContent = false;
    ASSERT(m_pPage.get());
    ASSERT(m_pPage->focusController());

    m_hasHostFocus = false;
	if (WebCore::Frame* frame = focusedFrame()) 
	{
		if(frame->editor()->hasComposition())
		{
			frame->editor()->confirmCompositionWithoutDisturbingSelection();
			webHost()->m_pVTable->compositionAbandoned(webHost());
		}
	}

    // m_isIMEForPluginActive is set either by a SWF-in-HTML plugin being focus/unfocused
    // or when a HTML input is focused / blured
    if (m_isIMEForPluginActive) 
    {
        webHost()->m_pVTable->compositionAbandoned(webHost());
        webHost()->m_pVTable->deactivateIMEForPlugin(webHost());
    }

    if (focusedOrMainFrame())
        m_removeFocusTimer.startOneShot(0);
}

void WebViewImpl::onWindowActivate()
{
    processFocusIn();
}

void WebViewImpl::onWindowDeactivate()
{
    onRemoveFocusTimer(&m_removeFocusTimer);
}

// check if the only key pressed is meta on Mac or Ctrl on Win/Lin
static bool commandKeyPressed(WebCore::PlatformKeyboardEvent& keyEvent)
{
#if OS(DARWIN)
    // the only key pressed is meta
    bool onlyMetaKeyPressed = 
        (keyEvent.metaKey() && 
        !keyEvent.shiftKey() && 
        !keyEvent.ctrlKey() && 
        !keyEvent.altKey());
    return onlyMetaKeyPressed;
#else
    // the only key pressed is ctrl
    bool onlyCtrlKeyPressed = 
        (!keyEvent.metaKey() && 
        !keyEvent.shiftKey() && 
        keyEvent.ctrlKey() && 
        !keyEvent.altKey());
    return onlyCtrlKeyPressed;
#endif
}
    
// meta/ctrl + Z    
bool  WebViewImpl::isUndoCommand(WebCore::PlatformKeyboardEvent& keyEvent) const
{
    return ((keyEvent.windowsVirtualKeyCode() == Web_VK_Z) && commandKeyPressed(keyEvent));
}

// meta/ctrl + Y    
bool  WebViewImpl::isRedoCommand(WebCore::PlatformKeyboardEvent& keyEvent) const
{
    return ((keyEvent.windowsVirtualKeyCode() == Web_VK_Y) && commandKeyPressed(keyEvent));
}
    

bool WebViewImpl::onKeyDown( unsigned int const winCharCode
                           , unsigned int const winKeyCode
                           , bool const isShiftKey
                           , bool const isCtrlKey
                           , bool const isAltKey
                           , bool const isMetaKey
                           , bool const isAutoRepeat)
{
    

    WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
                                           , winCharCode
                                           , winKeyCode
                                           , isShiftKey
                                           , isCtrlKey
                                           , isAltKey 
                                           , isMetaKey
                                           , isAutoRepeat);
	WebCore::Frame* const pMainFrame = focusedOrMainFrame();
    if (!pMainFrame)
        return false;

    bool handled = pMainFrame->eventHandler()->keyEvent(keyEvent);

#if OS(WINDOWS) && ENABLE(NETSCAPE_PLUGIN_API)
    WebCore::PluginView *view = pluginViewForFocusedNode();
    // Returning true here (the event is handled) will cause the keypress event not to be generated anymore.
    // On windows, this means that the plugin will never receive WM_CHAR events.
    if (handled && view) {
        return false;
    }
#endif

    if(handled)
        return handled;

    int windowsKeyCode = keyEvent.windowsVirtualKeyCode();
    if ((windowsKeyCode == Web_VK_BACK && keyEvent.shiftKey()) || (windowsKeyCode == Web_VK_RIGHT && keyEvent.ctrlKey()))
        return m_pPage->goForward();
    else if (windowsKeyCode == Web_VK_BACK || (windowsKeyCode == Web_VK_LEFT && keyEvent.ctrlKey()))
        return m_pPage->goBack();
    
    // undo/redo
    // Ctrl+Z is undo, Ctrl+Y is redo
    // in both cases, mark the event as handled so that OnKeyPress is not called after leaving this function
    // For both commands, execute ends up in WebEditorClient::undo/redo which call canUndo/canRedo before making any processing
    if (isUndoCommand(keyEvent)) {
        pMainFrame->editor()->command("Undo").execute(); 
        return true;
    } else if (isRedoCommand(keyEvent)) {
        pMainFrame->editor()->command("Redo").execute(); 
        return true;
    }    
    
    // Need to scroll the page if the arrow keys, space(shift), pgup/dn, or home/end are hit.
    WebCore::ScrollDirection direction;
    WebCore::ScrollGranularity granularity;
    switch (windowsKeyCode) {
        case Web_VK_LEFT:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollLeft;
            break;
        case Web_VK_RIGHT:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollRight;
            break;
        case Web_VK_UP:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollUp;
            break;
        case Web_VK_DOWN:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollDown;
            break;
        case Web_VK_HOME:
            granularity = WebCore::ScrollByDocument;
            direction = WebCore::ScrollUp;
            break;
        case Web_VK_END:
            granularity = WebCore::ScrollByDocument;
            direction = WebCore::ScrollDown;
            break;
        case Web_VK_PRIOR:
            granularity = WebCore::ScrollByPage;
            direction = WebCore::ScrollUp;
            break;
        case Web_VK_NEXT:
            granularity = WebCore::ScrollByPage;
            direction = WebCore::ScrollDown;
            break;
        default:
            return false;
    }
    
    if (!pMainFrame->eventHandler()->scrollOverflow(direction, granularity)) {
        handled = pMainFrame->view()->scroll(direction, granularity);
        WebCore::Frame* parent = pMainFrame->tree()->parent();
        while(!handled && parent) {
            handled = parent->view()->scroll(direction, granularity);
            parent = parent->tree()->parent();
        }
    }
    
    return handled;
}


bool WebViewImpl::onKeyUp( unsigned int const winCharCode
                         , unsigned int const winKeyCode
                         , bool const isShiftKey
                         , bool const isCtrlKey
                         , bool const isAltKey
                         , bool const isMetaKey)
{
    
    WebCore::PlatformKeyboardEvent keyEvent(  WebCore::PlatformKeyboardEvent::KeyUp
                                           , winCharCode
                                           , winKeyCode
                                           , isShiftKey
                                           , isCtrlKey
                                           , isAltKey
                                           , isMetaKey 
                                           , false);
    WebCore::Frame* const pMainFrame = focusedOrMainFrame();
    if (pMainFrame)
        return pMainFrame->eventHandler()->keyEvent(keyEvent);
    
    return false;
}

bool WebViewImpl::onKeyPress( unsigned int const winCharCode
                         , bool const isShiftKey
                         , bool const isCtrlKey
                         , bool const isAltKey
                         , bool const isMetaKey
                         , bool const isAutoRepeat)
{
    
    WebCore::PlatformKeyboardEvent keyEvent(  WebCore::PlatformKeyboardEvent::Char
                                           , winCharCode
                                           , 0
                                           , isShiftKey
                                           , isCtrlKey
                                           , isAltKey
                                           , isMetaKey 
    
                                           , isAutoRepeat);

    bool handled = false;
    WebCore::Frame* const pMainFrame = focusedOrMainFrame();
    if (pMainFrame)
        handled = pMainFrame->eventHandler()->keyEvent(keyEvent);
    if (!handled)
        handled = doDefaultKeyPressAction(keyEvent);

    return handled;
}

inline bool WebViewImpl::doDefaultKeyPressAction(const WebCore::PlatformKeyboardEvent& keyEvent)
{
    bool handled = false;
    if (keyEvent.text() == " ") {
		WebCore::Frame *frame = focusedOrMainFrame();
		if(!frame)
			return false;

		WebCore::ScrollDirection direction = keyEvent.shiftKey() ? WebCore::ScrollUp : WebCore::ScrollDown;
		if (!frame->eventHandler()->scrollOverflow(direction, WebCore::ScrollByPage)) {
			handled = frame->view()->scroll(direction, WebCore::ScrollByPage);
			WebCore::Frame* parent = frame->tree()->parent();
			while(!handled && parent) {
				handled = parent->view()->scroll(direction, WebCore::ScrollByPage);
				parent = parent->tree()->parent();
			}
		}
    }
    return handled;
}

bool WebViewImpl::doDefaultKeyPressActionForEditor(WebCore::Editor* const editor, WebCore::KeyboardEvent* keyEvent)
{
    ASSERT(editor);
    ASSERT(keyEvent);
    bool handled = false;
    if (keyEvent->charCode() == ' ' && editor->canEdit())
        handled = editor->insertText(WebCore::String(" "), 0);
    return handled;
}

bool WebViewImpl::insertText(const uint16_t* utf16Text, unsigned long numUTF16CodeUnits)
{
    WebCore::String text(reinterpret_cast<const UChar*>(utf16Text), numUTF16CodeUnits);
    
#if OS(DARWIN)
    // if the plugin is focused, then send it the text
    // otherwise send the text in WebKit
    WebCore::PluginView *view = pluginViewForFocusedNode();
    if (view)
		return view->handleInsertText(text);   
#endif
    
    WebCore::Frame* frame = focusedFrame();
    if(frame && frame->editor()->canEdit())
        return frame->editor()->insertText(text, 0);
    
    return false;
}

void WebViewImpl::setComposition(const uint16_t* utf16Text, unsigned long numUTF16CodeUnits,
    WebViewCompositionUnderlineRange *underlineRanges, unsigned long numUnderlineRanges,
    int startSelection, int endSelection)
{
    WebCore::Frame* frame = focusedFrame();

    if (!frame || !frame->selection()->isContentEditable())
        return;

    WebCore::String composition(reinterpret_cast<const UChar*>(utf16Text), numUTF16CodeUnits);
    Vector<WebCore::CompositionUnderline> underlines;
    for (unsigned long i=0; i<numUnderlineRanges; i++) {
        uint8_t r, g, b, a;
        r = underlineRanges[i].color.red;
        g = underlineRanges[i].color.green;
        b = underlineRanges[i].color.blue;
        a = underlineRanges[i].color.alpha;
        WebCore::Color c(r, g, b, a);
        WebCore::CompositionUnderline u(underlineRanges[i].start,
            underlineRanges[i].end, c, underlineRanges[i].thick);
        underlines.append(u);
    }

    if (WebCore::Frame* frame = focusedFrame())
        frame->editor()->setComposition(composition, underlines, startSelection, endSelection);
}

void WebViewImpl::confirmComposition(const uint16_t* utf16Text, unsigned long numUTF16CodeUnits)
{
    WebCore::String composition(reinterpret_cast<const UChar*>(utf16Text), numUTF16CodeUnits);
    if (WebCore::Frame* frame = focusedFrame())
        frame->editor()->confirmComposition(composition);
}

void WebViewImpl::confirmComposition(bool preserveSelection)
{
    if (WebCore::Frame* frame = focusedFrame()) {
        if (preserveSelection)
            frame->editor()->confirmCompositionWithoutDisturbingSelection();
        else
            frame->editor()->confirmComposition();
    }
}

static bool getRangeExtents(WebCore::Frame* frame, WebCore::Range* range, int& startPosition, int& endPosition)
{
    int exception = 0;
    if (!range || !range->startContainer())
        return false;

    WebCore::Element* selectionRoot = frame->selection()->rootEditableElement();
    WebCore::Element* scope = selectionRoot ? selectionRoot : frame->document()->documentElement();

    if (range->startContainer(exception) != scope && !range->startContainer(exception)->isDescendantOf(scope))
        return false;
    if (range->endContainer(exception) != scope && !range->endContainer(exception)->isDescendantOf(scope))
        return false;

    RefPtr<WebCore::Range> testRange = WebCore::Range::create(scope->document(), scope, 0, range->startContainer(exception), range->startOffset(exception));
    ASSERT(testRange->startContainer(exception) == scope);
    startPosition = WebCore::TextIterator::rangeLength(testRange.get());

    testRange->setEnd(range->endContainer(exception), range->endOffset(exception), exception);
    ASSERT(testRange->startContainer(exception) == scope);
    endPosition = WebCore::TextIterator::rangeLength(testRange.get());

    return true;
}

PassRefPtr<WebCore::Range> rangeWithExtents(WebCore::Frame* frame, int startPosition, int endPosition);
PassRefPtr<WebCore::Range> rangeWithExtents(WebCore::Frame* frame, int startPosition, int endPosition)
{
    WebCore::Element* selectionRoot = frame->selection()->rootEditableElement();
    WebCore::Element* scope = selectionRoot ? selectionRoot : frame->document()->documentElement();
    return WebCore::TextIterator::rangeFromLocationAndLength(scope, startPosition, endPosition - startPosition);
}

bool WebViewImpl::getSelectedRange(int& startSelection, int& endSelection, bool& isEditable)
{
    if (WebCore::Frame* frame = focusedFrame()) {
        isEditable = frame->selection()->isContentEditable();
        return getRangeExtents(frame, frame->selection()->toNormalizedRange().get(), startSelection, endSelection);
    }
    return false;
}

void WebViewImpl::setSelectedRange(int startSelection, int endSelection)
{
    if (WebCore::Frame* frame = focusedFrame()) {
        RefPtr<WebCore::Range> domRange = rangeWithExtents(frame, startSelection, endSelection);
        if (domRange)
            frame->selection()->setSelection(WebCore::VisibleSelection(domRange.get(), WebCore::SEL_DEFAULT_AFFINITY));
    }
}

bool WebViewImpl::getCompositionRange(int& startComposition, int& endComposition)
{
    if (WebCore::Frame* frame = focusedFrame())
        return getRangeExtents(frame, frame->editor()->compositionRange().get(), startComposition, endComposition);
    return false;
}

WebIntRect WebViewImpl::getFirstRectForCharacterRange(int startRange, int endRange)
{
    WebIntRect webFirstRect = {0, 0, 0, 0};
    if (WebCore::Frame* frame = focusedFrame()) {
        RefPtr<WebCore::Range> range(rangeWithExtents(frame, startRange, endRange));
        if (!range)
            return webFirstRect;
        WebCore::IntRect firstRect = frame->firstRectForRange(range.get());
        webFirstRect.m_left = firstRect.x();
        webFirstRect.m_top = firstRect.y();
        webFirstRect.m_right = firstRect.right();
        webFirstRect.m_bottom = firstRect.bottom();
    }
    return webFirstRect;
}

bool WebViewImpl::getTextForCharacterRange(int startRange, int endRange, uint16_t* utf16Text)
{
    ASSERT(startRange >= 0);
    ASSERT(startRange <= endRange);
    if (WebCore::Frame* frame = focusedFrame()) {
        RefPtr<WebCore::Range> range(rangeWithExtents(frame, startRange, endRange));
        WebCore::String text(range->text());
        ASSERT(text.length() <= unsigned(endRange - startRange + 1));
        ::memcpy(utf16Text, text.characters(), text.length() * sizeof(UChar));
        return true;
    }
    return false;
}

void WebViewImpl::selectionChanged()
{
    WebCore::Frame* targetFrame = focusedOrMainFrame();
    if (!targetFrame)
        return;
  
    if (targetFrame->editor()->ignoreCompositionSelectionChange())
        return;
    
    // set data to primary gtk's primary clipboard on selection event 
    platformSelectionChanged();

    if (targetFrame->editor()->hasComposition()) {
        unsigned int start, end;
        if (targetFrame->editor()->getCompositionSelection(start, end)) {
            webHost()->m_pVTable->compositionSelectionChanged(webHost(), start, end);
        } else {
            targetFrame->editor()->confirmCompositionWithoutDisturbingSelection();
            webHost()->m_pVTable->compositionAbandoned(webHost());
        }
    } else {
        webHost()->m_pVTable->selectionChanged(webHost());
    }
}

void WebViewImpl::activateIMEForPlugin()
{
    if (m_hasHostFocus && !m_isIMEForPluginActive)
        webHost()->m_pVTable->activateIMEForPlugin(webHost());
    m_isIMEForPluginActive = true;
}

void WebViewImpl::deactivateIMEForPlugin()
{
    if (m_hasHostFocus && m_isIMEForPluginActive)
        webHost()->m_pVTable->deactivateIMEForPlugin(webHost());
    m_isIMEForPluginActive = false;
}

bool WebViewImpl::onDragEnter( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey
                             , WebScriptProxyVariant* dataTransfer)
{
    WebCore::Frame* const pMainFrame = mainFrame();

    JSC::JSLock lock(false);

    JSC::ExecState* exec = pMainFrame->script()->globalObject(WebCore::mainThreadNormalWorld())->globalExec();
    JSC::JSValue dataTransferValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, dataTransfer);

    JSC::JSObject* dataTransferObject = dataTransferValue.toObject(exec);

    WebCore::ClipboardApolloHelper dragClipboard(dataTransferObject, exec);

    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);

    WebCore::DragData dragData(&dragClipboard, localPoint, screenPoint, dragClipboard.dragOperationAllowed());
    WebCore::DragOperation operation = m_pPage.get()->dragController()->dragEntered(&dragData);
    dragClipboard.setDragOperation(operation);
    return operation != WebCore::DragOperationNone;
}

bool WebViewImpl::onDragOver( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey
                             , WebScriptProxyVariant* dataTransfer)
{
    WebCore::Frame* const pMainFrame = mainFrame();

    JSC::JSLock lock(false);

    JSC::ExecState* exec = pMainFrame->script()->globalObject(WebCore::mainThreadNormalWorld())->globalExec();
    JSC::JSValue dataTransferValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, dataTransfer);
	JSC::JSObject* dataTransferObject = dataTransferValue.toObject(exec);

    WebCore::ClipboardApolloHelper dragClipboard(dataTransferObject, exec);

    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);

    WebCore::DragData dragData(&dragClipboard, localPoint, screenPoint, dragClipboard.dragOperationAllowed());
    WebCore::DragOperation operation = m_pPage.get()->dragController()->dragUpdated(&dragData);
    dragClipboard.setDragOperation(operation);
    return operation != WebCore::DragOperationNone;
}

bool WebViewImpl::onDragDrop( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey
                             , WebScriptProxyVariant* dataTransfer)
{
    WebCore::Frame* const pMainFrame = mainFrame();

    JSC::JSLock lock(false);

    JSC::ExecState* exec = pMainFrame->script()->globalObject(WebCore::mainThreadNormalWorld())->globalExec();
    JSC::JSValue dataTransferValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, dataTransfer);

    JSC::JSObject* dataTransferObject = dataTransferValue.toObject(exec);

    WebCore::ClipboardApolloHelper dragClipboard(dataTransferObject, exec);

    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);

    WebCore::DragData dragData(&dragClipboard, localPoint, screenPoint, dragClipboard.dragOperationAllowed());
    return m_pPage.get()->dragController()->performDrag(&dragData);
}

void WebViewImpl::onDragExit( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey
                             , WebScriptProxyVariant* dataTransfer)
{
    WebCore::Frame* const pMainFrame = mainFrame();

    JSC::JSLock lock(false);

    JSC::ExecState* exec = pMainFrame->script()->globalObject(WebCore::mainThreadNormalWorld())->globalExec();
    JSC::JSValue dataTransferValue = WebCore::ApolloScriptBridging::jsValueFromBridgingVariant(exec, dataTransfer);

    JSC::JSObject* dataTransferObject = dataTransferValue.toObject(exec);

    WebCore::ClipboardApolloHelper dragClipboard(dataTransferObject, exec);

    WebCore::IntPoint const localPoint(localX,localY);
    WebCore::IntPoint const screenPoint(screenX,screenY);

    WebCore::DragData dragData(&dragClipboard, localPoint, screenPoint, dragClipboard.dragOperationAllowed());
    m_pPage.get()->dragController()->dragExited(&dragData);
}

void WebViewImpl::onDragStart( int const localX
                             , int const localY
                             , int const screenX
                             , int const screenY
                             , bool const shiftKey
                             , bool const ctrlKey
                             , bool const altKey
                             , bool const metaKey)
{
}

void WebViewImpl::onDragUpdate(int localX
                              , int localY
                              , int screenX
                              , int screenY
                              , bool shiftKey
                              , bool ctrlKey
                              , bool altKey
                              , bool metaKey)
{
    WebCore::PlatformMouseEvent dragEvent(WebCore::IntPoint(localX, localY), WebCore::IntPoint(screenX, screenY), WebCore::LeftButton, WebCore::MouseEventMoved, 0, shiftKey, ctrlKey, altKey, metaKey, currentTime());
	WebCore::Frame* pFrame = mainFrame();
    if(pFrame) {
        // TODO: apollo integrate -> dragSourceMoveTo removed
		//pFrame->eventHandler()->dragSourceMovedTo(dragEvent);
    }
}

void WebViewImpl::onDragComplete( int const localX
                                 , int const localY
                                 , int const screenX
                                 , int const screenY
                                 , bool const shiftKey
                                 , bool const ctrlKey
                                 , bool const altKey
                                 , bool const metaKey
                                 , const uint16_t *dropAction)
{
    page()->dragController()->dragEnded();

	WebCore::Frame* const pMainFrame = mainFrame();
	if (pMainFrame)
	{
		{
			WebCore::PlatformMouseEvent dragEvent(WebCore::IntPoint(localX, localY), WebCore::IntPoint(screenX, screenY), WebCore::LeftButton, WebCore::MouseEventMoved, 0, shiftKey, ctrlKey, altKey, metaKey, currentTime());
			pMainFrame->eventHandler()->dragSourceEndedAt(dragEvent, WebCore::Clipboard::dragOpFromIEOp(WebCore::String((const UChar *) dropAction)));
		}
		{
			// Synthesize a mouse up event and send it to the mouse capture widget.
			WebCore::PlatformMouseEvent mouseUpEvent(WebCore::IntPoint(localX, localY), WebCore::IntPoint(screenX, screenY), WebCore::LeftButton, WebCore::MouseEventReleased, 0, shiftKey, ctrlKey, altKey, metaKey, currentTime());
			pMainFrame->eventHandler()->handleMouseReleaseEvent(mouseUpEvent);
		}
	}
}

bool WebViewImpl::onCut()
{
    WebCore::Frame* frame = focusedOrMainFrame();
    if(!frame)
        return false;
    
    // We need special handling if the focused node is a plugin.
#if ENABLE(NETSCAPE_PLUGIN_API)
    WebCore::PluginView *view = pluginViewForFocusedNode();
    if (view)
		return view->handleOnCut();
#endif

    WebCore::Editor* editor = frame->editor();
    return editor->command("Cut").execute();
}

bool WebViewImpl::onCopy()
{
    WebCore::Frame* frame = focusedOrMainFrame();
    if(!frame)
        return false;

#if ENABLE(NETSCAPE_PLUGIN_API)
    // We need special handling if the focused node is a plugin.
    WebCore::PluginView *view = pluginViewForFocusedNode();
    if (view)
		return view->handleOnCopy();
#endif

    WebCore::Editor* editor = frame->editor();
    return editor->command("Copy").execute();
}

bool WebViewImpl::onPaste()
{
    WebCore::Frame* frame = focusedOrMainFrame();
    if(!frame)
        return false;

#if ENABLE(NETSCAPE_PLUGIN_API)
    // We need special handling if the focused node is a plugin.
    WebCore::PluginView *view = pluginViewForFocusedNode();
    if (view)
		return view->handleOnPaste();
#endif
    
    WebCore::Editor* editor = frame->editor();
    return editor->command("Paste").execute();
}

bool WebViewImpl::onSelectAll()
{
    WebCore::Frame* frame = focusedOrMainFrame();
    if(!frame)
        return false;
    
#if ENABLE(NETSCAPE_PLUGIN_API)
    // We need special handling if the focused node is a plugin.
    WebCore::PluginView *view = pluginViewForFocusedNode();
    if (view)
		return view->handleSelectAll();
#endif

    WebCore::Editor* editor = frame->editor();
    return editor->command("SelectAll").execute();
}

void WebViewImpl::sendResizeEvent( )
{
    WebCore::Frame* const pMainFrame = mainFrame();
    if (pMainFrame && pMainFrame->view()) {
        pMainFrame->view()->updateScrollbarsAfterResize();
        pMainFrame->view()->setNeedsLayout();
    }
}

void WebViewImpl::sendScrollEvent( )
{
    m_canTransformToRootContent = false;
    WebCore::Frame* const pMainFrame = mainFrame();
    if (pMainFrame)
    {
        WebMainFrameView* view = static_cast<WebMainFrameView*>(pMainFrame->view());
        int XOffset, YOffset;
        m_pTargetWindow->m_pVTable->scrollOffset(m_pTargetWindow, &XOffset, &YOffset);
        
        view->setScrollPosition(WebCore::IntPoint(XOffset, YOffset));
        pMainFrame->eventHandler()->sendScrollEvent();
    }
}



void WebViewImpl::convertToRootContent()
{
    ASSERT(m_canTransformToRootContent);
    ASSERT(m_pTargetWindow);
    WebCore::Frame* const pMainFrame = mainFrame();
    ASSERT(pMainFrame);
    WebFrameImpl::kit(pMainFrame)->transformToRootContent(m_pTargetWindow);
    
    ASSERT(pMainFrame->view());    
    WebMainFrameView * pMainView = static_cast<WebMainFrameView*>(pMainFrame->view());
    pMainView->setCanHaveScrollbars(true);

    m_canTransformToRootContent = false;
    ASSERT(mainFrame());
    m_hasHostFocus = true;
}

void WebViewImpl::setShouldPaintDefaultBackground(bool newValue)
{
    WebCore::Frame* const pMainFrame = mainFrame();
    ASSERT( pMainFrame ); 
    if ( newValue )
        pMainFrame->view()->setBaseBackgroundColor( WebCore::Color::white );
    else
        pMainFrame->view()->setBaseBackgroundColor( WebCore::Color( WebCore::makeRGBA( 0x00, 0xff, 0x00, 0x00 ) ) );
    ASSERT(pMainFrame);
}

void WebViewImpl::getContextMenuItems(WebMenuItem*** items, int *nItems)
{
    if(items)
        *items = 0;

    if(nItems)
        *nItems = 0;

    WebCore::ContextMenu* menu = m_pPage->contextMenuController()->contextMenu();
    WebMenu* webMenu = menu ? menu->platformDescription() : 0;
    if (!webMenu || !items || !nItems)
        return;

    *items = webMenu->getItems();
    *nItems = webMenu->getNumItems();
}

void WebViewImpl::getContextMenuItemInfo(WebMenuItem* item, short **title, unsigned int *titleCharLen, int *type, unsigned char *checked, unsigned char *enabled, int *action, WebMenuItem ***subMenuItems, int *numSubMenuItems)
{
    if(title)
        *title = item->getTitle();

    if(titleCharLen)
        *titleCharLen = item->getTitleCharLen();

    if(type)
        *type = item->getType();

    if(checked)
        *checked = item->getChecked();

    if(enabled)
        *enabled = item->getEnabled();

    if(action)
        *action = item->getAction();

    if(subMenuItems)
        *subMenuItems = item->getSubMenuItems();

    if(numSubMenuItems)
        *numSubMenuItems = item->getNumSubMenuItems();
}

void WebViewImpl::updatePlugins(WebCore::Timer<WebViewImpl>* timer)
{
    ASSERT(timer == &m_updatePluginsTimer);
    bool canShowPlugins = false;
    bool canShowWindowedPlugins = false;
    m_pHost->m_pVTable->canShowPlugins(m_pHost, &canShowPlugins, &canShowWindowedPlugins);

    WTF::HashSet<WebCore::Widget*>::const_iterator ppCurrPluginImpl(m_pluginImpls.begin());
    WTF::HashSet<WebCore::Widget*>::const_iterator const ppLastPluginImpl(m_pluginImpls.end());
    while (ppCurrPluginImpl != ppLastPluginImpl) {
        WebCore::Widget* const pCurrPluginImpl = *ppCurrPluginImpl;
        ++ppCurrPluginImpl;
        ASSERT(pCurrPluginImpl);
        pCurrPluginImpl->updatePluginWindow(canShowPlugins, canShowWindowedPlugins);
    }
}

void WebViewImpl::setTextEncodingOverride(const uint16_t* utf16TextEncoding, unsigned long numUTF16CodeUnits)
{
    ASSERT((numUTF16CodeUnits == 0) || (utf16TextEncoding));
    // need to get to the frame loader and set the override text encoding.
    WebCore::Frame* frame = mainFrame();
    if (frame && frame->loader() && frame->loader()->documentLoader()) {
        WebCore::String encodingName(reinterpret_cast<const UChar*>(utf16TextEncoding), numUTF16CodeUnits);
        frame->loader()->documentLoader()->setOverrideEncoding(encodingName);
        if (encodingName.length() == 0) {
            ASSERT(m_pPage.get());
            WebCore::Settings* const pSettings = m_pPage->settings();
            ASSERT(pSettings);
            encodingName = pSettings->defaultTextEncodingName();
        }
        frame->loader()->reloadWithOverrideEncoding(encodingName);
    }
}

void WebViewImpl::getTextEncodingOverride(uint16_t** utf16TextEncoding)
{
    ASSERT(utf16TextEncoding);

    // need to get to the frame loader and set the override text encoding.
    WebCore::Frame* frame = mainFrame();
    if (utf16TextEncoding && frame && frame->loader() && frame->loader()->documentLoader()) {
        WebCore::String encodingName = frame->loader()->documentLoader()->overrideEncoding();
        *utf16TextEncoding = (uint16_t *)g_HostFunctions->allocBytes(sizeof(uint16_t)*(encodingName.length()+1));
        if (encodingName.length() > 0)
            memcpy(*utf16TextEncoding, encodingName.characters(), sizeof(uint16_t)*(encodingName.length()));
        (*utf16TextEncoding)[encodingName.length()] = 0;
    }
}

void WebViewImpl::setTextEncodingFallback(const uint16_t* utf16TextEncoding, unsigned long numUTF16CodeUnits)
{
    ASSERT((numUTF16CodeUnits == 0) || (utf16TextEncoding));
    if (numUTF16CodeUnits) {
        ASSERT(m_pPage.get());
        WebCore::Settings* const pSettings = m_pPage->settings();
        ASSERT(pSettings);
        WebCore::String const encodingName(reinterpret_cast<const UChar*>(utf16TextEncoding), numUTF16CodeUnits);
        pSettings->setDefaultTextEncodingName(encodingName);
    }
}

void WebViewImpl::getTextEncodingFallback(uint16_t** utf16TextEncoding)
{
    ASSERT(utf16TextEncoding);
    ASSERT(m_pPage.get());
    WebCore::Settings* const pSettings = m_pPage->settings();
    ASSERT(pSettings);
    if (utf16TextEncoding) {
        const WebCore::String &encodingName = pSettings->defaultTextEncodingName();
        *utf16TextEncoding = (uint16_t *)g_HostFunctions->allocBytes( sizeof(uint16_t)*(encodingName.length()+1) );
        if (encodingName.length() > 0)
            memcpy(*utf16TextEncoding, encodingName.characters(), sizeof(uint16_t)*(encodingName.length()));
        (*utf16TextEncoding)[encodingName.length()] = 0;
    }
}

void WebViewImpl::initSettings()
{
    ASSERT(m_pPage.get());
    WebCore::Settings* const pSettings = m_pPage->settings();
    ASSERT(pSettings);
    pSettings->setStandardFontFamily("Times");
    pSettings->setFixedFontFamily("Courier");
    pSettings->setSerifFontFamily("Times");
    pSettings->setSansSerifFontFamily("Helvetica");
    pSettings->setCursiveFontFamily("Apple Chancery");
    pSettings->setFantasyFontFamily("Papyrus");
    pSettings->setMinimumFontSize(1);
    pSettings->setMinimumLogicalFontSize(9);
    pSettings->setDefaultFontSize(16);
    pSettings->setDefaultFixedFontSize(13);
    pSettings->setLoadsImagesAutomatically(true);
    pSettings->setJavaScriptEnabled(true);
    pSettings->setJavaScriptCanOpenWindowsAutomatically(false);  // requires user gesture
    pSettings->setJavaEnabled(false);
    pSettings->setPluginsEnabled(true);
    pSettings->setPrivateBrowsingEnabled(false);
    pSettings->setDefaultTextEncodingName("ISO-8859-1");
    pSettings->setUserStyleSheetLocation(WebCore::KURL(WebCore::ParsedURLString, ""));
    pSettings->setShouldPrintBackgrounds(false);
    pSettings->setTextAreasAreResizable(false);
    pSettings->setEditableLinkBehavior(WebCore::EditableLinkDefaultBehavior);
    pSettings->setNeedsAdobeFrameReloadingQuirk(false);
    pSettings->setShrinksStandaloneImagesToFit(false);
}

void WebViewImpl::addPluginImpl(WebCore::Widget* const pluginImpl)
{
    ASSERT(!m_pluginImpls.contains(pluginImpl));
    m_pluginImpls.add(pluginImpl);
    ASSERT(m_pluginImpls.contains(pluginImpl));

    if (!m_updatePluginsTimer.isActive()) {
        static const int updatePluginTimerPeriodInMiliSeconds = 34;
        m_updatePluginsTimer.startRepeating( static_cast<double>(updatePluginTimerPeriodInMiliSeconds) / 1000 );
    }
}

void WebViewImpl::removePluginImpl(WebCore::Widget* const pluginImpl)
{
    // don't assert here because we also remove the plugin
    // from the RenderWidget before it is destroyed.
    // ASSERT(m_pluginImpls.contains(pluginImpl));
    m_pluginImpls.remove(pluginImpl);
    ASSERT(!m_pluginImpls.contains(pluginImpl));
    if (m_pluginImpls.isEmpty())
        m_updatePluginsTimer.stop();

}

WebEditorClient* WebViewImpl::webEditorClient() const
{
    WebCore::EditorClient* const editorClient = m_pPage->editorClient();
    ASSERT(editorClient);
    WebEditorClient* const webEditorClient = WebEditorClient::kit(editorClient);
    ASSERT(webEditorClient);
    return webEditorClient;
}

void WebViewImpl::setPageGroupName(const uint16_t* groupName)
{
    m_pPage->setGroupName(groupName ? WebCore::String((const UChar*)groupName) : WebCore::String());
}

WebString* WebViewImpl::getPageGroupName()
{
    WebCore::String groupName = m_pPage->groupName();

    return groupName.webString();
}

inline WebCore::FrameView* WebViewImpl::focusedOrMainFrameView() const
{
    WebCore::FocusController* const controller = m_pPage->focusController();
    WebCore::Frame* const frame = controller->focusedOrMainFrame();
    WebCore::FrameView* const view = frame->view();
    return view;
}

WEBKIT_APOLLO_PROTO1 WebViewApollo* WEBKIT_APOLLO_PROTO2 webViewCreate(WebHost* pWebHost, WebWindow* pTargetWindow, WebError* pErrorInfo )
{
    ASSERT(pErrorInfo);

    WebViewImpl* const pWebViewImpl = new WebViewImpl(pWebHost, pTargetWindow);
    
    if (pWebViewImpl) {
        pErrorInfo->m_Type = WebErrorNoError;
    }
    else {
        pErrorInfo->m_Type = WebErrorNoMemory;
    }
    return pWebViewImpl->getWebView();
}

}

