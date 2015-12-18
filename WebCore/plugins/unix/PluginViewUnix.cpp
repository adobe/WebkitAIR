/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
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

#include "Document.h"
#include "DocumentLoader.h"
#include "Element.h"
#include "EventNames.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoadRequest.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "Image.h"
#include "HTMLNames.h"
#include "HTMLPlugInElement.h"
#include "HostWindow.h"
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "NotImplemented.h"
#include "Page.h"
#include "PlatformMouseEvent.h"
#include "PlatformKeyboardEvent.h"
#include "PluginDebug.h"
#include "PluginPackage.h"
#include "PluginMainThreadScheduler.h"
#include "PluginStream.h"
#include "npruntime_impl.h"
#include "npapi.h"
#include "runtime_root.h"
#include "Settings.h"
#include "runtime/JSLock.h"
#include "runtime/JSValue.h"
#include "ScriptController.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "gtk2xtbin.h"
#include <gtk/gtk.h>
#include <X11/keysymdef.h>
#include <WindowsKeyboardCodes.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if PLATFORM(APOLLO)
#define XEMBED_SUPPORTED 1
#define ADOBE_NPPDF_HACK    1
#include "FrameLoaderClientApollo.h"
#include <WebKitApollo/WebKit.h>
namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions; }
#endif

#if DEBUG
static FILE *pluginLogFile = NULL;
#define PrintPluginLog(X...)                                           \
        {                                                               \
                if(!pluginLogFile) pluginLogFile = fopen("/tmp/plugin_logs", "w"); \
                if(pluginLogFile)                                \
                {                                                       \
                        fprintf(pluginLogFile, X);               \
                        fflush(pluginLogFile);                   \
                }                                                       \
        }
#else
#define PrintPluginLog(X...)
#endif

//#define FIX_IFRAME_BUG

using JSC::ExecState;
using JSC::Interpreter;
using JSC::JSLock;
using JSC::JSObject;
using JSC::JSValue;
using JSC::UString;

