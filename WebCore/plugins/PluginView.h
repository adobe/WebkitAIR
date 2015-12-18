/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
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

#ifndef PluginView_h
#define PluginView_h

#include "FrameLoadRequest.h"
#include "HaltablePlugin.h"
#include "IntRect.h"
#include "MediaCanStartListener.h"
#include "PluginStream.h"
#include "ResourceRequest.h"
#include "Timer.h"
#include "Widget.h"
#include "npruntime_internal.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/CString.h>

#if OS(WINDOWS) && (PLATFORM(QT) || PLATFORM(WX))
typedef struct HWND__* HWND;
typedef HWND PlatformPluginWidget;
#else
typedef PlatformWidget PlatformPluginWidget;
#if defined(XP_MACOSX) && PLATFORM(QT)
#include <QPixmap>
#endif
#endif

namespace JSC {
    namespace Bindings {
        class Instance;
    }
}

namespace WebCore {
    class Element;
    class Frame;
    class Image;
    class KeyboardEvent;
    class MouseEvent;
    class WheelEvent;
    class KURL;
#if OS(WINDOWS) && ENABLE(NETSCAPE_PLUGIN_API)
    class PluginMessageThrottlerWin;
#endif
    class PluginPackage;
    class PluginRequest;
    class PluginStream;
    class ResourceError;
    class ResourceResponse;

    enum PluginStatus {
        PluginStatusCanNotFindPlugin,
        PluginStatusCanNotLoadPlugin,
        PluginStatusLoadedSuccessfully
    };

    class PluginRequest : public Noncopyable {
    public:
        PluginRequest(const FrameLoadRequest& frameLoadRequest, bool sendNotification, void* notifyData, bool shouldAllowPopups)
            : m_frameLoadRequest(frameLoadRequest)
            , m_notifyData(notifyData)
            , m_sendNotification(sendNotification)
            , m_shouldAllowPopups(shouldAllowPopups) { }
    public:
        const FrameLoadRequest& frameLoadRequest() const { return m_frameLoadRequest; }
        void* notifyData() const { return m_notifyData; }
        bool sendNotification() const { return m_sendNotification; }
        bool shouldAllowPopups() const { return m_shouldAllowPopups; }
    private:
        FrameLoadRequest m_frameLoadRequest;
        void* m_notifyData;
        bool m_sendNotification;
        bool m_shouldAllowPopups;
    };

    class PluginManualLoader {
    public:
        virtual ~PluginManualLoader() {}
        virtual void didReceiveResponse(const ResourceResponse&) = 0;
        virtual void didReceiveData(const char*, int) = 0;
        virtual void didFinishLoading() = 0;
        virtual void didFail(const ResourceError&) = 0;
    };

    class PluginView : public Widget, private PluginStreamClient, public PluginManualLoader, private HaltablePlugin, private MediaCanStartListener {
    public:
        static PassRefPtr<PluginView> create(Frame* parentFrame, const IntSize&, Element*, const KURL&, const Vector<String>& paramNames, const Vector<String>& paramValues, const String& mimeType, bool loadManually);
        virtual ~PluginView();


        PluginPackage* plugin() const { return m_plugin.get(); }
        NPP instance() const { return m_instance; }

        void setNPWindowRect(const IntRect&);
        static PluginView* currentPluginView();

        PassRefPtr<JSC::Bindings::Instance> bindingInstance();

        PluginStatus status() const { return m_status; }

        // NPN functions
        NPError getURLNotify(const char* url, const char* target, void* notifyData);
        NPError getURL(const char* url, const char* target);
        NPError postURLNotify(const char* url, const char* target, uint32_t len, const char* but, NPBool file, void* notifyData);
        NPError postURL(const char* url, const char* target, uint32_t len, const char* but, NPBool file);
        NPError newStream(NPMIMEType type, const char* target, NPStream** stream);
        int32_t write(NPStream* stream, int32_t len, void* buffer);
        NPError destroyStream(NPStream* stream, NPReason reason);
        const char* userAgent();
#if ENABLE(NETSCAPE_PLUGIN_API)
        static const char* userAgentStatic();
#endif
        void status(const char* message);
        
#if ENABLE(NETSCAPE_PLUGIN_API)
        NPError getValue(NPNVariable variable, void* value);
        static NPError getValueStatic(NPNVariable variable, void* value);
#endif
        NPError setValue(NPPVariable variable, void* value);
        void invalidateRect(NPRect*);
        void invalidateRegion(NPRegion);
        void forceRedraw();
        void pushPopupsEnabledState(bool state);
        void popPopupsEnabledState();

