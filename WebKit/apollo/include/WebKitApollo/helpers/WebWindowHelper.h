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
#ifndef WebWindowHelper_h
#define WebWindowHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo {
    template <class ImplClass>
    class WebWindowHelper : private WebWindow {

    public:
    
        inline WebWindow* getWebWindow() { return this; }

    protected:
        WebWindowHelper();
        virtual ~WebWindowHelper();

    private:
        static WebWindowVTable const s_VTable;
        static ImplClass* getThis(WebWindow* const pWebWindow) { return static_cast<ImplClass*>(pWebWindow); }
        static const ImplClass* getThis(const WebWindow* const pWebWindow) { return static_cast<const ImplClass*>(pWebWindow); }

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sInvalidateRect(struct WebWindow* pWindow, const struct WebIntRect* pRect, unsigned char bNow);

        static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sVisibleWidth(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sVisibleHeight(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetContentsPos(struct WebWindow* pWindow, int newX, int newY);

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sResizeContents(struct WebWindow* pWindow, int newW, int newH);

        static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sContentsX(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sContentsY(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sContentsWidth(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 int WEBKIT_APOLLO_PROTO2 sContentsHeight(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sViewportToContents(const struct WebWindow* pWindow, const struct WebIntPoint* pViewportPoint, struct WebIntPoint* pContentPoint);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sContentsToViewport(const struct WebWindow* pWindow, const struct WebIntPoint* pContentPoint, struct WebIntPoint* pViewportPoint);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sViewportToWindow(const struct WebWindow* pWindow, const struct WebIntPoint* pViewportPoint, struct WebIntPoint* pWindowPoint);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sWindowToViewport(const struct WebWindow* pWindow, const struct WebIntPoint* pWindowPoint, struct WebIntPoint* pViewportPoint);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sScrollOffset(const struct WebWindow* pWindow, int* pXOffset, int* pYOffset);
        static WEBKIT_APOLLO_PROTO1 bool WEBKIT_APOLLO_PROTO2 sScrollBy(struct WebWindow* pWindow, int dx, int dy);
        static WEBKIT_APOLLO_PROTO1 WebWindowScrollbarMode WEBKIT_APOLLO_PROTO2 sHScrollbarMode(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 WebWindowScrollbarMode WEBKIT_APOLLO_PROTO2 sVScrollbarMode(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSuppressScrollbars(struct WebWindow* pWindow, unsigned char suppressed, unsigned char repaintOnSuppress);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetHScrollbarMode(struct WebWindow* pWindow, WebWindowScrollbarMode newMode);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetVScrollbarMode(struct WebWindow* pWindow, WebWindowScrollbarMode newMode);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetStaticBackground(struct WebWindow* pWindow, unsigned char hasStaticBackground);
        static WEBKIT_APOLLO_PROTO1 struct WebBitmap* WEBKIT_APOLLO_PROTO2 sGetBitmapSurface(const struct WebWindow* pWindow);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetWindowRect(const struct WebWindow* pWindow, struct WebFloatRect* pWindowRect);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetWindowRect(struct WebWindow* pWindow, const struct WebFloatRect* pWindowRect);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetPageRect(const struct WebWindow* pWindow, struct WebFloatRect* pPageRect);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sFocus( struct WebWindow* pWindow );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sUnfocus( struct WebWindow* pWindow );
		static WEBKIT_APOLLO_PROTO1 WebPlatformWindow WEBKIT_APOLLO_PROTO2 sGetPlatformWindow( struct WebWindow* pWindow );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sTakeFocusFromWebKit( struct WebWindow* pWindow, const int direction );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sCanTakeFocusFromWebKit( struct WebWindow* pWindow );
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sProcessingUserGesture( const struct WebWindow* );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sStartWindowedPluginUserGesture( struct WebWindow* );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sEndWindowedPluginUserGesture( struct WebWindow* );
#if defined(TARGET_OS_MAC) && ! defined(NP_NO_CARBON)
		static WEBKIT_APOLLO_PROTO1 WebPlatformWindowRef WEBKIT_APOLLO_PROTO2 sGetPlatformWindowRef( struct WebWindow* pWindow );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetPlatformWindowGeometry( struct WebWindow* pWindow, struct WebFloatRect* pFrameRect, struct WebFloatRect* pContentRect);
#endif
    };

    template <class ImplClass>
    WebWindowVTable const WebWindowHelper<ImplClass>::s_VTable = {
        sizeof(WebWindowVTable),
        WebWindowHelper<ImplClass>::sInvalidateRect,
        WebWindowHelper<ImplClass>::sVisibleWidth,
        WebWindowHelper<ImplClass>::sVisibleHeight,
        WebWindowHelper<ImplClass>::sSetContentsPos,
        WebWindowHelper<ImplClass>::sResizeContents,
        WebWindowHelper<ImplClass>::sContentsX,
        WebWindowHelper<ImplClass>::sContentsY,
        WebWindowHelper<ImplClass>::sContentsWidth,
        WebWindowHelper<ImplClass>::sContentsHeight,
        WebWindowHelper<ImplClass>::sViewportToContents,
        WebWindowHelper<ImplClass>::sContentsToViewport,
        WebWindowHelper<ImplClass>::sViewportToWindow,
        WebWindowHelper<ImplClass>::sWindowToViewport,
        WebWindowHelper<ImplClass>::sScrollOffset,
        WebWindowHelper<ImplClass>::sScrollBy,
        WebWindowHelper<ImplClass>::sHScrollbarMode,
        WebWindowHelper<ImplClass>::sVScrollbarMode,
        WebWindowHelper<ImplClass>::sSuppressScrollbars,
        WebWindowHelper<ImplClass>::sSetHScrollbarMode,
        WebWindowHelper<ImplClass>::sSetVScrollbarMode,
        WebWindowHelper<ImplClass>::sSetStaticBackground,
        WebWindowHelper<ImplClass>::sGetBitmapSurface,
        WebWindowHelper<ImplClass>::sGetWindowRect,
        WebWindowHelper<ImplClass>::sSetWindowRect,
        WebWindowHelper<ImplClass>::sGetPageRect,
        WebWindowHelper<ImplClass>::sFocus,
        WebWindowHelper<ImplClass>::sUnfocus,
		WebWindowHelper<ImplClass>::sGetPlatformWindow,
        WebWindowHelper<ImplClass>::sTakeFocusFromWebKit,
        WebWindowHelper<ImplClass>::sCanTakeFocusFromWebKit,
        WebWindowHelper<ImplClass>::sProcessingUserGesture,
        WebWindowHelper<ImplClass>::sStartWindowedPluginUserGesture,
        WebWindowHelper<ImplClass>::sEndWindowedPluginUserGesture
#if defined(TARGET_OS_MAC) && ! defined(NP_NO_CARBON)
		,WebWindowHelper<ImplClass>::sGetPlatformWindowRef
		,WebWindowHelper<ImplClass>::sGetPlatformWindowGeometry
#endif
    };
    
    template <class ImplClass>
    WebWindowHelper<ImplClass>::WebWindowHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebWindowHelper<ImplClass>::~WebWindowHelper()
    {
        m_pVTable = 0;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sInvalidateRect(struct WebWindow* pWindow, const struct WebIntRect* pRect, unsigned char bNow)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->invalidateRect(pRect, bNow != 0);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    int WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sVisibleWidth(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->visibleWidth();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    int WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sVisibleHeight(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->visibleHeight();
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sSetContentsPos(struct WebWindow* pWindow, int newX, int newY)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->setContentsPos(newX, newY);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sResizeContents(struct WebWindow* pWindow, int newW, int newH)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->resizeContents(newW, newH);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    int WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sContentsX(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->contentsX();
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    int WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sContentsY(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->contentsY();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    int WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sContentsWidth(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->contentsWidth();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    int WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sContentsHeight(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->contentsHeight();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sViewportToContents(const struct WebWindow* pWindow,  const struct WebIntPoint* pViewportPoint, struct WebIntPoint* pContentPoint)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->viewportToContents(pViewportPoint, pContentPoint);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sContentsToViewport(const struct WebWindow* pWindow, const struct WebIntPoint* pContentPoint, struct WebIntPoint* pViewportPoint)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->contentsToViewport(pContentPoint, pViewportPoint);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sViewportToWindow(const struct WebWindow* pWindow, const struct WebIntPoint* pViewportPoint, struct WebIntPoint* pWindowPoint)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->viewportToWindow(pViewportPoint, pWindowPoint);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sWindowToViewport(const struct WebWindow* pWindow, const struct WebIntPoint* pWindowPoint, struct WebIntPoint* pViewportPoint)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->windowToViewport(pWindowPoint, pViewportPoint);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sScrollOffset(const struct WebWindow* pWindow, int* pXOffset, int* pYOffset)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->scrollOffset(pXOffset, pYOffset);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    bool WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sScrollBy(struct WebWindow* pWindow, int dx, int dy)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return false;
        return pThis->scrollBy(dx, dy);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    WebWindowScrollbarMode  WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sHScrollbarMode(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return WebWindowScrollbarModeAuto;
        return pThis->hScrollbarMode();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    WebWindowScrollbarMode  WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sVScrollbarMode(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return WebWindowScrollbarModeAuto;
        return pThis->vScrollbarMode();
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sSuppressScrollbars(struct WebWindow* pWindow, unsigned char suppressed, unsigned char repaintOnSuppress)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->suppressScrollbars(suppressed != 0, repaintOnSuppress != 0);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sSetHScrollbarMode(struct WebWindow* pWindow, WebWindowScrollbarMode newMode)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->setHScrollbarMode(newMode);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sSetVScrollbarMode(struct WebWindow* pWindow, WebWindowScrollbarMode newMode)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->setVScrollbarMode(newMode);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sSetStaticBackground(struct WebWindow* pWindow, unsigned char hasStaticBackground)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->setStaticBackground(hasStaticBackground != 0);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    WebBitmap* WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sGetBitmapSurface(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->getBitmapSurface();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sGetWindowRect(const struct WebWindow* pWindow, struct WebFloatRect* pWindowRect)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->getWindowRect(pWindowRect);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sSetWindowRect(struct WebWindow* pWindow, const struct WebFloatRect* pWindowRect)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->setWindowRect(pWindowRect);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sGetPageRect(const struct WebWindow* pWindow, struct WebFloatRect* pPageRect)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->getPageRect(pPageRect);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sFocus(struct WebWindow* pWindow)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->focus();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sUnfocus(struct WebWindow* pWindow)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->unfocus();
    }
                                                                            
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    WebPlatformWindow WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sGetPlatformWindow(struct WebWindow* pWindow)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        return pThis->getPlatformWindow();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sTakeFocusFromWebKit(struct WebWindow* pWindow, const int direction)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->takeFocusFromWebKit(direction);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sCanTakeFocusFromWebKit(struct WebWindow* pWindow)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        bool const boolResult = pThis->canTakeFocusFromWebKit();
        unsigned char const result = boolResult ? 1 : 0;
        return result;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sProcessingUserGesture(const struct WebWindow* pWindow)
    {
        const ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return 0;
        bool const boolResult = pThis->processingUserGesture();
        unsigned char const result = boolResult ? 1 : 0;
        return result;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sStartWindowedPluginUserGesture(struct WebWindow* pWindow)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->startWindowedPluginUserGesture();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sEndWindowedPluginUserGesture(struct WebWindow* pWindow)
    {
        ImplClass* const pThis = getThis(pWindow);
        if (pThis->isUnusable()) return;
        pThis->endWindowedPluginUserGesture();
    }

#if defined(TARGET_OS_MAC) && ! defined(NP_NO_CARBON)
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebPlatformWindowRef WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sGetPlatformWindowRef(struct WebWindow* pWindow)
	{
		ImplClass* const pThis = getThis(pWindow);
		if (pThis->isUnusable()) return 0;
		return pThis->getPlatformWindowRef();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebWindowHelper<ImplClass>::sGetPlatformWindowGeometry(struct WebWindow* pWindow, struct WebFloatRect* pFrameRect, struct WebFloatRect* pContentRect)
	{
		ImplClass* const pThis = getThis(pWindow);
		if (!pThis->isUnusable())
		{
			pThis->getPlatformWindowGeometry(pFrameRect, pContentRect);
		}
	}
#endif

}


#endif