namespace WebCore {

using namespace HTMLNames;

void printWidgetHierarchy(GtkWidget *widget);

static GtkWidget* GetChildWindow(GtkWidget* parentWindow)
{
#if DEBUG
    printWidgetHierarchy(parentWindow);
#endif

    if(!GTK_IS_CONTAINER(parentWindow))
        return NULL;

    GtkWidget *child;
    GList *children = gtk_container_get_children(GTK_CONTAINER(parentWindow));
    if(children)
    {
        GList *firstChild = children;
        if(firstChild)
        {
            child = (GtkWidget*)(firstChild->data);
            g_list_free(children);
            return child;
        }
    }
    return NULL;
}

void PluginView::setFocus(bool focus)
{
    if (platformPluginWidget() && focus)
        gtk_widget_grab_focus(platformPluginWidget());

    Widget::setFocus(focus);
}

void PluginView::show()
{
    init();

    if(m_isEnabled)
        setNPWindowRect(frameRect());

    setSelfVisible(true);

    if (isParentVisible() && platformPluginWidget() && GTK_IS_WIDGET(platformPluginWidget()))
        gtk_widget_show(platformPluginWidget());

    Widget::show();
}

void PluginView::hide()
{
    setSelfVisible(false);

    if (isParentVisible() && platformPluginWidget())
        gtk_widget_hide(platformPluginWidget());

    Widget::hide();
}

static bool isUnixUserGesture(int message)
{
    const int KeyPress = 2;
    const int KeyRelease = 3;

    switch (message) {
        case KeyPress:
        case KeyRelease:
        case ButtonPress:
        case ButtonRelease:
            return true;
        default:
            return false;
    }
}


IntPoint PluginView::clientToWindow(const IntPoint &p) const
{
    return p;
}

IntPoint PluginView::viewportToWindow(const IntPoint &pIn) const
{
    IntPoint p = pIn;

#if PLATFORM(APOLLO)
    WebWindow* pWindow = root()->hostWindow()->getApolloWindow();
	WebIntPoint leftTopView = {p.x(), p.y()};
	WebIntPoint leftTop;
	pWindow->m_pVTable->viewportToWindow(pWindow, &leftTopView, &leftTop);
	p.setX(leftTop.m_x);
	p.setY(leftTop.m_y);
#endif

    return m_isWindowed ? p : clientToWindow(p);
}

void PluginView::paint(GraphicsContext* context, const IntRect& rect)
{
    if (!m_isStarted) {
        // Draw the "missing plugin" image
        paintMissingPluginIcon(context, rect);
        return;
    }

    if (m_isWindowed || context->paintingDisabled() || !isSelfVisible() || !m_isEnabled)
        return;

    IntRect rectInWindow = static_cast<FrameView*>(parent())->contentsToWindow(frameRect());

// Bug 2429238 (swf content is displaying outside of the iframe in html control)
// Actually the issue is that Flash player expects the drawable to be of size equal to that of the complete swf
// It doesn't take just the clipRect area (i.e. only the visible area)
// So, if we want to avoid the black patches of the invisible area, we actually need to use 
// two drawables - one of size same as the swf (which we pass to the Flash player) and another 
// of size same as clipRect (which we use for getting and setting into the HTML control)
#ifdef FIX_IFRAME_BUG 
    PrintPluginLog("orig rectInWindow    %d   %d   %d   %d\n", rectInWindow.x(), rectInWindow.y(), rectInWindow.width(), rectInWindow.height());
    rectInWindow.setX(rectInWindow.x() + m_clipRect.x());
    rectInWindow.setY(rectInWindow.y() + m_clipRect.y());
    rectInWindow.setWidth(m_clipRect.width());
    rectInWindow.setHeight(m_clipRect.height());
    PrintPluginLog("new  rectInWindow    %d   %d   %d   %d\n", rectInWindow.x(), rectInWindow.y(), rectInWindow.width(), rectInWindow.height());
#endif

    GdkDrawable* gdkdrawable = context->getDrawable(rectInWindow, m_isTransparent);

#ifdef FIX_IFRAME_BUG
    m_windowlessDrawable = GDK_DRAWABLE(gdk_pixmap_new(NULL, frameRect().width(), frameRect().height(), gdk_drawable_get_depth(gdkdrawable)));
    gdk_draw_drawable(m_windowlessDrawable, gdk_gc_new(gdkdrawable), gdkdrawable, 0, 0, m_clipRect.x(), m_clipRect.y(), m_clipRect.width(), m_clipRect.height());
#else
    m_windowlessDrawable = gdkdrawable;
#endif

    m_npWindow.type = NPWindowTypeDrawable;
    m_npWindow.window = (void*)GDK_WINDOW_XWINDOW(m_windowlessDrawable);

    setNPWindowRect(frameRect());

    PrintPluginLog("::paint m_clipRect    %d   %d   %d   %d\n", m_clipRect.x(), m_clipRect.y(), m_clipRect.width(), m_clipRect.height());
    PrintPluginLog("::paint frameRect    %d   %d   %d   %d\n", frameRect().x(), frameRect().y(), frameRect().width(), frameRect().height());

    NPEvent npEvent_paint;
    memset(&npEvent_paint, 0, sizeof(NPEvent));
    npEvent_paint.type = GraphicsExpose;
    npEvent_paint.xgraphicsexpose.display = GDK_WINDOW_XDISPLAY(m_windowlessDrawable);
    npEvent_paint.xgraphicsexpose.drawable = GDK_WINDOW_XID(m_windowlessDrawable);

    npEvent_paint.xgraphicsexpose.x = m_clipRect.x();
    npEvent_paint.xgraphicsexpose.y = m_clipRect.y();

    if(m_clipRect.x() > 0)
        npEvent_paint.xgraphicsexpose.width = frameRect().width();
    else
        npEvent_paint.xgraphicsexpose.width = m_clipRect.width();

    if(m_clipRect.y() > 0)
        npEvent_paint.xgraphicsexpose.height = frameRect().height();
    else
        npEvent_paint.xgraphicsexpose.height = m_clipRect.height();

    PrintPluginLog ("::paint FP %d %d %d %d\n", npEvent_paint.xgraphicsexpose.x, npEvent_paint.xgraphicsexpose.y,
                                                npEvent_paint.xgraphicsexpose.width, npEvent_paint.xgraphicsexpose.height);

    dispatchNPEvent(npEvent_paint);

#ifdef FIX_IFRAME_BUG
    gdk_draw_drawable(gdkdrawable, gdk_gc_new(m_windowlessDrawable), m_windowlessDrawable, m_clipRect.x(), m_clipRect.y(), 0, 0, m_clipRect.width(), m_clipRect.height());
    g_object_unref(m_windowlessDrawable);
#endif

    context->releaseDrawable(rectInWindow);
}

bool PluginView::dispatchNPEvent(NPEvent& npEvent)
{
    if (!m_plugin->pluginFuncs()->event)
        return true;

    bool shouldPop = false;

    if (m_plugin->pluginFuncs()->version < NPVERS_HAS_POPUPS_ENABLED_STATE && isUnixUserGesture(npEvent.type)) {
        pushPopupsEnabledState(true);
        shouldPop = true;
    }

    JSC::JSLock::DropAllLocks dropAllLocks(false);
    setCallingPlugin(true);
    bool result = m_plugin->pluginFuncs()->event(m_instance, &npEvent);
    setCallingPlugin(false);

    if (shouldPop)
        popPopupsEnabledState();

    return result;
}

void PluginView::handleKeyboardEvent(KeyboardEvent* event)
{
    GtkWidget *cwindow = m_parentFrame->view()->hostWindow()->platformPageClient();
    if(!cwindow)
        return;

   /*
    * Reason for hardcoding KeyPress and KeyRelease values below:
    *   Following is the inclusion hierarchy of headers:
    *   ../../../../../Content/html/WebCore/plugins/PluginStream.h
    *      ../../../../../Content/html/WebCore/bridge/npruntime_internal.h
    *        ../../../../../Content/html/WebCore/bridge/npapi.h  (includes X11/X.h that defines KeyPress and KeyRelease)
    *   npruntime_internal.h after including npapi.h undefines the KeyPress and KeyRelease macros
    *   (see npruntime_internal.h for comments)
    */
    const int KeyPress = 2;
    const int KeyRelease = 3;

    unsigned xkeysym = 0;
    switch(event->keyCode())
    {
        case VK_BACK:		xkeysym = XK_BackSpace; break;
        case VK_TAB:		xkeysym = XK_Tab; break;
        case VK_CLEAR:		xkeysym = XK_Clear; break;
        case VK_RETURN:		xkeysym = XK_Return; break;
        case VK_SHIFT:		xkeysym = XK_Shift_Lock; break;
        case VK_CONTROL:	xkeysym = XK_Control_L; break;
        case VK_MENU:		xkeysym = XK_Menu; break;
        case VK_PAUSE:		xkeysym = XK_Pause; break;
        case VK_CAPITAL:	xkeysym = XK_Caps_Lock; break;
        case VK_KANA:		xkeysym = XK_Kana_Lock; break;
        //case VK_HANGUL:	xkeysym = XK_Hangul; break;
        //case VK_HANJA:	xkeysym = XK_Hangul_Hanja; break;
        case VK_KANJI:		xkeysym = XK_Kanji; break;
        case VK_ESCAPE:		xkeysym = XK_Escape; break;
        case VK_MODECHANGE:	xkeysym = XK_Mode_switch; break;
        case VK_SPACE:		xkeysym = XK_space; break;
        case VK_PRIOR:		xkeysym =XK_Prior; break;
        case VK_NEXT:		xkeysym =XK_Next; break;
        case VK_END:		xkeysym =XK_End; break;
        case VK_HOME:		xkeysym =XK_Home; break;
        case VK_LEFT:		xkeysym =XK_Left; break;
        case VK_UP:		xkeysym =XK_Up; break;
        case VK_RIGHT:		xkeysym =XK_Right; break;
        case VK_DOWN:		xkeysym =XK_Down; break;
        case VK_PRINT:		xkeysym =XK_Print; break;
        case VK_INSERT:		xkeysym =XK_Insert; break;
        case VK_DELETE:		xkeysym =XK_Delete; break;
        case VK_NUMPAD0:	xkeysym =XK_KP_0; break;
        case VK_NUMPAD1:	xkeysym =XK_KP_1; break;
        case VK_NUMPAD2:	xkeysym =XK_KP_2; break;
        case VK_NUMPAD3:	xkeysym =XK_KP_3; break;
        case VK_NUMPAD4:	xkeysym =XK_KP_4; break;
        case VK_NUMPAD5:	xkeysym =XK_KP_5; break;
        case VK_NUMPAD6:	xkeysym =XK_KP_6; break;
        case VK_NUMPAD7:	xkeysym =XK_KP_7; break;
        case VK_NUMPAD8:	xkeysym =XK_KP_8; break;
        case VK_NUMPAD9:	xkeysym =XK_KP_9; break;
        case VK_MULTIPLY:	xkeysym =XK_KP_Multiply; break;
        case VK_ADD:		xkeysym =XK_KP_Add; break;
        case VK_SEPARATOR:	xkeysym =XK_KP_Separator; break;
        case VK_SUBTRACT:	xkeysym =XK_KP_Subtract; break;
        case VK_DECIMAL:	xkeysym =XK_KP_Decimal; break;
        case VK_DIVIDE:		xkeysym =XK_KP_Divide; break;
        case VK_F1:		xkeysym = XK_F1; break;
        case VK_F2:		xkeysym = XK_F2; break;
        case VK_F3:		xkeysym = XK_F3; break;
        case VK_F4:		xkeysym = XK_F4; break;
        case VK_F5:		xkeysym = XK_F5; break;
        case VK_F6:		xkeysym = XK_F6; break;
        case VK_F7:		xkeysym = XK_F7; break;
        case VK_F8:		xkeysym = XK_F8; break;
        case VK_F9:		xkeysym = XK_F9; break;
        case VK_F10:		xkeysym = XK_F10; break;
        case VK_F11:		xkeysym = XK_F11; break;
        case VK_F12:		xkeysym = XK_F12; break;
        case VK_F13:		xkeysym = XK_F13; break;
        case VK_F14:		xkeysym = XK_F14; break;
        case VK_F15:		xkeysym = XK_F15; break;
        case VK_F16:		xkeysym = XK_F16; break;
        case VK_F17:		xkeysym = XK_F17; break;
        case VK_F18:		xkeysym = XK_F18; break;
        case VK_F19:		xkeysym = XK_F19; break;
        case VK_F20:		xkeysym = XK_F20; break;
        case VK_F21:		xkeysym = XK_F21; break;
        case VK_F22:		xkeysym = XK_F22; break;
        case VK_F23:		xkeysym = XK_F23; break;
        case VK_F24:		xkeysym = XK_F24; break;
        case VK_NUMLOCK:	xkeysym =XK_Num_Lock; break;
        case VK_SCROLL:		xkeysym =XK_Scroll_Lock; break;
        case VK_LSHIFT:		xkeysym =XK_Shift_L; break;
        case VK_RSHIFT:		xkeysym =XK_Shift_R; break;
        case VK_LCONTROL:	xkeysym =XK_Control_L; break;
        case VK_RCONTROL:	xkeysym =XK_Control_R; break;
        case VK_OEM_COMMA:	xkeysym = XK_comma; break;
        case VK_OEM_MINUS:	xkeysym = XK_minus; break;
        case VK_OEM_PERIOD:	xkeysym = XK_period; break;
        case VK_OEM_PLUS:	xkeysym = XK_plus; break;
        case VK_OEM_1:		xkeysym = XK_semicolon; break;
        case VK_OEM_2:		xkeysym = XK_slash; break;
	case VK_OEM_3:		xkeysym = XK_asciitilde; break;
        case VK_OEM_4:		xkeysym = XK_bracketleft; break;
        case VK_OEM_5:		xkeysym = XK_backslash; break;
        case VK_OEM_6:		xkeysym = XK_bracketright; break;
        case VK_OEM_7:		xkeysym = XK_quotedbl; break;
        default:		xkeysym = event->keyCode(); break;
    }

    NPEvent npEvent;
    memset(&npEvent, 0, sizeof(NPEvent));

    npEvent.xkey.keycode = XKeysymToKeycode(GDK_WINDOW_XDISPLAY(cwindow->window), xkeysym);

    if(event->shiftKey())
        npEvent.xkey.state |= ShiftMask;
    if(event->ctrlKey())
        npEvent.xkey.state |= ControlMask;
    if(event->altKey())
        npEvent.xkey.state |= Mod3Mask;
    if(event->metaKey())
        npEvent.xkey.state |= Mod1Mask;
    if(event->keyEvent()->currentCapsLockState())
        npEvent.xkey.state |= LockMask;

    npEvent.xkey.display = GDK_WINDOW_XDISPLAY(cwindow->window);
    npEvent.xkey.window = GDK_WINDOW_XID(cwindow->window);

    if (event->type() == eventNames().keydownEvent) {
        npEvent.type = KeyPress;
        npEvent.xkey.type = KeyPress;
    } else if (event->type() == eventNames().keyupEvent) {
        npEvent.type = KeyRelease;
        npEvent.xkey.type = KeyRelease;
    }

    startUserGesture();

    JSC::JSLock::DropAllLocks dropLocks(false);
    if (!dispatchNPEvent(npEvent))
        event->setDefaultHandled();
}

void PluginView::handleMouseEvent(MouseEvent* event)
{
    PrintPluginLog ("handleMouseEvent  %d %d\n", event->pageX(), event->pageY());

    NPEvent npEvent;
    memset(&npEvent, 0, sizeof(NPEvent));

    IntPoint p(event->pageX(), event->pageY());

    if (event->type() == eventNames().mousemoveEvent) {
        npEvent.type = MotionNotify;
        npEvent.xmotion.x = p.x() - frameRect().x();
        npEvent.xmotion.y = p.y() - frameRect().y();
        PrintPluginLog ("\thandleMouseEvent  MOVE  %d %d\n", npEvent.xmotion.x, npEvent.xmotion.y);
    }
    else if (event->type() == eventNames().mousedownEvent) {
        npEvent.type = ButtonPress;
        npEvent.xbutton.x = p.x() - frameRect().x();
        npEvent.xbutton.y = p.y() - frameRect().y();
        npEvent.xbutton.button = event->button() + 1;
        npEvent.xbutton.time = gtk_get_current_event_time ();
	if(event->button() == 0)	//left
             m_parentFrame->document()->setFocusedNode(m_element);
        PrintPluginLog ("\thandleMouseEvent  DOWN  %d %d\n", npEvent.xbutton.x, npEvent.xbutton.y);
    } else if (event->type() == eventNames().mouseupEvent) {
        npEvent.type = ButtonRelease;
        npEvent.xbutton.x = p.x() - frameRect().x();
        npEvent.xbutton.y = p.y() - frameRect().y();
        npEvent.xbutton.button = event->button() + 1;
        npEvent.xbutton.time = gtk_get_current_event_time ();
        PrintPluginLog ("\thandleMouseEvent  UP  %d %d\n", npEvent.xbutton.x, npEvent.xbutton.y);
    } else
        return;

    startUserGesture();

    JSC::JSLock::DropAllLocks dropLocks(false);
    if (!dispatchNPEvent(npEvent))
        event->setDefaultHandled();
}

void PluginView::setNPWindowRect(const IntRect& rect)
{
    if (!m_isStarted)
        return;
/*
    if (!m_isStarted || !isSelfVisible() || !isParentVisible())
        return;
*/
    if(m_isWindowed && (!platformPluginWidget() || !(m_npWindow.window)))
        return;
/*
    if(!m_isEnabled)
        return;
*/

    // Flashplayer is not able to draw for height,width <= 1. 
    if(rect.width() <= 1 && rect.height() <= 1)
        return;

    if(rect.width() == 0 || rect.height() == 0)
        return;

    IntPoint p = m_parentFrame->view()->contentsToWindow(rect.location());

#if PLATFORM(APOLLO)
    if(m_isWindowed)
    {
        // make it relative to the window
        p = convertToApolloWindowPoint(p);
    }
#endif //PLATFORM(APOLLO)

    if(m_isWindowed)
    {
        m_npWindow.x = p.x();
        m_npWindow.y = p.y();
    }
    else
    {
        m_npWindow.x = 0;
        m_npWindow.y = 0;
    }

    m_npWindow.width = rect.width();
    m_npWindow.height = rect.height();

    m_npWindow.clipRect.left = 0;
    m_npWindow.clipRect.top = 0;
    m_npWindow.clipRect.right = rect.width();
    m_npWindow.clipRect.bottom = rect.height();

    if(m_isWindowed && !m_needsXEmbed)
    {
        if(!m_gtkXtBin || !m_gtkXtBin->window)
            return;
    }

    if(m_isWindowed)
    	PopulateWindowAndWSInfoForNPWindow();

    if (m_plugin->pluginFuncs()->setwindow) {
        JSC::JSLock::DropAllLocks dropAllLocks(false);
	setCallingPlugin(true);
        m_plugin->pluginFuncs()->setwindow(m_instance, &m_npWindow);
        PrintPluginLog ("setNPWindowRect  %d %d\n", rect.width(), rect.height());
	setCallingPlugin(false);
    }
}

namespace {
   static const off_t maxPostFileSize = 0x2000000; // 500 Megabytes 
}    

NPError PluginView::handlePostReadFile(Vector<char>& buffer, uint32_t len, const char* buf)
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

bool PluginView::platformGetValueStatic(NPNVariable variable, void* value, NPError* result)
{
    switch (variable) {
	// For XEmbed Support
        case NPNVxDisplay:
        {
		/*
 		 * Ask the plugin if it requires XEmbed support
 		 * For the case when Reader would start supporting XEmbed
                NPBool rtv = false;
		m_plugin->pluginFuncs()->getvalue(m_instance, NPPVpluginNeedsXEmbed, &rtv);
                if (rtv) {
                    (*(Display **)value) = GDK_DISPLAY();
                    m_needsXEmbed = true;
                    return NPERR_NO_ERROR;
                }
		*/

#if ADOBE_NPPDF_HACK
                // Adobe nppdf hack (picked up from Mozilla Firefox source code)
                // Adobe nppdf uses Xt (X Toolkit) to get a display from the browser and then
                // use it to embed adobe reader inside.
                // adobe nppdf calls XtGetApplicationNameAndClass(display, &instance, &class) and so
                // we have to init Xt toolkit. Before getting XtDisplay, just call gtk_xtbin_new(w,0) once

                // TODO - gtkxtbin is not a part of standard gtk. Mozilla has implemented this on its
                // own and so webkit would also need the same.

                static GtkWidget *xtbin = 0;
                if (!xtbin) {
                    xtbin = gtk_xtbin_new(GDK_ROOT_PARENT(),0);
                    // it crashes on destroy, let it leak    // also a part of mozilla code
                    // gtk_widget_destroy(xtbin);
                }
                (*(Display **)value) =  GTK_XTBIN(xtbin)->xtdisplay;
                *result =  NPERR_NO_ERROR;
#else
                *result =  NPERR_GENERIC_ERROR;
#endif
                return true;
        }

        default:
            return false;
    }
}

void PluginView::invalidateRect(const IntRect& r)
{
    if (m_isWindowed) {
        GdkRectangle invalidRect = {r.x(), r.y(), r.width(), r.height()};
        gdk_window_invalidate_rect(platformPluginWidget()->window, &invalidRect, false);
        return;
    }

    invalidateWindowlessPluginRect(r);
}

void PluginView::invalidateRect(NPRect* rect)
{
    if (!rect) {
        invalidate();
        return;
    }

    IntRect r(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);

    if (m_isWindowed) {
        GdkRectangle invalidRect = {r.x(), r.y(), r.width(), r.height()};
        gdk_window_invalidate_rect(platformPluginWidget()->window, &invalidRect, false);
    } else {
        if (m_plugin->quirks().contains(PluginQuirkThrottleInvalidate)) {
            m_invalidRects.append(r);
            if (!m_invalidateTimer.isActive())
                m_invalidateTimer.startOneShot(0.001);
        } else
            invalidateRect(r);
    }
}

void PluginView::invalidateRegion(NPRegion region)
{
    if (m_isWindowed)
        return;

    XRectangle r;
    XClipBox(region, &r);

    IntRect rect(IntPoint(r.x, r.y), IntSize(r.width, r.height));
    invalidateRect(rect);
}

void PluginView::forceRedraw()
{
    if (m_isWindowed)
        gdk_window_invalidate_region(platformPluginWidget()->window, NULL, false);
    else
        Widget::invalidate();
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

/*void PluginView::startUserGestureForWindowedPlugins()
{
    startUserGesture();
}*/

void PluginView::userGestureTimerFired(Timer<PluginView>*)
{
    m_doingUserGesture = false;
}

static gboolean
flash_canvas_event_handler (GtkWidget *widget, GdkEventButton* event, gpointer data)
{
    if(data)
    {
        switch (event->type)
        {
            case GDK_KEY_PRESS:
            case GDK_KEY_RELEASE:
            case GDK_BUTTON_PRESS:
            case GDK_BUTTON_RELEASE:
            	// ach: todo
                //((PluginView*)data)->startUserGestureForWindowedPlugins();
                break;
            default:
                break;
        }
    }
    return false;
}

gboolean
plug_removed_cb (GtkWidget *widget, gpointer data)
{
    return TRUE;
}

static gboolean
flash_canvas_added_to_plug (GtkContainer *container, GtkWidget *widget, gpointer data)
{
    GtkWidget *flashCanvas = widget;

    unsigned int eventfilter = GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK;
    gtk_widget_add_events (flashCanvas, eventfilter);
    g_signal_connect(G_OBJECT(flashCanvas), "event", G_CALLBACK(flash_canvas_event_handler), (void*) data);

    return true;
}

static gboolean
plug_added_cb (GtkWidget *widget, gpointer data)
{
    if(((GtkSocket*)widget)->plug_widget)
        g_signal_connect(G_OBJECT(((GtkSocket*)widget)->plug_widget), "add", G_CALLBACK(flash_canvas_added_to_plug), (void*) data);
    return true;
}

NPError PluginView::PopulateWindowAndWSInfoForNPWindow() 
{    
    if(!m_npWindow.width || !m_npWindow.height)
        return NPERR_GENERIC_ERROR;

    GdkWindow *gdkWindow = platformPluginWidget()->window;
    if (!gdkWindow)
        return NPERR_GENERIC_ERROR;

    m_npWindow.ws_info = (NPSetWindowCallbackStruct *)malloc(sizeof(NPSetWindowCallbackStruct));
 
    if(!m_needsXEmbed)
    {    
        if(!m_gtkXtBin)
            m_gtkXtBin = gtk_xtbin_new(gdkWindow, 0);
        if(!m_gtkXtBin)
            return NPERR_GENERIC_ERROR;
        gtk_widget_set_usize(m_gtkXtBin, m_npWindow.width, m_npWindow.height);
        gtk_widget_show(m_gtkXtBin);
    }    

    NPSetWindowCallbackStruct *ws; 
    ws = (NPSetWindowCallbackStruct *)m_npWindow.ws_info;

    // fill in window info structure
    ws->type = 0; 
    ws->depth = gdk_window_get_visual(gdkWindow)->depth;

    if (!m_needsXEmbed)
        ws->display = GTK_XTBIN(m_gtkXtBin)->xtdisplay;
    else 
        ws->display = GDK_WINDOW_XDISPLAY(gdkWindow);

    ws->visual = GDK_VISUAL_XVISUAL(gdk_window_get_visual(gdkWindow));
    ws->colormap = GDK_COLORMAP_XCOLORMAP(gdk_window_get_colormap(gdkWindow));

    XFlush(ws->display);


    if (!m_needsXEmbed) {
    // And now point the NPWindow structures window to the actual X window
        m_npWindow.window = (void*)GTK_XTBIN(m_gtkXtBin)->xtwindow;
        gtk_xtbin_resize(m_gtkXtBin, m_npWindow.width, m_npWindow.height);
    }    

    return NPERR_NO_ERROR;
}    

// apollo integrate
// functions implemented in the generic PluginView.cpp
// disabling for linking
// moved to platformDestroy
#if 0
void PluginView::init()
{
    if (m_haveInitialized)
        return;
    m_haveInitialized = true;

    if (!m_plugin) {
        ASSERT(m_status == PluginStatusCanNotFindPlugin);
        return;
    }

    if (!m_plugin->load()) {
        m_plugin = 0;
        m_status = PluginStatusCanNotLoadPlugin;
        return;
    }

    if (!start()) {
        m_status = PluginStatusCanNotLoadPlugin;
        return;
    }

    if (m_isWindowed) {

    	// Flashplayer is not able to draw for height,width <= 1. 
    	if(frameRect().width() <= 1 && frameRect().height() <= 1)
            return ;

        // This has been done because without showing the gtk_widget, its
        // corresponding  window is not getting created
        setSelfVisible(true);

#if XEMBED_SUPPORTED
        GtkContainer *container = GTK_CONTAINER(m_parentFrame->view()->hostWindow()->platformPageClient());
        if(!container)
        {
            m_npWindow.type = NPWindowTypeWindow;
            m_npWindow.window = 0;
            return;
        }

	m_needsXEmbed = false;
	m_plugin->pluginFuncs()->getvalue(m_instance, NPPVpluginNeedsXEmbed, &m_needsXEmbed);

	if(m_needsXEmbed)
        	setPlatformPluginWidget(gtk_socket_new());
	else
		setPlatformPluginWidget(gtk_layout_new(NULL, NULL));

        gtk_container_add(container, platformPluginWidget());

	if(m_needsXEmbed)
	{
        	// When a plug is removed/destroyed, its corresponding socket is then destroyed.
        	// Another case is when we try to destroy the socket first. That results in
        	// destruction of the plug also which in turn again tries to destroy the parent
        	// socket. This way, the socket is tried to destroy twice - resulting into
        	// random crashes. To avoid this, we handle the plug_removed signal here in
        	// which do not do anything (by returning true) thereby preventing the second time
        	// destruction of the socket.
        	g_signal_connect(platformPluginWidget(), "plug_removed", G_CALLBACK(plug_removed_cb), NULL);
	}

        gtk_widget_realize(platformPluginWidget());
        if (isVisible() && GTK_IS_WIDGET(platformPluginWidget()))
            gtk_widget_show(platformPluginWidget());

        g_object_set_data(G_OBJECT(platformPluginWidget()), "kWebPluginViewProperty", this);

        m_npWindow.type = NPWindowTypeWindow;

        GdkWindow *gdkWindow = ((GtkWidget*)container)->window;

        if(!m_needsXEmbed)
        {
           if(!m_gtkXtBin)
	    {
                m_gtkXtBin = gtk_xtbin_new(platformPluginWidget()->window, 0);
		if(!m_gtkXtBin || !m_gtkXtBin->window)
			return;
            }
           gtk_widget_set_usize(m_gtkXtBin, m_npWindow.width, m_npWindow.height);
           gtk_widget_show(m_gtkXtBin);
        }

        // And now point the NPWindow structures window to the actual X window
	if(m_needsXEmbed)
            m_npWindow.window = (void*)gtk_socket_get_id(GTK_SOCKET(platformPluginWidget()));
	else
            m_npWindow.window = (void*)GTK_XTBIN(m_gtkXtBin)->xtwindow;

        if(!m_npWindow.width || !m_npWindow.height)
            return;

#else
        GtkContainer *container = GTK_CONTAINER(m_parentFrame->view()->hostWindow()->platformPageClient());
        if(!container)
           return;

        setPlatformPluginWidget(gtk_window_new(GTK_WINDOW_TOPLEVEL));

        gtk_widget_set_parent_window(platformPluginWidget(), (container->widget).window);

        if (m_isVisible)
            gtk_window_present(GTK_WINDOW(platformPluginWidget()));

        g_object_set_data(G_OBJECT(platformPluginWidget()), "kWebPluginViewProperty", this);

        m_npWindow.type = NPWindowTypeWindow;
        m_npWindow.window = (void*)GDK_WINDOW_XWINDOW(platformPluginWidget()->window);
#endif

    } else {

	m_window = 0;

        m_npWindow.type = NPWindowTypeDrawable;
        m_npWindow.window = 0;

        m_npWindow.ws_info = (NPSetWindowCallbackStruct *)malloc(sizeof(NPSetWindowCallbackStruct));
        NPSetWindowCallbackStruct *ws = (NPSetWindowCallbackStruct *)m_npWindow.ws_info;

        // fill in window info structure
        GtkWidget *w = m_parentFrame->view()->hostWindow()->platformPageClient();
	if(w)
        {
            GdkWindow *gdkWindow = w->window;
            ws->type = 0;
            ws->depth = gdk_window_get_visual(gdkWindow)->depth;
            ws->display = GDK_WINDOW_XDISPLAY(gdkWindow);
            ws->visual = GDK_VISUAL_XVISUAL(gdk_window_get_visual(gdkWindow));
            ws->colormap = GDK_COLORMAP_XCOLORMAP(gdk_window_get_colormap(gdkWindow));
         }
         else
         {
            ws->type = 0;
            ws->depth = 24;
            ws->display = XOpenDisplay(0);
            ws->visual = DefaultVisual(ws->display, DefaultScreen(ws->display));
            ws->colormap = DefaultColormap(ws->display, DefaultScreen(ws->display));
         }
    }

    if (!m_plugin->quirks().contains(PluginQuirkDeferFirstSetWindowCall))
        setNPWindowRect(frameRect());

    m_status = PluginStatusLoadedSuccessfully;
}
#endif

// Use this function to debug the widget hierarchy
void printWidgetHierarchy(GtkWidget *widget)
{
    if(!GTK_IS_WIDGET(widget))
         return;

    static int indent = 0;
    int w, h;
    gtk_widget_get_size_request(widget, &w, &h);
    PrintPluginLog ("%s (%p)(%d x %d) \n", GTK_OBJECT_TYPE_NAME(widget), widget, w, h);
    if (GTK_IS_CONTAINER(widget))
    {
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
        GList *current_child = NULL;
        indent++;
        if(!children)
            PrintPluginLog ("---------- This GTK container (%p) does not have any children\n", widget);
        for(current_child = children; current_child != NULL; current_child = current_child->next)
        {
            for(int i=0; i<indent*4; i++)
            {
                PrintPluginLog("-");
            }
            printWidgetHierarchy((GtkWidget*)(current_child->data));
            PrintPluginLog("\n");
        }
        g_list_free(children);
        indent--;
    }
    else
       PrintPluginLog ("---------- This is not a GTK container\n");

}

void PluginView::updatePluginWidget()
{
	if(!isSelfVisible())
		return;

	FrameView* frameView = m_parentFrame->view();

	IntRect oldWindowRect = m_windowRect;
	IntRect oldClipRect = m_clipRect;

	m_windowRect = IntRect(frameView->contentsToWindow(frameRect().location()), frameRect().size());

	m_clipRect = windowClipRect();

	PrintPluginLog("1 m_windowRect    %d   %d   %d   %d\n", m_windowRect.x(), m_windowRect.y(), m_windowRect.width(), m_windowRect.height());
	PrintPluginLog("1 m_clipRect    %d   %d   %d   %d\n", m_clipRect.x(), m_clipRect.y(), m_clipRect.width(), m_clipRect.height());

#if PLATFORM(APOLLO)
	m_windowRect = convertToApolloWindowRect(m_windowRect);
	m_clipRect = convertToApolloWindowRect(m_clipRect);
#endif

	m_clipRect.move(-m_windowRect.x(), -m_windowRect.y());

	if(!m_isWindowed || !GTK_IS_WIDGET(m_window))
		return;

	if (m_window && m_windowRect.width() && m_windowRect.height())
	{
		setCallingPlugin(true);

		PrintPluginLog("3 m_windowRect    %d   %d   %d   %d\n", m_windowRect.x(), m_windowRect.y(), m_windowRect.width(), m_windowRect.height());
		PrintPluginLog("3 m_clipRect    %d   %d   %d   %d\n", m_clipRect.x(), m_clipRect.y(), m_clipRect.width(), m_clipRect.height());

		gtk_widget_set_size_request(m_window, m_clipRect.width(), m_clipRect.height());
		gtk_widget_set_uposition(m_window, m_windowRect.x() + m_clipRect.x(), m_windowRect.y() + m_clipRect.y());

		if(!m_needsXEmbed && m_gtkXtBin && m_gtkXtBin->window)	
		{
			gtk_xtbin_resize(m_gtkXtBin, m_windowRect.width(), m_windowRect.height());
           		gtk_widget_set_usize(m_gtkXtBin, m_npWindow.width, m_npWindow.height);
		}

		// Handle the child window content
		GtkWidget* gtkChild = GetChildWindow(m_window);

		// There is 2-level hierarchy for Xembed (flash windows). So get the child's child.
		if(m_needsXEmbed)
		{
			if(gtkChild)
				gtkChild = GetChildWindow(gtkChild);
		}

		if(gtkChild)
		{
			gdk_window_move_resize(gtkChild->window, -m_clipRect.x(), -m_clipRect.y(), m_windowRect.width(), m_windowRect.height());
		}

		gtk_widget_set_size_request(m_window, m_clipRect.width(), m_clipRect.height());

/*
		// For windowless mode

		GdkRectangle rect;
		rect.x = m_clipRect.x();
		rect.y = m_clipRect.y();
		rect.width = m_clipRect.width();
		rect.height = m_clipRect.height();
		gdk_window_invalidate_rect(m_window->window, &rect, false);
*/

		setCallingPlugin(false);

	}
}

void PluginView::setParentVisible(bool visible)
{
	if (isParentVisible() == visible)
		return;

	Widget::setParentVisible(visible);

	if (isSelfVisible() && GTK_IS_WIDGET(platformPluginWidget())) {
		if (visible && m_isEnabled)
			gtk_widget_show(platformPluginWidget());
		else
			gtk_widget_hide(platformPluginWidget());
	}
}

void PluginView::setParent(ScrollView* parent)
{
	Widget::setParent(parent);

	if (parent)
		init();
	else {
		if (!platformPluginWidget())
			return;

		// If the plug-in window or one of its children have the focus, we need to
		// clear it to prevent the web view window from being focused because that can
		// trigger a layout while the plugin element is being detached.
		// WINDOWS
		//HWND focusedWindow = ::GetFocus();
		//if (platformPluginWidget() == focusedWindow || ::IsChild(platformPluginWidget(), focusedWindow))
		//	::SetFocus(0);
	}
}

#if PLATFORM(APOLLO) && ENABLE(NETSCAPE_PLUGIN_API)
bool PluginView::handleOnCopy()
{
    ASSERT(m_parentFrame);
    ASSERT(m_parentFrame->document());
    ASSERT(m_parentFrame->document()->defaultView());

    WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
        , 'c' /* winCharCode */
        , 0x43 /* Web_VK_C */ /* keyCode*/
        , false /* isShiftKey */
        , true /* isCtrlKey */
        , false /* isAltKey */ 
        , false  /* isMetaKey */
        , false  /* isAutoRepeat */);
    RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent, m_parentFrame->document()->defaultView());
    handleEvent(keyDown.get());
    return true;
}