        virtual void invalidateRect(const IntRect&);

#if OS(DARWIN)
        NPBool convertPoint(double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace);
#endif
        
        bool arePopupsAllowed() const;

        void setJavaScriptPaused(bool);

        void privateBrowsingStateChanged(bool);

        void disconnectStream(PluginStream*);
        void streamDidFinishLoading(PluginStream* stream) { disconnectStream(stream); }

        // Widget functions
        virtual void setFrameRect(const IntRect&);
        virtual void frameRectsChanged();
        virtual void setFocus(bool);
        virtual void show();
        virtual void hide();
        virtual void paint(GraphicsContext*, const IntRect&);

        // This method is used by plugins on all platforms to obtain a clip rect that includes clips set by WebCore,
        // e.g., in overflow:auto sections.  The clip rects coordinates are in the containing window's coordinate space.
        // This clip includes any clips that the widget itself sets up for its children.
        IntRect windowClipRect() const;

        virtual void handleEvent(Event*);
        virtual void setParent(ScrollView*);
        virtual void setParentVisible(bool);

        virtual bool isPluginView() const { return true; }

        Frame* parentFrame() const { return m_parentFrame.get(); }

#if PLATFORM(APOLLO) && ENABLE(NETSCAPE_PLUGIN_API)
        void beginModal();
        void endModal();
        void updatePluginWindow(bool canShowPlugins, bool canShowWindowedPlugins);
        void removePlugin();
		bool enterPlayer(WebHostEnterPlayerCallback callback, void* arg);
		// handle copy,cut,selectAll
		bool handleOnCopy();
		bool handleOnCut();
		bool handleOnPaste();
		bool handleSelectAll();
        
#if OS(DARWIN)
        bool handleInsertText(const String& text);
#endif
        
#if PLATFORM(APOLLO_WINDOWS)
    private:
        HWND m_parentWindow;
        void updatePluginWindow();
		void createPluginWindow();
	public:
#endif
#endif

        void focusPluginElement();

        const String& pluginsPage() const { return m_pluginsPage; }
        const String& mimeType() const { return m_mimeType; }
        const KURL& url() const { return m_url; }

#if OS(WINDOWS) && ENABLE(NETSCAPE_PLUGIN_API)
        static LRESULT CALLBACK PluginViewWndProc(HWND, UINT, WPARAM, LPARAM);
        LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        WNDPROC pluginWndProc() const { return m_pluginWndProc; }
#endif

#if PLATFORM(APOLLO_WINDOWS)
        bool m_isIMECompositionActive;
#endif // PLATFORM(APOLLO)

        // Used for manual loading
        void didReceiveResponse(const ResourceResponse&);
        void didReceiveData(const char*, int);
        void didFinishLoading();
        void didFail(const ResourceError&);

        // HaltablePlugin
        virtual void halt();
        virtual void restart();
        virtual Node* node() const;
        virtual bool isWindowed() const { return m_isWindowed; }
        virtual String pluginName() const;

        bool isHalted() const { return m_isHalted; }
        bool hasBeenHalted() const { return m_hasBeenHalted; }

        static bool isCallingPlugin();

        bool start();


        static void keepAlive(NPP);

        void keepAlive();

#if PLATFORM(APOLLO)
        void startUserGesture();

        // returns the real window rect, not the stage/client rect
        IntPoint viewportToWindow(const IntPoint &p) const;
        IntPoint clientToWindow(const IntPoint &p) const;

        bool isEvilFlashJSString(String const jsString) const;
        void userGestureTimerFired(Timer<PluginView>*);

#if OS(DARWIN) 
        #if ! defined(NP_NO_CARBON)
        // these should all be moved as non member functions in PluginViewMac.cpp
		IntRect getCarbonRectForNSWindowRect(const IntRect& rect) const;
        IntRect getGeometryInCarbonWindow() const;
        IntPoint getOriginOfHTMLControlInWebWindow() const;
        IntPoint getOriginOfHTMLControlInCarbonWindow() const;
        WindowRef getCarbonWindow() const;
        void getCarbonMousePosition(::Point* const mousePoint);
        
#ifndef NP_NO_QUICKDRAW
        Timer<PluginView> m_nullEventTimer;
#endif
        void nullEventTimerFired(Timer<PluginView>*);
        #endif // ! defined(NP_NO_CARBON)

