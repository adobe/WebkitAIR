/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "PluginView.h"

#include "Bridge.h"
#include "Document.h"
#include "DocumentLoader.h"
#include "Element.h"
#include "EventNames.h"
#include "FocusController.h"
#include "FrameLoader.h"
#include "FrameLoadRequest.h"
#include "FrameTree.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HostWindow.h"
#include "HTMLNames.h"
#include "HTMLPlugInElement.h"
#include "Image.h"
#include "JSDOMBinding.h"
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "NotImplemented.h"
#include "Page.h"
#include "PlatformMouseEvent.h"
#include "PlatformKeyboardEvent.h"
#include "PluginDebug.h"
#include "PluginPackage.h"
#include "PluginMainThreadScheduler.h"
#include "RenderLayer.h"
#include "ScriptController.h"
#include "Settings.h"
#include "npruntime_impl.h"
#include "runtime_root.h"
#include <runtime/JSLock.h>
#include <runtime/JSValue.h>
#include <wtf/RetainPtr.h>
#include "loader/apollo/FrameLoaderClientApollo.h"


using JSC::ExecState;
using JSC::Interpreter;
using JSC::JSLock;
using JSC::JSObject;
using JSC::JSValue;
using JSC::UString;

#if PLATFORM(QT)
#include <QWidget>
#include <QKeyEvent>
#include <QPainter>
#include "QWebPageClient.h"
QT_BEGIN_NAMESPACE
extern Q_GUI_EXPORT OSWindowRef qt_mac_window_for(const QWidget* w);
QT_END_NAMESPACE
#endif

#if PLATFORM(WX)
#include <wx/defs.h>
#include <wx/wx.h>
#endif

using std::min;

using namespace WTF;

namespace WebCore {

using namespace HTMLNames;

#ifndef NP_NO_CARBON
static int modifiersForEvent(UIEventWithKeyState *event);
#endif

static inline WindowRef nativeWindowFor(PlatformWidget widget)
{
#if PLATFORM(QT)
    if (widget)
        return static_cast<WindowRef>(qt_mac_window_for(widget));
#endif
#if PLATFORM(WX)
    if (widget)
        return (WindowRef)widget->MacGetTopLevelWindowRef();
#endif
    return 0;
}

static inline CGContextRef cgHandleFor(PlatformWidget widget)
{
#if PLATFORM(QT)
    if (widget)
        return (CGContextRef)widget->macCGHandle();
#endif
#if PLATFORM(WX)
    if (widget)
        return (CGContextRef)widget->MacGetCGContextRef();
#endif
    return 0;
}

static inline IntPoint topLevelOffsetFor(PlatformWidget widget)
{
#if PLATFORM(QT)
    if (widget) {
        PlatformWidget topLevel = widget->window();
        return widget->mapTo(topLevel, QPoint(0, 0)) + topLevel->geometry().topLeft() - topLevel->pos();
    }
#endif
#if PLATFORM(WX)
    if (widget) {
        PlatformWidget toplevel = wxGetTopLevelParent(widget);
        return toplevel->ScreenToClient(widget->GetScreenPosition());
    }
#endif
    return IntPoint();
}

// --------------- Lifetime management -----------------

bool PluginView::platformStart()
{
    ASSERT(m_isStarted);
    ASSERT(m_status == PluginStatusLoadedSuccessfully);

    if (m_drawingModel == NPDrawingModel(-1)) {
        // We default to QuickDraw, even though we don't support it,
        // since that's what Safari does, and some plugins expect this
        // behavior and never set the drawing model explicitly.
#ifndef NP_NO_QUICKDRAW
        m_drawingModel = NPDrawingModelQuickDraw;
#else
        // QuickDraw not available, so we have to default to CoreGraphics
        m_drawingModel = NPDrawingModelCoreGraphics;
#endif
    }

#if defined(XP_MACOSX)&& !PLATFORM(APOLLO)
    if (m_eventModel == NPEventModel(-1)) {
        // If the plug-in did not specify an event model
        // we default to Carbon, when it is available.
#ifndef NP_NO_CARBON
        m_eventModel = NPEventModelCarbon;
#else
        m_eventModel = NPEventModelCocoa;
#endif
    }
#endif // defined(XP_MACOSX)&& !PLATFORM(APOLLO)

#if defined(XP_MACOSX)&& !PLATFORM(APOLLO)
    // Gracefully handle unsupported drawing or event models. We can do this
    // now since the drawing and event model can only be set during NPP_New.
#ifndef NP_NO_CARBON
    NPBool eventModelSupported;
    if (getValueStatic(NPNVariable(NPNVsupportsCarbonBool + m_eventModel), &eventModelSupported) != NPERR_NO_ERROR
            || !eventModelSupported) {
#endif
        m_status = PluginStatusCanNotLoadPlugin;
        LOG(Plugins, "Plug-in '%s' uses unsupported event model %s",
                m_plugin->name().utf8().data(), prettyNameForEventModel(m_eventModel));
        return false;
#ifndef NP_NO_CARBON
    }
#endif
#endif // defined(XP_MACOSX)&& !PLATFORM(APOLLO)

#ifndef NP_NO_QUICKDRAW
    NPBool drawingModelSupported;
    if (getValueStatic(NPNVariable(NPNVsupportsQuickDrawBool + m_drawingModel), &drawingModelSupported) != NPERR_NO_ERROR
            || !drawingModelSupported) {
#endif
        m_status = PluginStatusCanNotLoadPlugin;
        LOG(Plugins, "Plug-in '%s' uses unsupported drawing model %s",
                m_plugin->name().utf8().data(), prettyNameForDrawingModel(m_drawingModel));
        return false;
#ifndef NP_NO_QUICKDRAW
    }
#endif

#if PLATFORM(QT)
    // Set the platformPluginWidget only in the case of QWebView so that the context menu appears in the right place.
    // In all other cases, we use off-screen rendering
    if (QWebPageClient* client = m_parentFrame->view()->hostWindow()->platformPageClient()) {
        if (QWidget* widget = qobject_cast<QWidget*>(client->pluginParent()))
            setPlatformPluginWidget(widget);
    }
#endif
#if PLATFORM(WX)
    if (wxWindow* widget = m_parentFrame->view()->hostWindow()->platformPageClient())
        setPlatformPluginWidget(widget);
#endif

    // Create a fake window relative to which all events will be sent when using offscreen rendering
    if (!platformPluginWidget()) {
#if defined(XP_MACOSX)&& !PLATFORM(APOLLO)
#ifndef NP_NO_CARBON
        // Make the default size really big. It is unclear why this is required but with a smaller size, mouse move
        // events don't get processed. Resizing the fake window to flash's size doesn't help.
        ::Rect windowBounds = { 0, 0, 1000, 1000 };
        CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes, &windowBounds, &m_fakeWindow);
        // Flash requires the window to be hilited to process mouse move events.
        HiliteWindow(m_fakeWindow, true);
#endif
#endif // defined(XP_MACOSX)&& !PLATFORM(APOLLO)
    }

    show();

    // TODO: Implement null timer throttling depending on plugin activation
    m_nullEventTimer.set(new Timer<PluginView>(this, &PluginView::nullEventTimerFired));
    m_nullEventTimer->startRepeating(0.02);

#if defined(XP_MACOSX)&& !PLATFORM(APOLLO)
    m_lastMousePos.h = m_lastMousePos.v = 0;
#endif // defined(XP_MACOSX)&& !PLATFORM(APOLLO)

    return true;
}

void PluginView::platformDestroy()
{
    if (platformPluginWidget())
        setPlatformPluginWidget(0);
    else {
#if defined(XP_MACOSX)&& !PLATFORM(APOLLO)
        CGContextRelease(m_contextRef);
#ifndef NP_NO_CARBON
        if (m_fakeWindow)
            DisposeWindow(m_fakeWindow);
#endif
#endif // defined(XP_MACOSX)&& !PLATFORM(APOLLO)
    }
}

// Used before the plugin view has been initialized properly, and as a
// fallback for variables that do not require a view to resolve.
bool PluginView::platformGetValueStatic(NPNVariable variable, void* value, NPError* result)
{
    switch (variable) {
    case NPNVToolkit:
        *static_cast<uint32_t*>(value) = 0;
        *result = NPERR_NO_ERROR;
        return true;

    case NPNVjavascriptEnabledBool:
        *static_cast<NPBool*>(value) = true;
        *result = NPERR_NO_ERROR;
        return true;

#ifndef NP_NO_CARBON
    case NPNVsupportsCarbonBool:
        *static_cast<NPBool*>(value) = true;
        *result = NPERR_NO_ERROR;
        return true;

#endif
    case NPNVsupportsCocoaBool:
        *static_cast<NPBool*>(value) = false;
        *result = NPERR_NO_ERROR;
        return true;

    // CoreGraphics is the only drawing model we support
    case NPNVsupportsCoreGraphicsBool:
        *static_cast<NPBool*>(value) = true;
        *result = NPERR_NO_ERROR;
        return true;

#ifndef NP_NO_QUICKDRAW
    // QuickDraw is deprecated in 10.5 and not supported on 64-bit
    case NPNVsupportsQuickDrawBool:
#endif
    case NPNVsupportsOpenGLBool:
    case NPNVsupportsCoreAnimationBool:
        *static_cast<NPBool*>(value) = false;
        *result = NPERR_NO_ERROR;
        return true;

    default:
        return false;
    }
}

// Used only for variables that need a view to resolve
bool PluginView::platformGetValue(NPNVariable variable, void* value, NPError* error)
{
    return false;
}