bool PluginView::handleOnCut()
{
    ASSERT(m_parentFrame);
    ASSERT(m_parentFrame->document());
    ASSERT(m_parentFrame->document()->defaultView());

    WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
        , 'x' /* winCharCode */
        , 0x58 /* Web_VK_X */ /* keyCode*/
        , false /* isShiftKey */
        , true /* isCtrlKey */
        , false /* isAltKey */ 
        , false  /* isMetaKey */
        , false  /* isAutoRepeat */);
    RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent, m_parentFrame->document()->defaultView());
    handleEvent(keyDown.get());
    return true;
}

bool PluginView::handleOnPaste()
{
    ASSERT(m_parentFrame);
    ASSERT(m_parentFrame->document());
    ASSERT(m_parentFrame->document()->defaultView());

    WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
        , 'v' /* winCharCode */
        , 0x56 /* Web_VK_V */ /* keyCode*/
        , false /* isShiftKey */
        , true /* isCtrlKey */
        , false /* isAltKey */ 
        , false  /* isMetaKey */
        , false  /* isAutoRepeat */);
    RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent, m_parentFrame->document()->defaultView());
    handleEvent(keyDown.get());
    return true;
}

bool PluginView::handleSelectAll()
{
    ASSERT(m_parentFrame);
    ASSERT(m_parentFrame->document());
    ASSERT(m_parentFrame->document()->defaultView());

    WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
        , 'a' /* winCharCode */
        , 0x41 /* Web_VK_A */ /* keyCode*/
        , false /* isShiftKey */
        , true /* isCtrlKey */
        , false /* isAltKey */ 
        , false  /* isMetaKey */
        , false  /* isAutoRepeat */);
    RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent, m_parentFrame->document()->defaultView());
    handleEvent(keyDown.get());
    return true;
}
#endif

