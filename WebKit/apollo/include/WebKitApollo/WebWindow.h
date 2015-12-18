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
#ifndef WebWindow_h
#define WebWindow_h

#include <WebKitApollo/WebConfig.h>

struct WebWindow;
struct WebWindowVTable;
struct WebIntRect;
struct WebFloatRect;
struct WebIntPoint;

enum WebWindowScrollbarMode {
    WebWindowScrollbarModeAuto = 0,
    WebWindowScrollbarModeAlwaysOff,
    WebWindowScrollbarModeAlwaysOn,
    // ----------------- New Modes go above this line --------------
    WebWindowScrollbarModeLast
};

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowInvalidateRectFunction )( struct WebWindow* pWindow
                                                                                           , const struct WebIntRect* pRect
                                                                                           , unsigned char bNow );

typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebWindowVisibleWidthFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebWindowVisibleHeightFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowSetContentsPosFunction )( struct WebWindow* pWindow
                                                                                           , int newX
                                                                                           , int newY );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowResizeContentsFunction )( struct WebWindow* pWindow
                                                                                           , int newW
                                                                                           , int newH );

typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebWindowContentsXFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebWindowContentsYFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebWindowContentsWidthFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebWindowContentsHeightFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowViewportToContentsFunction )( const struct WebWindow* pWindow
                                                                                               , const struct WebIntPoint* pViewportPoint
                                                                                               , struct WebIntPoint* pContentPoint );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowContentsToViewportFunction )( const struct WebWindow* pWindow
                                                                                               , const struct WebIntPoint* pContentPoint
                                                                                               , struct WebIntPoint* pViewportPoint );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowViewportToWindowFunction )( const struct WebWindow* pWindow
                                                                                             , const struct WebIntPoint* pViewportPoint
                                                                                             , struct WebIntPoint* pWindowPoint );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowWindowToViewportFunction )( const struct WebWindow* pWindow
                                                                                                   , const struct WebIntPoint* pWindowPoint
                                                                                                   , struct WebIntPoint* pViewportPoint );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowScrollOffsetFunction )( const struct WebWindow* pWindow
                                                                                         , int* pXOffset
                                                                                         , int* pYOffset );
typedef WEBKIT_APOLLO_PROTO1 bool ( WEBKIT_APOLLO_PROTO2 *WebWindowScrollByFunction )( struct WebWindow* pWindow
                                                                                     , int dx
                                                                                     , int dy );
typedef WEBKIT_APOLLO_PROTO1 WebWindowScrollbarMode ( WEBKIT_APOLLO_PROTO2 *WebWindowHScrollbarModeFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 WebWindowScrollbarMode ( WEBKIT_APOLLO_PROTO2 *WebWindowVScrollbarModeFunction )( const struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowSuppressScrollbarsFunction )( struct WebWindow* pWindow
                                                                                               , unsigned char suppressed
                                                                                               , unsigned char repaintOnUnSuppress );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowSetHScrollbarModeFunction )( struct WebWindow* pWindow
                                                                                              , WebWindowScrollbarMode newMode );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowSetVScrollbarModeFunction )( struct WebWindow* pWindow
                                                                                              , WebWindowScrollbarMode newMode );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowSetStaticBackgroundFunction )( struct WebWindow* pWindow
                                                                                                , unsigned char hasStaticBackground );
typedef WEBKIT_APOLLO_PROTO1 struct WebBitmap* ( WEBKIT_APOLLO_PROTO2 *WebWindowGetBitmapSurfaceFunction )( const struct WebWindow* pWindow );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowGetWindowRectFunction )( const struct WebWindow* pWindow
                                                                                          , struct WebFloatRect* pWindowRect );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowSetWindowRectFunction )( struct WebWindow* pWindow
                                                                                          , const struct WebFloatRect* pWindowRect );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowGetPageRectFunction )( const struct WebWindow* pWindow
                                                                                        , struct WebFloatRect* pPageRect );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowFocusFunction )( struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowUnfocusFunction )( struct WebWindow* pWindow );

typedef WEBKIT_APOLLO_PROTO1 WebPlatformWindow ( WEBKIT_APOLLO_PROTO2 *WebWindowGetPlatformWindowFunction )( struct WebWindow* pWindow );
#if defined(TARGET_OS_MAC) && ! defined(NP_NO_CARBON)
typedef WEBKIT_APOLLO_PROTO1 WebPlatformWindowRef ( WEBKIT_APOLLO_PROTO2 *WebWindowGetPlatformWindowRefFunction )( struct WebWindow* pWindow );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowGetPlatformWindowGeometryFunction )( struct WebWindow* pWindow
																								  , struct WebFloatRect* pFrameRect
																								  , struct WebFloatRect* pContentRect);
#endif

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebWindowTakeFocusFromWebKitFunction )( struct WebWindow* pWindow, const int direction );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebWindowCanTakeFocusFromWebKitFunction )( struct WebWindow* pWindow );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebProcessingUserGestureFunction )( const struct WebWindow* );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebStartWindowedPluginUserGestureFunction )( struct WebWindow* );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebEndWindowedPluginUserGestureFunction )( struct WebWindow* );

struct WebWindowVTable {
    unsigned long m_vTableSize;
    WebWindowInvalidateRectFunction invalidateRect;
    WebWindowVisibleWidthFunction visibleWidth;
    WebWindowVisibleHeightFunction visibleHeight;
    WebWindowSetContentsPosFunction setContentsPos;
    WebWindowResizeContentsFunction resizeContents;
    WebWindowContentsXFunction contentsX;
    WebWindowContentsYFunction contentsY;
    WebWindowContentsWidthFunction contentsWidth;
    WebWindowContentsHeightFunction contentsHeight;
    WebWindowViewportToContentsFunction viewportToContents;
    WebWindowContentsToViewportFunction contentsToViewport;
    WebWindowViewportToWindowFunction viewportToWindow;
    WebWindowWindowToViewportFunction windowToViewport;
    WebWindowScrollOffsetFunction scrollOffset;
    WebWindowScrollByFunction scrollBy;
    WebWindowHScrollbarModeFunction hScrollbarMode;
    WebWindowVScrollbarModeFunction vScrollbarMode;
    WebWindowSuppressScrollbarsFunction suppressScrollbars;
    WebWindowSetHScrollbarModeFunction setHScrollbarMode;
    WebWindowSetVScrollbarModeFunction setVScrollbarMode;
    WebWindowSetStaticBackgroundFunction setStaticBackground;
    WebWindowGetBitmapSurfaceFunction getBitmapSurface;
    WebWindowGetWindowRectFunction getWindowRect;
    WebWindowSetWindowRectFunction setWindowRect;
    WebWindowGetPageRectFunction getPageRect;
	WebWindowFocusFunction focus;
	WebWindowUnfocusFunction unfocus;
	WebWindowGetPlatformWindowFunction getPlatformWindow;
    WebWindowTakeFocusFromWebKitFunction takeFocusFromWebKit;
    WebWindowCanTakeFocusFromWebKitFunction canTakeFocusFromWebKit;
    WebProcessingUserGestureFunction processingUserGesture;
    WebStartWindowedPluginUserGestureFunction startProcessingWindowedPluginUserGesture;
    WebEndWindowedPluginUserGestureFunction endProcessingWindowedPluginUserGesture;
#if defined(TARGET_OS_MAC) && ! defined(NP_NO_CARBON)
	WebWindowGetPlatformWindowRefFunction getPlatformWindowRef;
	WebWindowGetPlatformWindowGeometryFunction getPlatformWindowGeometry;
#endif
};

struct WebWindow {
    const WebWindowVTable* m_pVTable;
};

#endif