        IntRect clipDirtyRect(const IntRect& dirtyRect) const;
#endif // OS(DARWIN)

        Timer<PluginView> m_userGestureTimer;
        bool m_isEnabled;
        bool m_doingUserGesture;
#endif // PLATFORM(APOLLO)

    private:
        PluginView(Frame* parentFrame, const IntSize&, PluginPackage*, Element*, const KURL&, const Vector<String>& paramNames, const Vector<String>& paramValues, const String& mimeType, bool loadManually);

        void setParameters(const Vector<String>& paramNames, const Vector<String>& paramValues);
        bool startOrAddToUnstartedList();
        void init();
        bool platformStart();
        void stop();
        void platformDestroy();
		void removeFromUnstartedListIfNecessary();
        static void setCurrentPluginView(PluginView*);
        NPError load(const FrameLoadRequest&, bool sendNotification, void* notifyData);
        NPError handlePost(const char* url, const char* target, uint32_t len, const char* buf, bool file, void* notifyData, bool sendNotification, bool allowHeaders);
        NPError handlePostReadFile(Vector<char>& buffer, uint32_t len, const char* buf);
        static void freeStringArray(char** stringArray, int length);
        void setCallingPlugin(bool) const;

        void invalidateWindowlessPluginRect(const IntRect&);

        virtual void mediaCanStart();

#if OS(WINDOWS) && ENABLE(NETSCAPE_PLUGIN_API)
        void paintWindowedPluginIntoContext(GraphicsContext*, const IntRect&);
        static HDC WINAPI hookedBeginPaint(HWND, PAINTSTRUCT*);
        static BOOL WINAPI hookedEndPaint(HWND, const PAINTSTRUCT*);
#endif

        static bool platformGetValueStatic(NPNVariable variable, void* value, NPError* result);
        bool platformGetValue(NPNVariable variable, void* value, NPError* result);

        RefPtr<Frame> m_parentFrame;
        RefPtr<PluginPackage> m_plugin;
        Element* m_element;
        bool m_isStarted;
        KURL m_url;
        KURL m_baseURL;
        PluginStatus m_status;
        Vector<IntRect> m_invalidRects;

        void performRequest(PluginRequest*);
        void scheduleRequest(PluginRequest*);
        void requestTimerFired(Timer<PluginView>*);
        void invalidateTimerFired(Timer<PluginView>*);
        Timer<PluginView> m_requestTimer;
        Timer<PluginView> m_invalidateTimer;

        void popPopupsStateTimerFired(Timer<PluginView>*);
        Timer<PluginView> m_popPopupsStateTimer;

        void lifeSupportTimerFired(Timer<PluginView>*);
        Timer<PluginView> m_lifeSupportTimer;

#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
        bool dispatchNPEvent(NPEvent&);
#endif
        void updatePluginWidget();
        void paintMissingPluginIcon(GraphicsContext*, const IntRect&);

        void handleKeyboardEvent(KeyboardEvent*);
        void handleMouseEvent(MouseEvent*);
#if PLATFORM(APOLLO) && defined(NP_NO_CARBON)
        void handleWheelEvent(WheelEvent*);
#endif
        
#if defined(XP_UNIX)
        void handleFocusInEvent();
        void handleFocusOutEvent();
#endif

#if OS(WINDOWS)
        void paintIntoTransformedContext(HDC);
#if PLATFORM(APOLLO)
        void paintIntoTransformedContext(HDC, IntPoint&);
#endif
        PassRefPtr<Image> snapshot();
#endif

        int m_mode;
        int m_paramCount;
        char** m_paramNames;
        char** m_paramValues;
        String m_pluginsPage;

        String m_mimeType;
        WTF::CString m_userAgent;
        