bool PluginView::platformStart()
{
    ASSERT(m_isStarted);
    ASSERT(m_status == PluginStatusLoadedSuccessfully);

   if (m_isWindowed) {

        // Flashplayer is not able to draw for height,width <= 1. 
        if(frameRect().width() <= 1 && frameRect().height() <= 1)
            return false;

        // This has been done because without showing the gtk_widget, its
        // corresponding  window is not getting created
        setSelfVisible(true);

#if XEMBED_SUPPORTED
        GtkContainer *container = GTK_CONTAINER(m_parentFrame->view()->hostWindow()->platformPageClient());
        if(!container)
        {
            m_npWindow.type = NPWindowTypeWindow;
            m_npWindow.window = 0;
            return false;
        }

    m_needsXEmbed = false;
    m_plugin->pluginFuncs()->getvalue(m_instance, NPPVpluginNeedsXEmbed, &m_needsXEmbed);

    if(m_needsXEmbed)
            setPlatformPluginWidget(gtk_socket_new());
    else
        setPlatformPluginWidget(gtk_layout_new(NULL, NULL));

        gtk_container_add(container, platformPluginWidget());

    if(m_needsXEmbed)
    {
            // When a plug is removed/destroyed, its corresponding socket is then destroyed.
            // Another case is when we try to destroy the socket first. That results in
            // destruction of the plug also which in turn again tries to destroy the parent
            // socket. This way, the socket is tried to destroy twice - resulting into
            // random crashes. To avoid this, we handle the plug_removed signal here in
            // which do not do anything (by returning true) thereby preventing the second time
            // destruction of the socket.
            g_signal_connect(platformPluginWidget(), "plug_removed", G_CALLBACK(plug_removed_cb), NULL);
    }

        gtk_widget_realize(platformPluginWidget());
        if (isVisible() && GTK_IS_WIDGET(platformPluginWidget()))
            gtk_widget_show(platformPluginWidget());

        g_object_set_data(G_OBJECT(platformPluginWidget()), "kWebPluginViewProperty", this);

        m_npWindow.type = NPWindowTypeWindow;

        GdkWindow *gdkWindow = ((GtkWidget*)container)->window;

        if(!m_needsXEmbed)
        {
           if(!m_gtkXtBin)
        {
                m_gtkXtBin = gtk_xtbin_new(platformPluginWidget()->window, 0);
        if(!m_gtkXtBin || !m_gtkXtBin->window)
            return false;
            }
           gtk_widget_set_usize(m_gtkXtBin, m_npWindow.width, m_npWindow.height);
           gtk_widget_show(m_gtkXtBin);
        }

        // And now point the NPWindow structures window to the actual X window
    if(m_needsXEmbed)
            m_npWindow.window = (void*)gtk_socket_get_id(GTK_SOCKET(platformPluginWidget()));
    else
            m_npWindow.window = (void*)GTK_XTBIN(m_gtkXtBin)->xtwindow;

        if(!m_npWindow.width || !m_npWindow.height)
            return false;

#else
        GtkContainer *container = GTK_CONTAINER(m_parentFrame->view()->hostWindow()->platformPageClient());
        if(!container)
           return false;

        setPlatformPluginWidget(gtk_window_new(GTK_WINDOW_TOPLEVEL));

        gtk_widget_set_parent_window(platformPluginWidget(), (container->widget).window);

        if (m_isVisible)
            gtk_window_present(GTK_WINDOW(platformPluginWidget()));

        g_object_set_data(G_OBJECT(platformPluginWidget()), "kWebPluginViewProperty", this);

        m_npWindow.type = NPWindowTypeWindow;
        m_npWindow.window = (void*)GDK_WINDOW_XWINDOW(platformPluginWidget()->window);
#endif

    } else {

    m_window = 0;

        m_npWindow.type = NPWindowTypeDrawable;
        m_npWindow.window = 0;

        m_npWindow.ws_info = (NPSetWindowCallbackStruct *)malloc(sizeof(NPSetWindowCallbackStruct));
        NPSetWindowCallbackStruct *ws = (NPSetWindowCallbackStruct *)m_npWindow.ws_info;

        // fill in window info structure
        GtkWidget *w = m_parentFrame->view()->hostWindow()->platformPageClient();
    if(w)
        {
            GdkWindow *gdkWindow = w->window;
            ws->type = 0;
            ws->depth = gdk_window_get_visual(gdkWindow)->depth;
            ws->display = GDK_WINDOW_XDISPLAY(gdkWindow);
            ws->visual = GDK_VISUAL_XVISUAL(gdk_window_get_visual(gdkWindow));
            ws->colormap = GDK_COLORMAP_XCOLORMAP(gdk_window_get_colormap(gdkWindow));
         }
         else
         {
            ws->type = 0;
            ws->depth = 24;
            ws->display = XOpenDisplay(0);
            ws->visual = DefaultVisual(ws->display, DefaultScreen(ws->display));
            ws->colormap = DefaultColormap(ws->display, DefaultScreen(ws->display));
         }
    }

    if (!m_plugin->quirks().contains(PluginQuirkDeferFirstSetWindowCall))
        setNPWindowRect(frameRect()); 
        
    return true; 
}