void PluginView::setParent(ScrollView* parent)
{
    LOG(Plugins, "PluginView::setParent(%p)", parent);

    Widget::setParent(parent);

    if (parent)
        init();
#if PLATFORM(APOLLO)
    else
	{
        FrameLoaderClientApollo::clientApollo(m_parentFrame.get())->deactivateIMEForPlugin();
    }
#endif // PLATFORM(APOLLO)
}

// -------------- Geometry and painting ----------------

void PluginView::show()
{
    LOG(Plugins, "PluginView::show()");
	
    if (!m_plugin)
        return;
    
    const bool visibleStateChanged = (m_isEnabled && !isVisible() /*&& m_attachedToWindow*/);
    setSelfVisible(true);
	
	Widget::show();
    
    if(parent() && visibleStateChanged)
  	  invalidate();
}

void PluginView::hide()
{
    LOG(Plugins, "PluginView::hide()");
    
    if (!m_plugin)
        return;
    
    const bool visibleStateChanged = (m_isEnabled && isVisible()/* && m_attachedToWindow*/);
    setSelfVisible(false);
	
	Widget::hide();
    
    if(parent() && visibleStateChanged)
	    invalidate();
}

void PluginView::setFocus(bool focused)
    {
        if (focused) {
#ifndef NP_NO_QUICKDRAW
			// We need to cache the active TSMDocument (which is most likely the application's TSMDocument) in case the plug-in creates a new one when setting focus to a text field, 
			// but does not re-activate the current active TSMDocument when it loses focus (bug# 2633352)
			m_appTSMDocID = TSMGetActiveDocument();
#endif
        
            FrameLoaderClientApollo::clientApollo(m_parentFrame.get())->activateIMEForPlugin();
		}
        else
            FrameLoaderClientApollo::clientApollo(m_parentFrame.get())->deactivateIMEForPlugin();
        
#ifndef NP_NO_QUICKDRAW
        // Focus the plugin
        if (m_plugin) {
            EventRecord carbonEvent;
                carbonEvent.what = focused ? getFocusEvent : loseFocusEvent;
            carbonEvent.message = 0;
            carbonEvent.when = TickCount();
            carbonEvent.where.v = 0;
            carbonEvent.where.h = 0;
            carbonEvent.modifiers = 0;
                JSC::JSLock::DropAllLocks dropLocks(false);

			setCallingPlugin(true);
            m_plugin->pluginFuncs()->event(m_instance, &carbonEvent);
			setCallingPlugin(false);
        }
#endif

		if(m_plugin->quirks().contains(PluginQuirkApolloSWFPlugin) && !focused
#ifndef NP_NO_QUICKDRAW
		   && m_appTSMDocID && m_appTSMDocID != TSMGetActiveDocument()
#endif
		   )
#ifndef NP_NO_QUICKDRAW
			ActivateTSMDocument(m_appTSMDocID);
#endif
        
            Widget::setFocus(focused);
        
            // if focus is lost, we need to unfocus ourself
            ASSERT(m_parentFrame);
            ASSERT(m_parentFrame->document());
            ASSERT(m_element);
            if (!focused && m_parentFrame->document()->focusedNode() == m_element)
                m_parentFrame->document()->setFocusedNode(0);
}

void PluginView::setParentVisible(bool visible)
{
    if (isParentVisible() == visible)
        return;

    Widget::setParentVisible(visible);
}

void PluginView::setNPWindowRect(const IntRect&)
{
#ifdef exists_setNPWindowIfNeeded
    setNPWindowIfNeeded();
#endif
}

#ifdef exists_setNPWindowIfNeeded
void PluginView::setNPWindowIfNeeded()
{
    if (!m_isStarted || !parent() || !m_plugin->pluginFuncs()->setwindow)
        return;

    CGContextRef newContextRef = 0;
    WindowRef newWindowRef = 0;
    if (platformPluginWidget()) {
        newContextRef = cgHandleFor(platformPluginWidget());
        newWindowRef = nativeWindowFor(platformPluginWidget());
        m_npWindow.type = NPWindowTypeWindow;
    } else {
        newContextRef = m_contextRef;
        newWindowRef = m_fakeWindow;
        m_npWindow.type = NPWindowTypeDrawable;
    }

    if (!newContextRef || !newWindowRef)
        return;

    m_npWindow.window = (void*)&m_npCgContext;
#ifndef NP_NO_CARBON
    m_npCgContext.window = newWindowRef;
#endif
    m_npCgContext.context = newContextRef;

    m_npWindow.x = m_windowRect.x();
    m_npWindow.y = m_windowRect.y();
    m_npWindow.width = m_windowRect.width();
    m_npWindow.height = m_windowRect.height();

    // TODO: (also clip against scrollbars, etc.)
    m_npWindow.clipRect.left = max(0, m_windowRect.x());
    m_npWindow.clipRect.top = max(0, m_windowRect.y());
    m_npWindow.clipRect.right = m_windowRect.x() + m_windowRect.width();
    m_npWindow.clipRect.bottom = m_windowRect.y() + m_windowRect.height();

    LOG(Plugins, "PluginView::setNPWindowIfNeeded(): window=%p, context=%p,"
            " window.x:%d window.y:%d window.width:%d window.height:%d window.clipRect size:%dx%d",
            newWindowRef, newContextRef, m_npWindow.x, m_npWindow.y, m_npWindow.width, m_npWindow.height,
            m_npWindow.clipRect.right - m_npWindow.clipRect.left, m_npWindow.clipRect.bottom - m_npWindow.clipRect.top);

    PluginView::setCurrentPluginView(this);
    JSC::JSLock::DropAllLocks dropAllLocks(JSC::SilenceAssertionsOnly);
    setCallingPlugin(true);
    m_plugin->pluginFuncs()->setwindow(m_instance, &m_npWindow);
    setCallingPlugin(false);
    PluginView::setCurrentPluginView(0);
}
#endif // exists_setNPWindowIfNeeded

void PluginView::updatePluginWidget()
{
    if (!parent())
       return;

    ASSERT(parent()->isFrameView());
    FrameView* frameView = static_cast<FrameView*>(parent());

    IntRect oldWindowRect = m_windowRect;
    IntRect oldClipRect = m_clipRect;

    m_windowRect = frameView->contentsToWindow(frameRect());
    IntPoint offset = topLevelOffsetFor(platformPluginWidget());
    m_windowRect.move(offset.x(), offset.y());

    if (!platformPluginWidget()) {
        if (m_windowRect.size() != oldWindowRect.size()) {
#if defined(XP_MACOSX)&& !PLATFORM(APOLLO)
            CGContextRelease(m_contextRef);
#if PLATFORM(QT)
            m_pixmap = QPixmap(m_windowRect.size());
            m_pixmap.fill(Qt::transparent);
            m_contextRef = m_pixmap.isNull() ? 0 : qt_mac_cg_context(&m_pixmap);
#endif
#endif
        }
    }

    m_clipRect = windowClipRect();
    m_clipRect.move(-m_windowRect.x(), -m_windowRect.y());

#ifdef exists_setNPWindowIfNeeded
    if (platformPluginWidget() && (m_windowRect != oldWindowRect || m_clipRect != oldClipRect))
        setNPWindowIfNeeded();
#endif // exists_setNPWindowIfNeeded
}