        NPP m_instance;
        NPP_t m_instanceStruct;
        NPWindow m_npWindow;


#if PLATFORM(APOLLO_DARWIN)
        NPDrawingModel m_drawingModel;
        NPEventModel m_eventModel;

        // This is the context for rendring using Core Graphics
        NP_CGContext m_npCGContext;
        
#ifndef NP_NO_QUICKDRAW
        // The is the port for rendering using QuickDraw.
        NP_Port m_npPort;
#endif
        void* m_webBitmapPixels;
        unsigned long m_webBitmapWidth;
        unsigned long m_webBitmapHeight;
        unsigned long m_webBitmapStride;
		
#ifndef NP_NO_CARBON
		TSMDocumentID m_appTSMDocID;
#endif
#endif

        Vector<bool, 4> m_popupStateStack;

        HashSet<RefPtr<PluginStream> > m_streams;
        Vector<PluginRequest*> m_requests;

        bool m_isWindowed;
        bool m_isTransparent;
        bool m_haveInitialized;
        bool m_isWaitingToStart;

#if defined(XP_UNIX) || (defined(XP_UNIX) && PLATFORM(APOLLO))
        bool m_needsXEmbed;
#endif

#if OS(WINDOWS) && ENABLE(NETSCAPE_PLUGIN_API)
        OwnPtr<PluginMessageThrottlerWin> m_messageThrottler;
        WNDPROC m_pluginWndProc;
        unsigned m_lastMessage;
        bool m_isCallingPluginWndProc;
        HDC m_wmPrintHDC;
        bool m_haveUpdatedPluginWidget;
#endif

#if ((PLATFORM(QT) || PLATFORM(WX)) && OS(WINDOWS)) || defined(XP_MACOSX) || PLATFORM(APOLLO)
        // On Mac OSX and Qt/Windows the plugin does not have its own native widget,
        // but is using the containing window as its reference for positioning/painting.
        PlatformPluginWidget m_window;
public:
        PlatformPluginWidget platformPluginWidget() const { return m_window; }
        void setPlatformPluginWidget(PlatformPluginWidget widget) { m_window = widget; }
#else
public:
        void setPlatformPluginWidget(PlatformPluginWidget widget) { setPlatformWidget(widget); }
        PlatformPluginWidget platformPluginWidget() const { return platformWidget(); }
#endif

private:

#if defined(XP_UNIX) || OS(SYMBIAN)
        #define exists_setNPWindowIfNeeded
#elif defined(XP_MACOSX) && !PLATFORM(APOLLO)
        NP_CGContext m_npCgContext;
        OwnPtr<Timer<PluginView> > m_nullEventTimer;
        NPDrawingModel m_drawingModel;
        NPEventModel m_eventModel;
        CGContextRef m_contextRef;
        WindowRef m_fakeWindow;
#if PLATFORM(QT)
        QPixmap m_pixmap;
#endif

        Point m_lastMousePos;
        #define exists_setNPWindowIfNeeded
        void nullEventTimerFired(Timer<PluginView>*);
        Point globalMousePosForPlugin() const;
        Point mousePosForPlugin(MouseEvent* event = 0) const;
#endif

#ifdef exists_setNPWindowIfNeeded
        void setNPWindowIfNeeded();
#endif

#if defined(XP_UNIX)
        bool m_hasPendingGeometryChange;
        Pixmap m_drawable;
        Visual* m_visual;
        Colormap m_colormap;
        Display* m_pluginDisplay;

        void initXEvent(XEvent* event);
#endif

#if defined(XP_UNIX) && PLATFORM(APOLLO)
        GtkWidget *m_gtkXtBin;  // For supporting XT for acroread plugin
        GdkDrawable *m_windowlessDrawable;  // Reuse the drawable - this avoids NPN_InvalidateRect from FP
#endif

        IntRect m_clipRect; // The clip rect to apply to a windowed plug-in
        IntRect m_windowRect; // Our window rect.

        bool m_loadManually;
        RefPtr<PluginStream> m_manualStream;

        bool m_isJavaScriptPaused;

        bool m_isHalted;
        bool m_hasBeenHalted;

        bool m_haveCalledSetWindow;

        static PluginView* s_currentPluginView;

#if defined(XP_UNIX) 
	NPError PopulateWindowAndWSInfoForNPWindow();
#endif

    };

} // namespace WebCore

#endif 