void PluginView::platformDestroy()
{
    if (platformPluginWidget() && GTK_IS_WIDGET(platformPluginWidget()))
        gtk_widget_destroy(platformPluginWidget());

    // ach: fix this
    setPlatformWidget(0);
}

void PluginView::halt()
{
}

void PluginView::restart()
{
}

bool PluginView::platformGetValue(NPNVariable variable, void* value, NPError* result)
{
    switch (variable) {
        case NPNVSupportsXEmbedBool: {
            *(NPBool*)value = true;
            *result =  NPERR_NO_ERROR;
            return true;
        }

        case NPNVSupportsWindowless: {
            *(unsigned int*)value = 1;  /* true */
            *result =  NPERR_NO_ERROR;
            return true;
        }

        case NPNVToolkit: {
            *(unsigned int*)value = 2; /* NPNVGtk2 */
            *result =  NPERR_NO_ERROR;
            return true;
        }

        default:
            return false;
    }  
}

#if defined(XP_UNIX)
void PluginView::handleFocusInEvent()
{
//     XEvent npEvent;
//     initXEvent(&npEvent);

//     XFocusChangeEvent& event = npEvent.xfocus;
//     event.type = 9; // FocusIn gets unset somewhere
//     event.mode = NotifyNormal;
//     event.detail = NotifyDetailNone;

//     dispatchNPEvent(npEvent);
}

void PluginView::handleFocusOutEvent()
{
    // XEvent npEvent;
    // initXEvent(&npEvent);

    // XFocusChangeEvent& event = npEvent.xfocus;
    // event.type = 10; // FocusOut gets unset somewhere
    // event.mode = NotifyNormal;
    // event.detail = NotifyDetailNone;

    // dispatchNPEvent(npEvent);
}
#endif

} // namespace WebCore