void PluginView::paint(GraphicsContext* context, const IntRect& rect)
{
    if (!m_isStarted || m_status != PluginStatusLoadedSuccessfully) {
        paintMissingPluginIcon(context, rect);
        return;
    }

    if (context->paintingDisabled())
        return;

#ifdef exists_setNPWindowIfNeeded
    setNPWindowIfNeeded();
#endif

    CGContextRef cgContext = m_npCgContext.context;
    if (!cgContext)
        return;

    CGContextSaveGState(cgContext);
    if (platformPluginWidget()) {
        IntPoint offset = frameRect().location();
        CGContextTranslateCTM(cgContext, offset.x(), offset.y());
    }

    IntRect targetRect(frameRect());
    targetRect.intersects(rect);

    // clip the context so that plugin only updates the interested area.
    CGRect r;
    r.origin.x = targetRect.x() - frameRect().x();
    r.origin.y = targetRect.y() - frameRect().y();
    r.size.width = targetRect.width();
    r.size.height = targetRect.height();
    CGContextClipToRect(cgContext, r);

    if (!platformPluginWidget() && m_isTransparent) { // clean the pixmap in transparent mode
#if PLATFORM(QT)
        QPainter painter(&m_pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(QRectF(r.origin.x, r.origin.y, r.size.width, r.size.height), Qt::transparent);
#endif
    }

#ifndef NP_NO_CARBON
    EventRecord event;
    event.what = updateEvt;
    event.message = (long unsigned int)m_npCgContext.window;
    event.when = TickCount();
    event.where.h = 0;
    event.where.v = 0;
    event.modifiers = GetCurrentKeyModifiers();

    if (!dispatchNPEvent(event))
        LOG(Events, "PluginView::paint(): Paint event not accepted");
#endif

    CGContextRestoreGState(cgContext);

    if (!platformPluginWidget()) {
#if PLATFORM(QT)
        QPainter* painter = context->platformContext();
        painter->drawPixmap(targetRect.x(), targetRect.y(), m_pixmap, 
                            targetRect.x() - frameRect().x(), targetRect.y() - frameRect().y(), targetRect.width(), targetRect.height());
#endif
    }
}

void PluginView::invalidateRect(const IntRect& rect)
{
    if (platformPluginWidget())
#if PLATFORM(QT)
        platformPluginWidget()->update(convertToContainingWindow(rect));
#else
        platformPluginWidget()->RefreshRect(convertToContainingWindow(rect));
#endif
    else
        invalidateWindowlessPluginRect(rect);
}

void PluginView::invalidateRect(NPRect* rect)
{
    IntRect r(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
    invalidateRect(r);
}

void PluginView::invalidateRegion(NPRegion region)
{
    // TODO: optimize
    invalidate();
}

void PluginView::forceRedraw()
{
    notImplemented();
}


// ----------------- Event handling --------------------

void PluginView::handleMouseEvent(MouseEvent* event)
{
    if (!m_isStarted)
        return;

#ifndef NP_NO_CARBON
    EventRecord record;
        record.message = 0;
        record.when = TickCount();

    // some events override the position (below) 
    record.where = mousePosForPlugin(event);
	getCarbonMousePosition(&record.where);

        EventModifiers const btnStateMod = ((event->buttonDown()) && (event->button() == LeftButton)) ? btnState : 0;
		record.modifiers = modifiersForEvent(event) | btnStateMod;
    if (event->type() == eventNames().mousemoveEvent) {
            record.what = nullEvent;
        // Mouse movement is handled by null timer events
        m_lastMousePos = mousePosForPlugin(event);
        return;
    } else if (event->type() == eventNames().mouseoverEvent) {
        record.what = adjustCursorEvent;
    } else if (event->type() == eventNames().mouseoutEvent) {
        record.what = adjustCursorEvent;
    } else if (event->type() == eventNames().mousedownEvent) {
            record.what = mouseDown;
            ASSERT(event->buttonDown());
            switch (event->button()) {
                case LeftButton:
                    break;
                case RightButton:
					// on right click we need to send global coordinates
					GetGlobalMouse(&record.where);
                    // this is what the netscape PI is expecting
                    record.modifiers |= controlKey;
                    break;
                case MiddleButton:
                default:
        return;
                    break;
            }
            ASSERT(m_parentFrame);
            ASSERT(m_parentFrame->page());
            ASSERT(m_parentFrame->page()->focusController());
            ASSERT(m_element);
            // We need to make sure we get the focus when we get a mouse down.
            // This does not happen by default because we tell webcore that the default
            // has been handled down below ( because the player told us it handled the event ).
			
            // Focus the plugin
        if (Page* page = m_parentFrame->page())
            page->focusController()->setFocusedFrame(m_parentFrame);
        m_parentFrame->document()->setFocusedNode(m_element);
            ASSERT(m_parentFrame->selection());
            // Yuck! We need to kick the selection controller in the pants
            // if the plugin is the only thing on the doc that can be selected.
            // This is due to the early out
            // in FocusController::clearSelectionIfNeeded(Frame* oldFocusedFrame, Frame* newFocusedFrame, Node* newFocusedNode).
            // the line in question looks like this:
            // if (selectionStartNode == newFocusedNode || selectionStartNode->isDescendantOf(newFocusedNode) || selectionStartNode->shadowAncestorNode() == newFocusedNode)
            //     return;
            // If we are the only selectable node in the doc, then selectionStartNode will == newFocusedNode.
            // If the code in FocusController::clearSelectionIfNeeded changes we might be able to remove the hack below.
            // Rather than determine if this plugin is the only selectable content
            // in the doc, we'll just always clear the selection when we mouse down
            // on a plugin.
            m_parentFrame->selection()->clear();
    } else if (event->type() == eventNames().mouseupEvent) {
            record.what = mouseUp;
            ASSERT(event->buttonDown());
            switch (event->button()) {
                case LeftButton:
                    break;
                case RightButton:
                    // this is what the netscape PI is expecting
                    record.modifiers |= controlKey;
                    break;
                case MiddleButton:
                default:
                    return;
                    break;
            }
    } else {
        return;
    }

        startUserGesture();

        const NPPluginFuncs* const pluginFuncs = m_plugin->pluginFuncs();
        JSC::JSLock::DropAllLocks dropLocks(false);
    setCallingPlugin(true);
        bool const eventHandledByPlugin = pluginFuncs->event(m_instance, &record);
    setCallingPlugin(false);
        if (eventHandledByPlugin)
            event->setDefaultHandled();
			
/*
    if (!event->buttonDown())
        record.modifiers |= btnState;

    if (event->button() == 2)
        record.modifiers |= controlKey;

    if (!dispatchNPEvent(record)) {
        if (record.what == adjustCursorEvent)
            return; // Signals that the plugin wants a normal cursor

        LOG(Events, "PluginView::handleMouseEvent(): Mouse event type %d at %d,%d not accepted",
                record.what, record.where.h, record.where.v);
    } else {
        event->setDefaultHandled();
    }
*/
#endif
}

void PluginView::handleKeyboardEvent(KeyboardEvent* event)
{
    if (!m_isStarted)
        return;

    LOG(Plugins, "PluginView::handleKeyboardEvent() ----------------- ");

    LOG(Plugins, "PV::hKE(): KE.keyCode: 0x%02X, KE.charCode: %d",
            event->keyCode(), event->charCode());

#ifndef NP_NO_CARBON
    EventRecord record;

    if (event->type() == eventNames().keydownEvent) {
        // This event is the result of a PlatformKeyboardEvent::KeyDown which
        // was disambiguated into a PlatformKeyboardEvent::RawKeyDown. Since
        // we don't have access to the text here, we return, and wait for the
        // corresponding event based on PlatformKeyboardEvent::Char.
        return;
    } else if (event->type() == eventNames().keypressEvent) {
        // Which would be this one. This event was disambiguated from the same
        // PlatformKeyboardEvent::KeyDown, but to a PlatformKeyboardEvent::Char,
        // which retains the text from the original event. So, we can safely pass
        // on the event as a key-down event to the plugin.
        record.what = keyDown;
    } else if (event->type() == eventNames().keyupEvent) {
        // PlatformKeyboardEvent::KeyUp events always have the text, so nothing
        // fancy here.
        record.what = keyUp;
    } else {
        return;
    }

    const PlatformKeyboardEvent* platformEvent = event->keyEvent();
    int keyCode = platformEvent->nativeVirtualKeyCode();

    const String text = platformEvent->text();
    if (text.length() < 1) {
        event->setDefaultHandled();
        return;
    }

    WTF::RetainPtr<CFStringRef> cfText(WTF::AdoptCF, text.createCFString());

    LOG(Plugins, "PV::hKE(): PKE.text: %s, PKE.unmodifiedText: %s, PKE.keyIdentifier: %s",
            text.ascii().data(), platformEvent->unmodifiedText().ascii().data(),
            platformEvent->keyIdentifier().ascii().data());

    char charCodes[2] = { 0, 0 };
    if (!CFStringGetCString(cfText.get(), charCodes, 2, CFStringGetSystemEncoding())) {
        LOG_ERROR("Could not resolve character code using system encoding.");
        event->setDefaultHandled();
        return;
    }

    record.where = globalMousePosForPlugin();
    record.modifiers = modifiersForEvent(event);
    record.message = ((keyCode & 0xFF) << 8) | (charCodes[0] & 0xFF);
    record.when = TickCount();

    LOG(Plugins, "PV::hKE(): record.modifiers: %d", record.modifiers);

#if PLATFORM(QT)
    LOG(Plugins, "PV::hKE(): PKE.qtEvent()->nativeVirtualKey: 0x%02X, charCode: %d",
               keyCode, int(uchar(charCodes[0])));
#endif

    if (!dispatchNPEvent(record))
        LOG(Events, "PluginView::handleKeyboardEvent(): Keyboard event type %d not accepted", record.what);
    else
        event->setDefaultHandled();
#endif
}

void PluginView::updatePluginWindow(bool canShowPlugins, bool /*canShowWindowedPlugins*/)
{
	if (!m_plugin)
       return;

	const bool suppressionStateChanged = (m_isEnabled != canShowPlugins);
	m_isEnabled = canShowPlugins;
        
	bool const bCanShowPlugins = m_isEnabled && isVisible() /*&& m_attachedToWindow*/;
	if (!bCanShowPlugins) {
		m_nullEventTimer.stop();
	}
	else if (!(m_nullEventTimer.isActive())) {
		m_nullEventTimer.start(0, static_cast<double>(activeNullTimerPeriodInMilliSeconds) / static_cast<double>(1000));
	}
        
	// Note: this will result in QuickDraw being slower than CoreGraphics however this is how it worked up to 1.5.1
	// We need to research how to do on demand painting for QuickDraw.  See bug# 2660033
	if (parent() && ((m_drawingModel == NPDrawingModelQuickDraw) || suppressionStateChanged))
		invalidate();
}

#ifndef NP_NO_CARBON
void PluginView::nullEventTimerFired(Timer<PluginView>*)
{
    EventRecord record;

    record.what = nullEvent;
    record.message = 0;
    record.when = TickCount();
    record.where = m_lastMousePos;
    record.modifiers = GetCurrentKeyModifiers();
    if (!Button())
        record.modifiers |= btnState;

    if (!dispatchNPEvent(record))
        LOG(Events, "PluginView::nullEventTimerFired(): Null event not accepted");
}
#endif

#ifndef NP_NO_CARBON
static int modifiersForEvent(UIEventWithKeyState* event)
{
    int modifiers = 0;

    if (event->ctrlKey())
        modifiers |= controlKey;

    if (event->altKey())
        modifiers |= optionKey;

    if (event->metaKey())
        modifiers |= cmdKey;

    if (event->shiftKey())
        modifiers |= shiftKey;

     return modifiers;
}
#endif

#ifndef NP_NO_CARBON
static bool tigerOrBetter()
{
    static SInt32 systemVersion = 0;

    if (!systemVersion) {
        if (Gestalt(gestaltSystemVersion, &systemVersion) != noErr)
            return false;
    }

    return systemVersion >= 0x1040;
}
#endif

#ifndef NP_NO_CARBON
Point PluginView::globalMousePosForPlugin() const
{
    Point pos;
    GetGlobalMouse(&pos);

    float scaleFactor = tigerOrBetter() ? HIGetScaleFactor() : 1;

    pos.h = short(pos.h * scaleFactor);
    pos.v = short(pos.v * scaleFactor);

#if PLATFORM(WX)
    // make sure the titlebar/toolbar size is included
    WindowRef windowRef = nativeWindowFor(platformPluginWidget());
    ::Rect content, structure;

    GetWindowBounds(windowRef, kWindowStructureRgn, &structure);
    GetWindowBounds(windowRef, kWindowContentRgn, &content);

    int top = content.top  - structure.top;
    pos.v -= top;
#endif

    return pos;
}
#endif

#ifndef NP_NO_CARBON
Point PluginView::mousePosForPlugin(MouseEvent* event) const
{
    ASSERT(event);
    if (platformPluginWidget())
        return globalMousePosForPlugin();

    if (event->button() == 2) {
        // always pass the global position for right-click since Flash uses it to position the context menu
        return globalMousePosForPlugin();
    }

    Point pos;
    IntPoint postZoomPos = roundedIntPoint(m_element->renderer()->absoluteToLocal(event->absoluteLocation()));
    pos.h = postZoomPos.x() + m_windowRect.x();
    // The number 22 is the height of the title bar. As to why it figures in the calculation below
    // is left as an exercise to the reader :-)
    pos.v = postZoomPos.y() + m_windowRect.y() - 22;
    return pos;
}
#endif

#ifndef NP_NO_CARBON
bool PluginView::dispatchNPEvent(NPEvent& event)
{
    PluginView::setCurrentPluginView(this);
    JSC::JSLock::DropAllLocks dropAllLocks(JSC::SilenceAssertionsOnly);
    setCallingPlugin(true);

    bool accepted = m_plugin->pluginFuncs()->event(m_instance, &event);

    setCallingPlugin(false);
    PluginView::setCurrentPluginView(0);
    return accepted;
}
#endif

// ------------------- Miscellaneous  ------------------

NPError PluginView::handlePostReadFile(Vector<char>& buffer, uint32_t len, const char* buf)
{
    String filename(buf, len);

    if (filename.startsWith("file:///"))
        filename = filename.substring(8);

    if (!fileExists(filename))
        return NPERR_FILE_NOT_FOUND;

    FILE* fileHandle = fopen((filename.utf8()).data(), "r");

    if (fileHandle == 0)
        return NPERR_FILE_NOT_FOUND;

    int bytesRead = fread(buffer.data(), 1, 0, fileHandle);

    fclose(fileHandle);

    if (bytesRead <= 0)
        return NPERR_FILE_NOT_FOUND;

    return NPERR_NO_ERROR;
}

void PluginView::halt()
{
}

void PluginView::restart()
{
}

void PluginView::getCarbonMousePosition(::Point* const mousePoint)
{
    ASSERT(mousePoint);
    GetGlobalMouse(mousePoint);
    if (m_drawingModel == NPDrawingModelQuickDraw) {
        ASSERT(m_npWindow.window == &m_npPort);
        ASSERT(m_npPort.port);
        HIPoint hiMousePoint = { mousePoint->h, mousePoint->v };
        WindowRef const carbonWindow = getCarbonWindow();
        HIPointConvert(&hiMousePoint
                      , kHICoordSpaceScreenPixel  
                      , 0
                      , kHICoordSpaceWindow
                      , carbonWindow);
        IntPoint const htmlControlOriginInCarbonWindow(getOriginOfHTMLControlInCarbonWindow());
						
        mousePoint->h = static_cast<int>(hiMousePoint.x) - htmlControlOriginInCarbonWindow.x();
        mousePoint->v = static_cast<int>(hiMousePoint.y) - htmlControlOriginInCarbonWindow.y();
    }
}
//#endif // NP_NO_QUICKDRAW 

// similar to getGeometryInCarbonWindow
IntRect PluginView::clipDirtyRect(const IntRect& dirtyRect) const
{
    // Take our element and get the clip rect from the enclosing layer and frame view.
        IntRect result = dirtyRect;
        result.intersect(frameRect());
    
        RenderLayer* const layer = m_element->renderer()->enclosingLayer();
        ASSERT(layer);
        IntRect layerClipRect(layer->childrenClipRect());
        result.intersect(layerClipRect);
    

#ifndef NP_NO_QUICKDRAW
        if(m_drawingModel == NPDrawingModelQuickDraw)
        {
            result = static_cast<FrameView*>(parent())->contentsToWindow(result);
            if(result.x() < 0) {
                result.setWidth(result.width() + result.x());
                result.setX(0);
            }
            if(result.y() < 0) {
                result.setHeight(result.height() + result.y());
                result.setY(0);
            }
}
        else
#endif
{
        IntPoint pluginLocation = frameRect().location();
        result.move(-pluginLocation.x(), -pluginLocation.y());
        }
        return result;
}

void PluginView::paint(GraphicsContext* context, const IntRect& rect)
{
    if (!m_isEnabled || !isSelfVisible())
        return;
    
        IntRect const geometry(frameRect());
    context->save();
    context->clip(geometry);
    
    if (!m_isStarted) {
        // Draw the "missing plugin" image
        paintMissingPluginIcon(context, rect);
    }
    else {
#ifndef NP_NO_QUICKDRAW
            setNPWindowRect(frameRect());
#endif
        
            // This should really move to use m_clipRect, after calling updatePluginWidget()
            // It takes care of clipping to the window for windowless plug-ins.
        IntRect clipRect = clipDirtyRect(rect);
        
        if (m_drawingModel == NPDrawingModelCoreGraphics) {
            CGRect cgClipRect = CGRectMake(clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height());
            CGContextSaveGState(m_npCGContext.context);
            CGContextClipToRect(m_npCGContext.context, cgClipRect);
        }
#ifndef NP_NO_QUICKDRAW
        else  {
            ASSERT(m_drawingModel == NPDrawingModelQuickDraw);
            CGrafPtrPusherPopper const savePort(m_npPort.port);
            
            RgnHandle rgn = QDDeprecated::NewRgn();
            QDDeprecated::SetRectRgn(rgn, clipRect.x(), clipRect.y(), clipRect.right(), clipRect.bottom());
            QDDeprecated::SetClip (rgn);
            QDDeprecated::DisposeRgn(rgn);
        }
        
        EventRecord carbonEvent;
        carbonEvent.what = updateEvt;
        if (m_drawingModel == NPDrawingModelCoreGraphics) {
            ASSERT(getCarbonWindow() == m_npCGContext.window);
            carbonEvent.message = reinterpret_cast<unsigned long>(m_npCGContext.window);
        } 
        else {
            ASSERT(m_drawingModel == NPDrawingModelQuickDraw);
            carbonEvent.message = reinterpret_cast<unsigned long>(getCarbonWindow());
        }
            
        carbonEvent.when = TickCount();
        carbonEvent.where.v = 0;
        carbonEvent.where.h = 0;
        carbonEvent.modifiers = 0;
        
            JSC::JSLock::DropAllLocks dropLocks(false);
        setCallingPlugin(true);
        m_plugin->pluginFuncs()->event(m_instance, &carbonEvent);
        setCallingPlugin(false);
#endif
        if (m_drawingModel == NPDrawingModelCoreGraphics) {
            CGContextRestoreGState(m_npCGContext.context);
#ifndef NDEBUG
                const IntPoint p = viewportToWindow(p);
            IntRect clipRectInLoader(clipRect);
            clipRectInLoader.move(p.x(), p.y());
            ASSERT(clipRectInLoader.x() >= 0);
                ASSERT(clipRectInLoader.right() <= p.x() + static_cast<int>(m_webBitmapWidth));
            ASSERT(clipRectInLoader.y() >= 0);
                ASSERT(clipRectInLoader.bottom() <= p.y() + static_cast<int>(m_webBitmapHeight));
#endif
        }
#ifndef NP_NO_QUICKDRAW
        else {
            ASSERT(m_drawingModel == NPDrawingModelQuickDraw);
            // QuickDraw does not understand alpha, so we just saturate the alpha channel
            // after we draw.  The FlashPlayer plugin draws then entire bitmape everytime, so
            // I whack all the pixels (not just the update rect).
                IntPoint p = m_parentFrame->view()->contentsToWindow(frameRect().location());
                //p = viewportToWindow(p);
            
			IntRect clipRectInLoader(clipRect);
                //clipRectInLoader.move(p.x(), p.y());
			ASSERT(clipRectInLoader.x() >= 0);
            ASSERT(clipRectInLoader.right() <= static_cast<int>(m_webBitmapWidth));
            ASSERT(clipRectInLoader.y() >= 0);
            ASSERT(clipRectInLoader.bottom() <= static_cast<int>(m_webBitmapHeight));
            unsigned long const rowOffset = clipRectInLoader.x() * sizeof(uint32_t);
			unsigned char* const pixelBytesForPlugin = reinterpret_cast<unsigned char*>(m_webBitmapPixels) + (clipRectInLoader.y() * m_webBitmapStride) + rowOffset;
			unsigned long numRowsInPlugin = clipRectInLoader.height();
			unsigned long numColsInPlugin = clipRectInLoader.width();
			unsigned char* currRowFirstByte = pixelBytesForPlugin;
			for (unsigned long i = 0; i < numRowsInPlugin; ++i) {
				uint32_t* currPixel = reinterpret_cast<uint32_t*>(currRowFirstByte);
				for (unsigned long j = 0; j < numColsInPlugin; ++j) {
					(*currPixel) |= 0xFF000000;
					++currPixel;
				}
				currRowFirstByte += m_webBitmapStride;
			}
		}
#endif
    }
    
    context->restore();
}

#ifndef NP_NO_QUICKDRAW
    void PluginView::nullEventTimerFired(Timer<PluginView>* /*timer*/)
{
    // If we are already in the plugin, then don't re-enter it.
    if (isCallingPlugin())
        return;
        
        // nullEvent may fire before setNPWindowRect is called to create the port
        if((m_drawingModel == NPDrawingModelQuickDraw) && !m_npPort.port)
            return;
        
    EventRecord carbonEvent;
    carbonEvent.what = nullEvent;
    carbonEvent.message = 0;
    carbonEvent.when = TickCount();
    
		// bug# 2387914 - flash player expects to get the current mouse position on null events
		getCarbonMousePosition(&carbonEvent.where);
    
    ASSERT(m_parentFrame);
    Page* const page = m_parentFrame->page();
    ASSERT(page);
    FocusController* const focusController = page->focusController();
    ASSERT(focusController);
    bool const pageHasFocus = focusController->isActive();
        if (pageHasFocus && isSelfVisible()) {
        //Get the carbon window that really contains us.
        WindowRef const carbonWindowRef = getCarbonWindow();
        if (carbonWindowRef)
            getCarbonMousePosition(&carbonEvent.where);
    }
    carbonEvent.modifiers = GetCurrentKeyModifiers();
    if (!Button())
        carbonEvent.modifiers |= btnState;
    
    // Scope dropLocks to just the call to pluginFuncs()->event(...)
    {
            JSC::JSLock::DropAllLocks dropLocks(false);
        setCallingPlugin(true);
        m_plugin->pluginFuncs()->event(m_instance, &carbonEvent);
        setCallingPlugin(false);
    }
}

namespace {
    struct WinKeyCodeToMacKeyCodeEntry {
        char const macKeyCode;
    };
    
    WinKeyCodeToMacKeyCodeEntry const windowKeyCodeToMacKeyCode[] = {
          { 0x0a } // 0
        , { 0x7F } // No mac key for windows key 1.
        , { 0x7F } // No mac key for windows key 2.
        , { 0x7F } // No mac key for windows key 3.
        , { 0x7F } // No mac key for windows key 4.
        , { 0x7F } // No mac key for windows key 5.
        , { 0x7F } // No mac key for windows key 6.
        , { 0x7F } // No mac key for windows key 7.
        , { 0x33 } // 8
        , { 0x30 } // 9
        , { 0x7F } // No mac key for windows key 10.
        , { 0x4b } // 11
        , { 0x7F } // No mac key for windows key 12.
        , { 0x24 } // 13
        , { 0x7F } // No mac key for windows key 14.
        , { 0x37 } // 15
        , { 0x38 } // 16
        , { 0x3b } // 17
        , { 0x3a } // 18
        , { 0x7F } // No mac key for windows key 19.
        , { 0x39 } // 20
        , { 0x7F } // No mac key for windows key 21.
        , { 0x7F } // No mac key for windows key 22.
        , { 0x7F } // No mac key for windows key 23.
        , { 0x7F } // No mac key for windows key 24.
        , { 0x7F } // No mac key for windows key 25.
        , { 0x7F } // No mac key for windows key 26.
        , { 0x35 } // 27
        , { 0x7F } // No mac key for windows key 28.
        , { 0x7F } // No mac key for windows key 29.
        , { 0x7F } // No mac key for windows key 30.
        , { 0x7F } // No mac key for windows key 31.
        , { 0x31 } // 32
        , { 0x74 } // 33
        , { 0x79 } // 34
        , { 0x77 } // 35
        , { 0x73 } // 36
        , { 0x7b } // 37
        , { 0x7e } // 38
        , { 0x7c } // 39
        , { 0x7d } // 40
        , { 0x7F } // No mac key for windows key 41.
        , { 0x7F } // No mac key for windows key 42.
        , { 0x7F } // No mac key for windows key 43.
        , { 0x7F } // No mac key for windows key 44.
        , { 0x72 } // 45
        , { 0x75 } // 46
        , { 0x7F } // No mac key for windows key 47.
        , { 0x1d } // 48
        , { 0x12 } // 49
        , { 0x13 } // 50
        , { 0x14 } // 51
        , { 0x15 } // 52
        , { 0x17 } // 53
        , { 0x16 } // 54
        , { 0x1a } // 55
        , { 0x1c } // 56
        , { 0x19 } // 57
        , { 0x7F } // No mac key for windows key 58.
        , { 0x7F } // No mac key for windows key 59.
        , { 0x7F } // No mac key for windows key 60.
        , { 0x7F } // No mac key for windows key 61.
        , { 0x7F } // No mac key for windows key 62.
        , { 0x7F } // No mac key for windows key 63.
        , { 0x7F } // No mac key for windows key 64.
        , { 0x00 } // 65
        , { 0x0b } // 66
        , { 0x08 } // 67
        , { 0x02 } // 68
        , { 0x0e } // 69
        , { 0x03 } // 70
        , { 0x05 } // 71
        , { 0x04 } // 72
        , { 0x22 } // 73
        , { 0x26 } // 74
        , { 0x28 } // 75
        , { 0x25 } // 76
        , { 0x2e } // 77
        , { 0x2d } // 78
        , { 0x1f } // 79
        , { 0x23 } // 80
        , { 0x0c } // 81
        , { 0x0f } // 82
        , { 0x01 } // 83
        , { 0x11 } // 84
        , { 0x20 } // 85
        , { 0x09 } // 86
        , { 0x0d } // 87
        , { 0x07 } // 88
        , { 0x10 } // 89
        , { 0x06 } // 90
        , { 0x7F } // No mac key for windows key 91.
        , { 0x7F } // No mac key for windows key 92.
        , { 0x7F } // No mac key for windows key 93.
        , { 0x7F } // No mac key for windows key 94.
        , { 0x7F } // No mac key for windows key 95.
        , { 0x52 } // 96
        , { 0x53 } // 97
        , { 0x54 } // 98
        , { 0x55 } // 99
        , { 0x56 } // 100
        , { 0x57 } // 101
        , { 0x58 } // 102
        , { 0x59 } // 103
        , { 0x5b } // 104
        , { 0x5c } // 105
        , { 0x43 } // 106
        , { 0x45 } // 107
        , { 0x7F } // No mac key for windows key 108.
        , { 0x4e } // 109
        , { 0x41 } // 110
        , { 0x4b } // 111
        , { 0x7a } // 112
        , { 0x78 } // 113
        , { 0x63 } // 114
        , { 0x76 } // 115
        , { 0x60 } // 116
        , { 0x61 } // 117
        , { 0x62 } // 118
        , { 0x64 } // 119
        , { 0x65 } // 120
        , { 0x6d } // 121
        , { 0x67 } // 122
        , { 0x6f } // 123
        , { 0x69 } // 124
        , { 0x6b } // 125
        , { 0x71 } // 126
        , { 0x7F } // No mac key for windows key 127.
    };
    
    static const size_t numWindowKeyCodeToMacKeyCodeEntries = sizeof(windowKeyCodeToMacKeyCode) / sizeof(WinKeyCodeToMacKeyCodeEntry);
    
        const int CHARMAP_ENTRIES = 128;
        
        static const unsigned char kWin2MacTable[CHARMAP_ENTRIES] = {
        222, 223, 226, 196, 227, 201, 160, 
        224, 246, 228, 186, 220, 206, 173, 
        179, 178, 176, 212, 213, 210, 211, 
        165, 248, 209, 247, 170, 249, 221, 
        207, 240, 218, 217, 202, 193, 162, 
        163, 219, 180, 245, 164, 172, 169, 
        187, 199, 194, 208, 168, 195, 161, 
        177, 250, 254, 171, 181, 166, 225, 
        252, 255, 188, 200, 197, 253, 251, 
        192, 203, 231, 229, 204, 128, 129, 
        174, 130, 233, 131, 230, 232, 237, 
        234, 235, 236, 198, 132, 241, 238, 
        239, 205, 133, 215, 175, 244, 242, 
        243, 134, 183, 184, 167, 136, 135, 
        137, 139, 138, 140, 190, 141, 143, 
        142, 144, 145, 147, 146, 148, 149, 
        182, 150, 152, 151, 153, 155, 154, 
        214, 191, 157, 156, 158, 159, 189, 
        185, 216 
        };
        
}

    // Check if the specified script represent an accepted language script.
    // For now, support Western Languages, Central European Languages and Cyrillic.
    // CJK text is routed directly to plugin.
    // Other languages - greek, hebrew and arabic - we have to figure our how to send them correctly to FP.
    // Turkish is in script smUnicodeScript and the conversion using the below functions fails.
    // In the future, we have to upgrade the code for conversion since most of the below functions are deprecated.
    static bool sAcceptedScriptCode(const ScriptCode script)
{
        if ((smRoman == script)            ||
            (smCyrillic == script)         ||
            (smCentralEuroRoman == script)) 
            return true;
        return false;
    }
    
    
    bool PluginView::handleInsertText(const String& text)
    {
    ScriptCode script = (ScriptCode) GetScriptManagerVariable(smKeyScript);
        if (!sAcceptedScriptCode(script))
            return false;
        
        ::TextEncoding encoding = kCFStringEncodingUTF8;
        
        OSStatus result;
        if (script != smUnicodeScript) {
            result = UpgradeScriptInfoToTextEncoding(script, kTextLanguageDontCare, kTextRegionDontCare, NULL, &encoding);
            if (result != noErr) {
                return false;
            }
        }
    
        RetainPtr<CFStringRef> str(AdoptCF, text.createCFString());
        CFIndex bufferSize;
        CFIndex length;
        length = CFStringGetBytes(str.get(), CFRangeMake(0, CFStringGetLength(str.get())), encoding, '?', false, NULL, 0, &bufferSize);
        if (length <= 0) {
            return false;
        }
        
        Vector<UInt8> buffer(bufferSize);
        length = CFStringGetBytes(str.get(), CFRangeMake(0, CFStringGetLength(str.get())), encoding, '?', false, buffer.data(), bufferSize, NULL);
        
        for (CFIndex i=0; i<bufferSize; i++) {
            EventRecord carbonEvent;
            carbonEvent.message = buffer[i];
            carbonEvent.when = TickCount();
            carbonEvent.where.v = 0;
            carbonEvent.where.h = 0;
            carbonEvent.modifiers = 0;
            carbonEvent.what = keyDown;
            
            startUserGesture();
            
            JSC::JSLock::DropAllLocks dropLocks(false);
            setCallingPlugin(true);
            m_plugin->pluginFuncs()->event(m_instance, &carbonEvent);
            
            // on insertText, we don't get keyDown or keyUp events
            carbonEvent.when = TickCount();
            carbonEvent.what = keyUp;
            m_plugin->pluginFuncs()->event(m_instance, &carbonEvent);
            
            setCallingPlugin(false);
        }
        
        return true;
}

    // Convert the first char from the specified text. Used only for chars < 0xff.
    // This function shows the right way to convert, even if it uses deprecated functions.
    static unsigned char convertCharCode(const String& text)
    {
        ScriptCode script = (ScriptCode) GetScriptManagerVariable(smKeyScript);
        
    ::TextEncoding encoding;
    OSStatus result = UpgradeScriptInfoToTextEncoding(script, kTextLanguageDontCare, kTextRegionDontCare, NULL, &encoding);
    if (result != noErr)
            return 0;
    
    RetainPtr<CFStringRef> str(AdoptCF, text.createCFString());
    CFRange range = CFRangeMake(0, CFStringGetLength(str.get()));
    
    CFIndex bufferSize;
    CFIndex length = CFStringGetBytes(str.get(), CFRangeMake(0, CFStringGetLength(str.get())), encoding, '?', false, NULL, 0, &bufferSize);    
    if (length <= 0)
            return 0; //conversion failed
    
    Vector<UInt8> buffer(bufferSize);
    length = CFStringGetBytes(str.get(), CFRangeMake(0, CFStringGetLength(str.get())), encoding, '?', false, buffer.data(), bufferSize, NULL);
        
    ASSERT(bufferSize == 1);
        return buffer[0];
}    
   
    
void PluginView::handleKeyboardEvent(KeyboardEvent* kbEvent)
{
    EventRecord carbonEvent;
    
    const PlatformKeyboardEvent* keyEvent = kbEvent->keyEvent();
    PlatformKeyboardEvent::Type type = keyEvent->type();
    
    // punt on keypress events for now, when we handle them revise TODO internationalization section below
    if( type !=  PlatformKeyboardEvent::KeyUp && type !=  PlatformKeyboardEvent::RawKeyDown)
        return;
    
    String const keyEventText = keyEvent->text();
    int const charCode(keyEventText.isEmpty() ? 0 : keyEventText[0] );
    
    // treat only ascii chars
    if (charCode > 0xff)
        return;

        int windowsKeyCode = keyEvent->windowsVirtualKeyCode();
        if (windowsKeyCode >= static_cast<int>(numWindowKeyCodeToMacKeyCodeEntries))
            windowsKeyCode = 0x7F;
        
        ASSERT(windowsKeyCode < static_cast<int>(numWindowKeyCodeToMacKeyCodeEntries));
        unsigned const macKeyCode = windowKeyCodeToMacKeyCode[windowsKeyCode].macKeyCode;
    
    carbonEvent.message = (macKeyCode << 8) | ((charCode <= 0x7f) ? charCode : convertCharCode(keyEventText));
    ASSERT(macKeyCode == (carbonEvent.message >> 8));
 
        carbonEvent.when = TickCount();
			getCarbonMousePosition(&carbonEvent.where);
        
        // shiftKeyBit, controlKeyBit, optionKeyBit, and cmdKeyBit are defined
        // in /System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/Events.h
        UInt16 const shiftModifier = static_cast<UInt16>(kbEvent->shiftKey()) << shiftKeyBit;
        UInt16 const ctrlModified = static_cast<UInt16>(kbEvent->ctrlKey()) << controlKeyBit;
        UInt16 const altModifier = static_cast<UInt16>(kbEvent->altKey()) << optionKeyBit;
        UInt16 const metaModifier = static_cast<UInt16>(kbEvent->metaKey()) << cmdKeyBit;
        
        carbonEvent.modifiers = shiftModifier | ctrlModified | altModifier | metaModifier;

        carbonEvent.what = (type == PlatformKeyboardEvent::KeyUp) ? keyUp : keyDown;
        
        startUserGesture();
        
            JSC::JSLock::DropAllLocks dropLocks(false);
        setCallingPlugin(true);
        bool result = m_plugin->pluginFuncs()->event(m_instance, &carbonEvent);
        setCallingPlugin(false);
        
        if(result)
            kbEvent->setDefaultHandled();
    }
#endif // !NO_NO_QUICKDRAW

#ifndef NP_NO_QUICKDRAW
void PluginView::handleMouseEvent(MouseEvent* event)
{
    EventRecord carbonEvent;
    carbonEvent.message = 0;
    carbonEvent.when = TickCount();

    getCarbonMousePosition(&carbonEvent.where);

    EventModifiers const ctrlKeyMod = event->ctrlKey() ? controlKey : 0;
    EventModifiers const shiftKeyMod = event->shiftKey() ? shiftKey : 0;
    EventModifiers const cmdKeyMod = event->metaKey() ? cmdKey : 0;
    EventModifiers const optionKeyMod = event->altKey() ? optionKey : 0;
    EventModifiers const btnStateMod = ((event->buttonDown()) && (event->button() == LeftButton)) ? btnState : 0;
    carbonEvent.modifiers = ctrlKeyMod | shiftKeyMod | cmdKeyMod | optionKeyMod | btnStateMod;
    
        if (event->type() == eventNames().mousemoveEvent) {
        carbonEvent.what = nullEvent;
    }
        else if (event->type() == eventNames().mousedownEvent) {
        carbonEvent.what = mouseDown;
        ASSERT(event->buttonDown());
        switch (event->button()) {
            case LeftButton:
                break;
            case RightButton:
					// on right click we need to send global coordinates
					GetGlobalMouse(&carbonEvent.where);
					
                // this is what the netscape PI is expecting
                carbonEvent.modifiers |= controlKey;
                break;
            case MiddleButton:
            default:
                return;
                break;
        }
			
        ASSERT(m_parentFrame);
        ASSERT(m_parentFrame->page());
        ASSERT(m_parentFrame->page()->focusController());
        ASSERT(m_element);
        // We need to make sure we get the focus when we get a mouse down.
        // This does not happen by default because we tell webcore that the default
        // has been handled down below ( because the player told us it handled the event ).
        // Focus the plugin
        if (Page* page = m_parentFrame->page())
            page->focusController()->setFocusedFrame(m_parentFrame);
        m_parentFrame->document()->setFocusedNode(m_element);
        
            ASSERT(m_parentFrame->selection());
        
        // Yuck! We need to kick the selection controller in the pants
        // if the plugin is the only thing on the doc that can be selected.
        // This is due to the early out
        // in FocusController::clearSelectionIfNeeded(Frame* oldFocusedFrame, Frame* newFocusedFrame, Node* newFocusedNode).
        // the line in question looks like this:
        // if (selectionStartNode == newFocusedNode || selectionStartNode->isDescendantOf(newFocusedNode) || selectionStartNode->shadowAncestorNode() == newFocusedNode)
        //     return;
        // If we are the only selectable node in the doc, then selectionStartNode will == newFocusedNode.
        // If the code in FocusController::clearSelectionIfNeeded changes we might be able to remove the hack below.
        // Rather than determine if this plugin is the only selectable content
        // in the doc, we'll just always clear the selection when we mouse down
        // on a plugin.
            m_parentFrame->selection()->clear();
        
        } else if (event->type() == eventNames().mouseupEvent) {
        carbonEvent.what = mouseUp;
        ASSERT(event->buttonDown());
        switch (event->button()) {
            case LeftButton:
                break;
            case RightButton:
                // this is what the netscape PI is expecting
                carbonEvent.modifiers |= controlKey;
                break;
            case MiddleButton:
            default:
                return;
                break;
        }
    } else 
        return;
    
    startUserGesture();
    
    const NPPluginFuncs* const pluginFuncs = m_plugin->pluginFuncs();
        JSC::JSLock::DropAllLocks dropLocks(false);
    setCallingPlugin(true);
    bool const eventHandledByPlugin = pluginFuncs->event(m_instance, &carbonEvent);
    setCallingPlugin(false);
    if (eventHandledByPlugin)
        event->setDefaultHandled();
}
#endif

void PluginView::updatePluginWindow(bool canShowPlugins, bool /*canShowWindowedPlugins*/)
{
	if (!m_plugin)
		return;
    
	const bool suppressionStateChanged = (m_isEnabled != canShowPlugins);
	m_isEnabled = canShowPlugins;
    
#ifndef NP_NO_QUICKDRAW
	bool const bCanShowPlugins = m_isEnabled && isVisible() /*&& m_attachedToWindow*/;
	if (!bCanShowPlugins) {
		m_nullEventTimer.stop();
	}
	else if (!(m_nullEventTimer.isActive())) {
		m_nullEventTimer.start(0, static_cast<double>(activeNullTimerPeriodInMilliSeconds) / static_cast<double>(1000));
	}
#endif
    
	// Note: this will result in QuickDraw being slower than CoreGraphics however this is how it worked up to 1.5.1
	// We need to research how to do on demand painting for QuickDraw.  See bug# 2660033
	if (parent() && (
#ifndef NP_NO_QUICKDRAW
		 (m_drawingModel == NPDrawingModelQuickDraw) || 
#endif
		 suppressionStateChanged))
		invalidate();
}

IntPoint PluginView::viewportToWindow(const IntPoint &pIn) const
{
        return convertToApolloWindowPoint(pIn);
}
    
    void PluginView::setParentVisible(bool visible)
{
        if (isParentVisible() == visible)
        return;
    
        Widget::setParentVisible(visible);
}

#ifndef NP_NO_QUICKDRAW
void PluginView::setNPWindowRect(const IntRect& rect)
{
        if(rect != frameRect())
    {
        ASSERT(0); // not supported yet
    }
    
        if ((!m_isStarted) || (!m_isEnabled) || (!isVisible()) /*|| (!m_attachedToWindow)*/ || (!isSelfVisible()))
        return;
    
        updatePluginWidget();
        
    bool dirty = false;
    
    IntRect const geometryInCarbonWindow(getGeometryInCarbonWindow());
    IntPoint const locationInCarbonWindow(geometryInCarbonWindow.location());
        IntPoint const originOfHTMLControlInWindow(getOriginOfHTMLControlInCarbonWindow());
        IntPoint const originOfPluginInHTMLControl(locationInCarbonWindow.x() - originOfHTMLControlInWindow.x(), locationInCarbonWindow.y() - originOfHTMLControlInWindow.y());
 
        if(m_drawingModel == NPDrawingModelQuickDraw)
        {
            setAndUpdateDirty(dirty, originOfPluginInHTMLControl.x(), m_npWindow.x);
            setAndUpdateDirty(dirty, originOfPluginInHTMLControl.y(), m_npWindow.y);
        }
        else 
        {
    setAndUpdateDirty(dirty, locationInCarbonWindow.x(), m_npWindow.x);
    setAndUpdateDirty(dirty, locationInCarbonWindow.y(), m_npWindow.y);
        }
    
        
    int const widthInCarbonWindow =  geometryInCarbonWindow.width();
    int const heightInCarbonWindow =  geometryInCarbonWindow.height();
        
    setAndUpdateDirty(dirty, widthInCarbonWindow, m_npWindow.width);
    setAndUpdateDirty(dirty, heightInCarbonWindow, m_npWindow.height);
    
    
        WebWindow* const webWindow = getApolloWebWindow();
        ASSERT(webWindow);
    WebBitmap* const webBitmap = webWindow->m_pVTable->getBitmapSurface(webWindow);
    void* const bitmapPixels = webBitmap->m_pVTable->getPixelData(webBitmap);
    unsigned long const bitmapWidth = webBitmap->m_pVTable->getWidth(webBitmap);
    unsigned long const bitmapHeight = webBitmap->m_pVTable->getHeight(webBitmap);
    unsigned long const bitmapStride = webBitmap->m_pVTable->getStride(webBitmap);
    ASSERT((bitmapWidth * 4) <= bitmapStride);
    
        if(m_drawingModel == NPDrawingModelQuickDraw)
        {
            IntRect webWindowClipRect = m_clipRect;
            webWindowClipRect.move(m_windowRect.x(), m_windowRect.y());
            
            setAndUpdateDirty(dirty, webWindowClipRect.x(), m_npWindow.clipRect.left);
            setAndUpdateDirty(dirty, webWindowClipRect.y(), m_npWindow.clipRect.top);
            setAndUpdateDirty(dirty, webWindowClipRect.x() + webWindowClipRect.width(), m_npWindow.clipRect.right);
            setAndUpdateDirty(dirty, webWindowClipRect.y() + webWindowClipRect.height(), m_npWindow.clipRect.bottom);
        }
        else
        {
    setAndUpdateDirty(dirty, m_npWindow.x, m_npWindow.clipRect.left);
    setAndUpdateDirty(dirty, m_npWindow.y, m_npWindow.clipRect.top);
    setAndUpdateDirty(dirty, m_npWindow.clipRect.left + widthInCarbonWindow, m_npWindow.clipRect.right);
    setAndUpdateDirty(dirty, m_npWindow.clipRect.top + heightInCarbonWindow, m_npWindow.clipRect.bottom);
        }
    
    setAndUpdateDirty(dirty, bitmapPixels, m_webBitmapPixels);
    setAndUpdateDirty(dirty, bitmapWidth, m_webBitmapWidth);
    setAndUpdateDirty(dirty, bitmapHeight, m_webBitmapHeight);
    setAndUpdateDirty(dirty, bitmapStride, m_webBitmapStride);
    
        WindowRef const carbonWindow = getCarbonWindow();
        if (m_drawingModel == NPDrawingModelCoreGraphics) {
            if (m_npCGContext.window != carbonWindow)
                dirty = true;
    }

    if (!dirty)
        return;
    

    if (m_drawingModel == NPDrawingModelCoreGraphics) {
        if (m_npCGContext.context)
            CGContextRelease(m_npCGContext.context);
        m_npCGContext.context = 0;

        CGColorSpaceRef const colorSpace = CGColorSpaceCreateDeviceRGB();
        m_npCGContext.context = CGBitmapContextCreate(bitmapPixels, bitmapWidth, bitmapHeight, 8, bitmapStride, colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
        CGColorSpaceRelease(colorSpace);
        
        CGAffineTransform const transformToMakeCGContextUseWindowCoords = { 1, 0, 0, -1, originOfPluginInHTMLControl.x(), bitmapHeight - originOfPluginInHTMLControl.y()};
        CGContextConcatCTM(m_npCGContext.context, transformToMakeCGContextUseWindowCoords);
    
        if (m_npCGContext.window != carbonWindow)
            m_npCGContext.window = getCarbonWindow();
        if (m_npWindow.window != &m_npCGContext)
            m_npWindow.window = &m_npCGContext;
    } 
    else {
        ASSERT(m_drawingModel == NPDrawingModelQuickDraw);
        
        ::Rect boundsRect;
        QDDeprecated::SetRect(&boundsRect, 0, 0, bitmapWidth, bitmapHeight);
        
        GWorldPtr newPort = NULL;
        QDDeprecated::NewPortFromBytes(&newPort, &boundsRect, bitmapPixels, bitmapStride);
        
        if (m_npPort.port != NULL)
            QDDeprecated::DisposePortFromBytes(m_npPort.port);
        
        m_npPort.port = newPort;
        m_npPort.portx = -originOfPluginInHTMLControl.x();
        m_npPort.porty = -originOfPluginInHTMLControl.y();

        m_npWindow.window = &m_npPort;
        
        {
            ::HIPoint carbonWindowOrigin = { 0, 0 };
            HIPointConvert(&carbonWindowOrigin
                          , kHICoordSpaceWindow
                          , carbonWindow
                          , kHICoordSpaceScreenPixel
                          , 0);
                          
                //CGrafPtrPusherPopper const savePort(newPort);
            //QDDeprecated::SetOrigin(static_cast<short>(carbonWindowOrigin.x + originOfHTMLControlInWindow.x())
            //                       , static_cast<short>(carbonWindowOrigin.y + originOfHTMLControlInWindow.y()));
        }
    }

    if (m_plugin->pluginFuncs()->setwindow) {
            JSC::JSLock::DropAllLocks dropAllLocks(false);
        setCallingPlugin(true);
        m_plugin->pluginFuncs()->setwindow(m_instance, &m_npWindow);
        setCallingPlugin(false);
    }
}
#endif

namespace {
class CurrentPluginViewScope {
public:
    CurrentPluginViewScope(PluginView* pluginView) : m_originalPluginView(currentPluginView())
    {
        s_currentPluginView = pluginView;
        ASSERT(pluginView == currentPluginView());
    }
    ~CurrentPluginViewScope()
    {
        ASSERT(s_currentPluginView);
        s_currentPluginView = m_originalPluginView;
        ASSERT(currentPluginView() == m_originalPluginView);
    }
    static inline PluginView* currentPluginView() { return s_currentPluginView; };
private:
    CurrentPluginViewScope(const CurrentPluginViewScope&);
    CurrentPluginViewScope& operator=(const CurrentPluginViewScope&);
    static PluginView* s_currentPluginView;
    PluginView* const m_originalPluginView;
};

PluginView* CurrentPluginViewScope::s_currentPluginView = 0;

}

void PluginView::stop()
{
#ifndef NP_NO_QUICKDRAW
    m_nullEventTimer.stop();
#endif        
    if (!m_isStarted)
        return;
    
    HashSet<RefPtr<PluginStream> > streams = m_streams;
    HashSet<RefPtr<PluginStream> >::iterator end = streams.end();
    for (HashSet<RefPtr<PluginStream> >::iterator it = streams.begin(); it != end; ++it) {
        (*it)->stop();
        disconnectStream((*it).get());
    }

    ASSERT(m_streams.isEmpty());

        JSC::JSLock::DropAllLocks dropLocks(false);

    // Clear the window
    m_npWindow.window = 0;
    if (m_plugin->pluginFuncs()->setwindow && !m_plugin->quirks().contains(PluginQuirkDontSetNullWindowHandleOnDestroy)) {
        setCallingPlugin(true);
        m_plugin->pluginFuncs()->setwindow(m_instance, &m_npWindow);
        setCallingPlugin(false);
    }
    
        PluginMainThreadScheduler::scheduler().unregisterPlugin(m_instance);

    // Destroy the plugin
    NPSavedData* savedData = 0;
    setCallingPlugin(true);
    NPError npErr = m_plugin->pluginFuncs()->destroy(m_instance, &savedData);
    setCallingPlugin(false);
    LOG_NPERROR(npErr);
    
    if (savedData) {
        if (savedData->buf)
            NPN_MemFree(savedData->buf);
        NPN_MemFree(savedData);        
    }
    
    m_instance->pdata = 0;
}

#if ENABLE(NETSCAPE_PLUGIN_API)
const char* PluginView::userAgentStatic()
{
    return 0;
}
#endif

const char* PluginView::userAgent()
{
#if PLATFORM(APOLLO)
    if (m_plugin->quirks().contains(PluginQuirkWantsSpecialFlashUserAgent)) {
        unsigned long numUTF8BytesInUA = 0;
        const unsigned char* const uaUTF8Bytes = WebKitApollo::g_HostFunctions->getUserAgentForFlashNPP(&numUTF8BytesInUA);
        ASSERT(uaUTF8Bytes);
        ASSERT(uaUTF8Bytes[numUTF8BytesInUA] == '\0');
        return reinterpret_cast<const char*>(uaUTF8Bytes);
    }
    
    if (m_userAgent.isNull())
        m_userAgent = m_parentFrame->loader()->userAgent(m_url).utf8();
    
    return m_userAgent.data();
#endif    
}
    
namespace {
static const off_t maxPostFileSize = 0x2000000; // 500 Megabytes 
}

NPError PluginView::handlePostReadFile(Vector<char>& buffer, uint32 len, const char* buf)
{
    String filename(buf, len);
    
    if (filename.startsWith("file:///"))
        filename = filename.substring(8);
    CString const fileNameUTF8(filename.utf8());
    int const fd = open(fileNameUTF8.data(), O_RDONLY, 0);
    
    if (fd == -1)
        return NPERR_FILE_NOT_FOUND;
    
    // Get file info
    struct stat attrs;
    int const fstatRet = fstat(fd, &attrs);
    if (fstatRet != 0)
        return NPERR_FILE_NOT_FOUND;
    
    // Make sure file is not in fact a directory.
    if (attrs.st_mode & S_IFDIR)
        return NPERR_FILE_NOT_FOUND;
    
    // Make sure file is not too big.  
    if (attrs.st_size > maxPostFileSize)
        return NPERR_FILE_NOT_FOUND;
    size_t const fileSizeAsSizeT = static_cast<size_t>(attrs.st_size);
    buffer.resize(fileSizeAsSizeT);
    
    int const readRet = read(fd, buffer.data(), fileSizeAsSizeT);
    close(fd);

    if (readRet <= 0 || static_cast<size_t>(readRet) != fileSizeAsSizeT)
        return NPERR_FILE_NOT_FOUND;
    
    return NPERR_NO_ERROR;
}

    NPError PluginView::getValueStatic(NPNVariable /*variable*/, void* /*value*/)
{
    return NPERR_GENERIC_ERROR;
}

NPError PluginView::getValue(NPNVariable variable, void* value)
{
    switch (variable) {
#if ENABLE(NETSCAPE_PLUGIN_API)
        case NPNVWindowNPObject: {
            if (m_isJavaScriptPaused)
                return NPERR_GENERIC_ERROR;

                NPObject* windowScriptObject = m_parentFrame->script()->windowScriptNPObject();

            // Return value is expected to be retained, as described here: <http://www.mozilla.org/projects/plugin/npruntime.html>
            if (windowScriptObject)
                _NPN_RetainObject(windowScriptObject);

            void** v = (void**)value;
            *v = windowScriptObject;
            
            return NPERR_NO_ERROR;
        }

        case NPNVPluginElementNPObject: {
            if (m_isJavaScriptPaused)
                return NPERR_GENERIC_ERROR;

            NPObject* pluginScriptObject = 0;

            if (m_element->hasTagName(HTMLNames::appletTag) || m_element->hasTagName(HTMLNames::embedTag) || m_element->hasTagName(HTMLNames::objectTag))
                pluginScriptObject = static_cast<HTMLPlugInElement*>(m_element)->getNPObject();

            // Return value is expected to be retained, as described here: <http://www.mozilla.org/projects/plugin/npruntime.html>
            if (pluginScriptObject)
                _NPN_RetainObject(pluginScriptObject);

            void** v = (void**)value;
            *v = pluginScriptObject;

            return NPERR_NO_ERROR;
        }
#endif
        case NPNVsupportsCoreGraphicsBool:
            *reinterpret_cast<NPBool*>(value) = TRUE;
            return NPERR_NO_ERROR;
#ifndef NP_NO_QUICKDRAW
        case NPNVsupportsQuickDrawBool:
            *reinterpret_cast<NPBool*>(value) = TRUE;
            return NPERR_NO_ERROR;
#endif
        case NPNVsupportsOpenGLBool:
            *reinterpret_cast<NPBool*>(value) = FALSE;
            return NPERR_NO_ERROR;
        case NPNVpluginDrawingModel:
            *reinterpret_cast<NPDrawingModel*>(value) = m_drawingModel;
        default:
            return NPERR_GENERIC_ERROR;
    }
}

    void PluginView::invalidateRect(const IntRect& rect)
{
    if(!m_isWindowed)
        invalidateWindowlessPluginRect(rect);
    }

void PluginView::invalidateRect(NPRect* rect)
{
    if (!rect) {
        invalidate();
        return;
    }
    
    if (m_isWindowed && m_window)
    {
        ASSERT(0); // need to implement, see windows
        return;
    }
    
    IntRect r(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
    
    if (m_plugin->quirks().contains(PluginQuirkThrottleInvalidate)) {
        m_invalidRects.append(r);
        if (!m_invalidateTimer.isActive())
            m_invalidateTimer.startOneShot(0.001);
    } 
    else
        invalidateRect(r);
}

void PluginView::invalidateRegion(NPRegion region)
{
#if PLATFORM(APOLLO) && defined(NP_NO_QUICKDRAW)
		(void)region;
#else
    if (m_isWindowed)
        return;
    
    RgnHandle const qdRegion = reinterpret_cast<RgnHandle>(region);
    ::Rect r;
    QDDeprecated::GetRegionBounds(qdRegion, &r);
    ASSERT(r.left <= r.right);
    ASSERT(r.top <= r.bottom);
    WebCore::IntRect const wcRect(r.left, r.top, r.right - r.left, r.bottom - r.top);

    invalidateRect(wcRect);
#endif
}

void PluginView::forceRedraw()
{
    Widget::invalidate();
}

PluginView::~PluginView()
{
    stop();

    deleteAllValues(m_requests);

    freeStringArray(m_paramNames, m_paramCount);
    freeStringArray(m_paramValues, m_paramCount);

        m_parentFrame->script()->cleanupScriptObjectsForPlugin(this);

    if (m_plugin && !m_plugin->quirks().contains(PluginQuirkDontUnloadPlugin))
        m_plugin->unload();
        
    if (m_npCGContext.context)
        CGContextRelease(m_npCGContext.context);
#ifndef NP_NO_QUICKDRAW
    if (m_npPort.port)
        QDDeprecated::DisposePortFromBytes(m_npPort.port);
#endif

#if PLATFORM(APOLLO)
    removePlugin();
#endif
}

void PluginView::startUserGesture()
{
    bool const userInput = m_plugin->quirks().contains(PluginQuirkDetectUserInput);
    if (userInput) {
        m_doingUserGesture = true;
		m_userGestureTimer.stop();
		m_userGestureTimer.startOneShot(0.25);
    }
}

void PluginView::userGestureTimerFired(Timer<PluginView>*)
{
    if(isCallingPlugin()) {
        m_userGestureTimer.startOneShot(0.25);
        return;
    }
    
    m_doingUserGesture = false;
}

// --------------- Lifetime management -----------------

void PluginView::init()
{
    if (m_haveInitialized)
        return;
    
    m_haveInitialized = true;

    if (!m_plugin) {
        ASSERT(m_status == PluginStatusCanNotFindPlugin);
        return;
    }

#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
		if (!m_plugin->load(m_plugin->quirks().contains(PluginQuirkNeedsCarbonAPITrapsForQD))) {
#else
    if (!m_plugin->load()) {
#endif
        m_plugin = 0;
        m_status = PluginStatusCanNotLoadPlugin;
        return;
    }

    if (!start()) {
        m_status = PluginStatusCanNotLoadPlugin;
        return;
    }

#ifndef NP_NO_QUICKDRAW
    if (!m_plugin->quirks().contains(PluginQuirkDeferFirstSetWindowCall))
            setNPWindowRect(frameRect());

		m_appTSMDocID = 0;
#endif

    m_status = PluginStatusLoadedSuccessfully;
}

} // namespace WebCore


