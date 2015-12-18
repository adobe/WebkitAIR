/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2011 Adobe Systems
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

#include "FrameLoadRequest.h"
#include "PlatformString.h"
#include "KURL.h"
#include "RenderLayer.h"
#include "Element.h"
#include "Document.h"
#include "FrameView.h"
#include "Image.h"
#include "GraphicsContext.h"
#include "KeyboardEvent.h"
#include "EventNames.h"
#include "PluginPackage.h"
#include "c_instance.h"
#include "MouseEvent.h"
#include "WheelEvent.h"
#include "PlatformMouseEvent.h"
#include "PluginDebug.h"
#include "PluginDatabase.h"
#include "FrameLoader.h"
#include "FrameTree.h"
#include "Settings.h"
#include "NotImplemented.h"
#include "Page.h"
#include "FocusController.h"
#include "npruntime_impl.h"
#include "HTMLNames.h"
#include "HTMLPluginElement.h"
#include "JSDOMWindow.h"
#include <DeprecatedCarbonFunctions/QDDeprecated.h>
#include "runtime_root.h"
#include "Cursor.h"
#include "FrameLoaderClientApollo.h"
#include "PluginCarbonWindowForNSWindow.h"
#include "PluginCarbonWindow.h"
#include "FocusController.h"
#include "PlatformKeyboardEvent.h"
#include "SelectionController.h"
#include "JSLock.h"
#include "ScriptController.h"
#include "Frame.h"
#include "PluginMainThreadScheduler.h"

#if PLATFORM(APOLLO)
#include "FrameLoaderClientApollo.h"

#include <WebKitApollo/WebKit.h>
#include <WebKitApollo/WebWindow.h>
#include <WebKitApollo/WebBitmap.h>
#include <WebKitApollo/WebHost.h>
namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions; }
#endif

#ifdef NP_NO_CARBON
/* Defined in NSEvent.h. So as to disturb the WebKit project the least and since those are just numeric constants, it's better than trying to add the Cocoa AppKit to the WebKit project (ugh!...) */ 
enum {
    NSAlphaShiftKeyMask         = 1 << 16,
    NSShiftKeyMask              = 1 << 17,
    NSControlKeyMask            = 1 << 18,
    NSAlternateKeyMask          = 1 << 19,
    NSCommandKeyMask            = 1 << 20,
};
#endif

namespace WebCore {
	
	
	namespace {
		
#ifndef NP_NO_QUICKDRAW
		static const int activeNullTimerPeriodInMilliSeconds = 20;
		static const int inActiveNullTimerPeriodInMilliSeconds = 250;
#endif
		
		
		template <typename T1, typename T2>
		inline void setAndUpdateDirty(bool& dirty, T1 const newValue, T2& dest) {
			if ((dirty) || (static_cast<T2>(newValue) != dest)) {
				dirty = true;
				dest = newValue;
			}
		}
#ifndef NP_NO_QUICKDRAW
		class CGrafPtrPusherPopper
		{
		public:
			inline CGrafPtrPusherPopper(CGrafPtr const newCGrafPtr)
			: m_savePort(0)
			{
				QDDeprecated::GetPort(&m_savePort);
				QDDeprecated::SetPort(newCGrafPtr);
			}
			
			inline ~CGrafPtrPusherPopper()
			{
				QDDeprecated::SetPort(m_savePort);
			}
			
		private:
			CGrafPtrPusherPopper(const CGrafPtrPusherPopper&);
			CGrafPtrPusherPopper& operator=(const CGrafPtrPusherPopper&);
			CGrafPtr m_savePort;
		};
#endif
	}
	
#if PLATFORM(APOLLO) && !defined(NP_NO_QUICKDRAW)
	IntRect PluginView::getCarbonRectForNSWindowRect(const IntRect& rect) const
	{
		WebWindow* const webWindow = getApolloWebWindow();
		if (webWindow)
		{
			WebFloatRect frameRect, contentRect;
			webWindow->m_pVTable->getPlatformWindowGeometry(webWindow, &frameRect, &contentRect);
			return IntRect(rect.x(), rect.y() + (frameRect.m_bottom - frameRect.m_top) - (contentRect.m_bottom - contentRect.m_top), rect.width(), rect.height());
		}
		else
		{
			return rect;
		}
	}
	
	IntRect PluginView::getGeometryInCarbonWindow() const
	{
        IntRect geometryInContent(frameRect());
		ASSERT(m_element);
		ASSERT(m_element->renderer());
		
		// Take our element and get the clip rect from the enclosing layer.
		RenderLayer* const layer = m_element->renderer()->enclosingLayer();
		ASSERT(layer);
		IntRect layerClipRect(layer->childrenClipRect());
		geometryInContent.intersect(layerClipRect);
		
        ScrollView* parentScrollView = parent();
		
        IntPoint const locationInViewPort(parentScrollView->contentsToWindow(geometryInContent.location()));
        IntPoint const locationInWindow(viewportToWindow(locationInViewPort));
		
		return getCarbonRectForNSWindowRect(IntRect(locationInWindow, geometryInContent.size()));
	}
#endif
	
    void PluginView::updatePluginWidget()
	{
        if (!parent())
			return;
		
        ASSERT(parent()->isFrameView());
        FrameView* frameView = static_cast<FrameView*>(parent());
		
        m_windowRect = IntRect(frameView->contentsToWindow(frameRect().location()), frameRect().size());
		m_clipRect = windowClipRect();
		m_clipRect.move(-m_windowRect.x(), -m_windowRect.y());    
	}
    
    void PluginView::setParent(ScrollView* parent)
    {
        Widget::setParent(parent);
        if(parent)
            init();
        else
		{
            FrameLoaderClientApollo::clientApollo(m_parentFrame.get())->deactivateIMEForPlugin();
        }
		
    }
    
    void PluginView::setFocus(bool bFocus)
    {
        if (bFocus) {
#ifndef NP_NO_QUICKDRAW
			// We need to cache the active TSMDocument (which is most likely the application's TSMDocument) in case the plug-in creates a new one when setting focus to a text field, 
			// but does not re-activate the current active TSMDocument when it loses focus (bug# 2633352)
			m_appTSMDocID = TSMGetActiveDocument();
#endif
			
            FrameLoaderClientApollo::clientApollo(m_parentFrame.get())->activateIMEForPlugin();
		}
        else
            FrameLoaderClientApollo::clientApollo(m_parentFrame.get())->deactivateIMEForPlugin();
        
        // Focus the plugin
        if (m_plugin) {
#ifndef NP_NO_CARBON
            EventRecord focusEvent;
			focusEvent.what = bFocus ? getFocusEvent : loseFocusEvent;
            focusEvent.message = 0;
            focusEvent.when = TickCount();
            focusEvent.where.v = 0;
            focusEvent.where.h = 0;
            focusEvent.modifiers = 0;
#else
            NPCocoaEvent focusEvent;
            memset(&focusEvent, 0, sizeof(NPCocoaEvent));
            focusEvent.type = NPCocoaEventFocusChanged;//NOT NPCocoaEventWindowFocusChanged;
            focusEvent.data.focus.hasFocus = bFocus;
#endif
            
			JSC::JSLock::DropAllLocks dropLocks(false);
			
			setCallingPlugin(true);
            m_plugin->pluginFuncs()->event(m_instance, &focusEvent);
			setCallingPlugin(false);
        }
		
		if(m_plugin->quirks().contains(PluginQuirkApolloSWFPlugin) && !bFocus
#ifndef NP_NO_QUICKDRAW
		   && m_appTSMDocID && m_appTSMDocID != TSMGetActiveDocument()
#endif
		   )
#ifndef NP_NO_QUICKDRAW
			ActivateTSMDocument(m_appTSMDocID);
#endif
        
		Widget::setFocus(bFocus);
        
		// if focus is lost, we need to unfocus ourself
		ASSERT(m_parentFrame);
		ASSERT(m_parentFrame->document());
		ASSERT(m_element);
		if (!bFocus && m_parentFrame->document()->focusedNode() == m_element)
			m_parentFrame->document()->setFocusedNode(0);
	}
    
	void PluginView::show()
	{
		if (!m_plugin)
			return;
		
        const bool visibleStateChanged = (m_isEnabled && !isVisible() /*&& m_attachedToWindow*/);
        setSelfVisible(true);
		
        if(parent() && visibleStateChanged)
			invalidate();
	}
	
	void PluginView::hide()
	{
		if (!m_plugin)
			return;
		
        const bool visibleStateChanged = (m_isEnabled && isVisible()/* && m_attachedToWindow*/);
        setSelfVisible(false);
		
        if(parent() && visibleStateChanged)
			invalidate();
	}
	
#ifndef NP_NO_QUICKDRAW
	IntPoint PluginView::getOriginOfHTMLControlInWebWindow() const
	{
        return convertToApolloWindowPoint(IntPoint());
	}
	
	IntPoint PluginView::getOriginOfHTMLControlInCarbonWindow() const
	{
		IntPoint const originOfHTMLControlInWindow(getOriginOfHTMLControlInWebWindow());
		IntRect const onePixelRect(originOfHTMLControlInWindow, IntSize(1, 1));
		
		IntRect const onePixelRectInCarbonWindow(getCarbonRectForNSWindowRect(onePixelRect));
		return onePixelRectInCarbonWindow.location();
	}
	
	WindowRef PluginView::getCarbonWindow() const
	{
        WebWindow* const webWindow = getApolloWebWindow();
		WindowRef carbonWindow = NULL;
        if(webWindow)
        {
			NSWindow* const nsWindow = webWindow->m_pVTable->getPlatformWindow(webWindow);
			carbonWindow = reinterpret_cast<WindowRef>(webWindow->m_pVTable->getPlatformWindowRef(webWindow));
			if (nsWindow && carbonWindow)
			{
				pluginCarbonWindowCheckIsKey(nsWindow, carbonWindow);
			}
		}
		return carbonWindow;
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
#endif
	
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
            CGContextRef currentContext = context->platformContext();
            
#ifdef NP_NO_QUICKDRAW
            m_npWindow.window = currentContext;
#endif
            setNPWindowRect(frameRect());
			
            // This should really move to use m_clipRect, after calling updatePluginWidget()
            // It takes care of clipping to the window for windowless plug-ins.
			IntRect clipRect = clipDirtyRect(rect);
			
			if (m_drawingModel == NPDrawingModelCoreGraphics) {
				CGContextSaveGState(currentContext);

                CGAffineTransform const transformToMakeCGContextUseWindowCoords = { 1, 0, 0, 1, frameRect().x(), frameRect().y()};
                CGContextConcatCTM(currentContext, transformToMakeCGContextUseWindowCoords);

                NPCocoaEvent paint_event;
                memset(&paint_event, 0, sizeof(NPCocoaEvent));
                paint_event.type = NPCocoaEventDrawRect;
                paint_event.data.draw.context = currentContext;
                paint_event.data.draw.x = clipRect.x();
                paint_event.data.draw.y = clipRect.y();
                paint_event.data.draw.width = clipRect.width();
                paint_event.data.draw.height = clipRect.height();

                JSC::JSLock::DropAllLocks dropLocks(false);
                setCallingPlugin(true);
                m_plugin->pluginFuncs()->event(m_instance, &paint_event);
                setCallingPlugin(false);

                CGContextRestoreGState(currentContext);
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

            ASSERT(m_drawingModel == NPDrawingModelQuickDraw);
            // QuickDraw does not understand alpha, so we just saturate the alpha channel
            // after we draw.  The FlashPlayer plugin draws then entire bitmape everytime, so
            // I whack all the pixels (not just the update rect).
            //IntPoint p = m_parentFrame->view()->contentsToWindow(frameRect().location());
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
#endif
		}
		
		context->restore();
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
        
#ifndef NP_NO_CARBON
		
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
#endif
        
	}
    
#ifndef NP_NO_CARBON
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
#else
    bool PluginView::handleInsertText(const String& text)
    {
        NPCocoaEvent insertTextEvent;
        memset(&insertTextEvent, 0, sizeof(NPCocoaEvent));
        insertTextEvent.type = NPCocoaEventTextInput;
        insertTextEvent.data.text.text = (NPNSString*)(text.createCFString());

        JSC::JSLock::DropAllLocks dropLocks(false);
        setCallingPlugin(true);
        m_plugin->pluginFuncs()->event(m_instance, &insertTextEvent);
        setCallingPlugin(false);
        
        return true;
    }
#endif // !NP_NO_CARBON

#if ! defined(NP_NO_CARBON)

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
		CFIndex length = CFStringGetBytes(str.get(), range, encoding, '?', false, NULL, 0, &bufferSize);    
		if (length <= 0)
            return 0; //conversion failed
		
		Vector<UInt8> buffer(bufferSize);
		length = CFStringGetBytes(str.get(), range, encoding, '?', false, buffer.data(), bufferSize, NULL);
        
		ASSERT(bufferSize == 1);
        return buffer[0];
	}    
#endif // !NP_NO_CARBON

    static bool isFlagsChangedEvent(const PlatformKeyboardEvent* keyboardEvent)
    {
        switch (keyboardEvent->windowsVirtualKeyCode())
        {
            case 15: // Command
            case 16: // Shift
            case 17: // Alt
            case 18: // Control
            case 20: // Caps Lock
                return true;
        }
        
        return false;
    }

	void PluginView::handleKeyboardEvent(KeyboardEvent* kbEvent)
	{
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
        
#if ! defined(NP_NO_CARBON)

        // shiftKeyBit, controlKeyBit, optionKeyBit, and cmdKeyBit are defined
        // in /System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/Events.h
        UInt16 const shiftModifier = static_cast<UInt16>(kbEvent->shiftKey()) << shiftKeyBit;
        UInt16 const ctrlModified = static_cast<UInt16>(kbEvent->ctrlKey()) << controlKeyBit;
        UInt16 const altModifier = static_cast<UInt16>(kbEvent->altKey()) << optionKeyBit;
        UInt16 const metaModifier = static_cast<UInt16>(kbEvent->metaKey()) << cmdKeyBit;
        
        EventRecord keyboardEvent;
        keyboardEvent.message = (macKeyCode << 8) | ((charCode <= 0x7f) ? charCode : convertCharCode(keyEventText));
        ASSERT(macKeyCode == (keyboardEvent.message >> 8));
        keyboardEvent.when = TickCount();
        getCarbonMousePosition(&keyboardEvent.where);
        keyboardEvent.modifiers = shiftModifier | ctrlModified | altModifier | metaModifier;
        keyboardEvent.what = (type == PlatformKeyboardEvent::KeyUp) ? keyUp : keyDown;

#else
        
        NPCocoaEvent keyboardEvent;
        memset(&keyboardEvent, 0, sizeof(NPCocoaEvent));
        keyboardEvent.data.key.modifierFlags = (kbEvent->shiftKey() ? NSShiftKeyMask : 0) | (kbEvent->ctrlKey() ? NSControlKeyMask : 0) | (kbEvent->altKey() ? NSAlternateKeyMask : 0) | (kbEvent->metaKey() ? NSCommandKeyMask : 0);

        if (isFlagsChangedEvent(keyEvent))
        {
            // caps lock key is being handled in a different way
            if (macKeyCode == 57)
                keyboardEvent.data.key.modifierFlags |= NSAlphaShiftKeyMask;
            
            keyboardEvent.type = NPCocoaEventFlagsChanged;
            keyboardEvent.data.key.characters = NULL;
            keyboardEvent.data.key.charactersIgnoringModifiers = NULL;
            keyboardEvent.data.key.isARepeat = false;
        }
        else
        {
            keyboardEvent.type = (type == PlatformKeyboardEvent::KeyUp) ? NPCocoaEventKeyUp : NPCocoaEventKeyDown;
            keyboardEvent.data.key.characters = (NPNSString*)(keyEvent->text().createCFString());
            keyboardEvent.data.key.charactersIgnoringModifiers = (NPNSString*)(keyEvent->unmodifiedText().createCFString());
            keyboardEvent.data.key.isARepeat = keyEvent->isAutoRepeat();
        }
        keyboardEvent.data.key.keyCode = macKeyCode;

#endif
        
        startUserGesture();
        
		JSC::JSLock::DropAllLocks dropLocks(false);
        setCallingPlugin(true);
        bool result = m_plugin->pluginFuncs()->event(m_instance, &keyboardEvent);
        setCallingPlugin(false);
        
        if(result)
            kbEvent->setDefaultHandled();
    }
	
#ifndef NP_NO_CARBON
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
#else
    void PluginView::handleMouseEvent(MouseEvent* event)
    {
        NPCocoaEvent cocoaEvent;
        memset(&cocoaEvent, 0, sizeof(NPCocoaEvent));

        uint32_t modifierFlags = 0;
        
        if (event->type() == eventNames().mousedownEvent)
            cocoaEvent.type = NPCocoaEventMouseDown;
        else if (event->type() == eventNames().mouseupEvent)
            cocoaEvent.type = NPCocoaEventMouseUp;
        else if (event->type() == eventNames().mousemoveEvent)
            if (event->buttonDown())
                cocoaEvent.type = NPCocoaEventMouseDragged;
            else
                cocoaEvent.type = NPCocoaEventMouseMoved;
        else if (event->type() == eventNames().mouseoverEvent)
            cocoaEvent.type = NPCocoaEventMouseEntered;
        else if (event->type() == eventNames().mouseoutEvent)
            cocoaEvent.type = NPCocoaEventMouseExited;
        else if( event->type() == eventNames().clickEvent )
            cocoaEvent.data.mouse.clickCount = 1;
        else if( event->type() == eventNames().dblclickEvent )
            cocoaEvent.data.mouse.clickCount = 2;
        else
        {
//          Do not remove the line below, the printout of the event type is non-trivial
//          printf("Something is seriously wrong!, we got an unexpected event: %s\n", event->type().string().utf8().data());
	        ASSERT(0);
            return;
        }
        
        modifierFlags += event->ctrlKey() ? NSControlKeyMask : 0;
        modifierFlags += event->shiftKey() ? NSShiftKeyMask : 0;
        modifierFlags += event->metaKey() ? NSCommandKeyMask : 0;
        modifierFlags += event->altKey() ? NSAlternateKeyMask : 0;
        cocoaEvent.data.mouse.modifierFlags = modifierFlags;
        
        cocoaEvent.data.mouse.pluginX = event->offsetX();
        cocoaEvent.data.mouse.pluginY = event->offsetY();

        // which()--> 1,2,3 for Left,Middle,Right. buttonNumber as expected by the plugin --> 0,1,2 for Left,Right,Middle
        switch(event->button())
        {
            case LeftButton:
                cocoaEvent.data.mouse.buttonNumber = 0; // left 
                break;
            case MiddleButton:
                cocoaEvent.data.mouse.buttonNumber = 2; // middle 
                break;
            case RightButton:
                cocoaEvent.data.mouse.buttonNumber = 1; // right
                cocoaEvent.data.mouse.modifierFlags |= NSControlKeyMask;
                break;
            default:
                cocoaEvent.data.mouse.buttonNumber = event->button();
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
		
		startUserGesture();

        JSC::JSLock::DropAllLocks dropLocks(false);
        setCallingPlugin(true);
        bool const eventHandledByPlugin = m_plugin->pluginFuncs()->event(m_instance, &cocoaEvent);
        setCallingPlugin(false);
		if (eventHandledByPlugin)
			event->setDefaultHandled();
    }
#endif // !NP_NO_CARBON

#if PLATFORM(APOLLO) && defined(NP_NO_CARBON)
    void PluginView::handleWheelEvent(WheelEvent* event)
    {
        NPCocoaEvent cocoaEvent;
        memset(&cocoaEvent, 0, sizeof(NPCocoaEvent));
        
        if (event->type() == eventNames().mousewheelEvent)
            cocoaEvent.type = NPCocoaEventScrollWheel;
        else
        {
//          Do not remove the line below, the printout of the event type is non-trivial
//          printf("Something is seriously wrong!, we got an unexpected event: %s\n", event->type().string().utf8().data());
	        ASSERT(0);
            return;
        }
        
        cocoaEvent.data.mouse.pluginX = event->offsetX();
        cocoaEvent.data.mouse.pluginY = event->offsetY();
        cocoaEvent.data.mouse.deltaX = event->wheelDeltaX(); // plugin currently does not use horizontal scrolling but we may in the future.
        cocoaEvent.data.mouse.deltaY = event->wheelDeltaY();
        		
		startUserGesture();
        
        JSC::JSLock::DropAllLocks dropLocks(false);
        setCallingPlugin(true);
        bool const eventHandledByPlugin = m_plugin->pluginFuncs()->event(m_instance, &cocoaEvent);
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
#else
    void PluginView::setNPWindowRect(const IntRect& rect)
    {
        if (!m_isStarted || !parent() || !m_plugin->pluginFuncs()->setwindow)
            return;
        
        if (platformPluginWidget()) {
            m_npWindow.type = NPWindowTypeWindow;
        } else {
            m_npWindow.type = NPWindowTypeDrawable;
        }
                
        m_npWindow.x = rect.x();
        m_npWindow.y = rect.y();
        m_npWindow.width = rect.width();
        m_npWindow.height = rect.height();

        m_npWindow.clipRect.left = max(0, rect.x());
        m_npWindow.clipRect.top = max(0, rect.y());
        m_npWindow.clipRect.right = rect.x() + rect.width();
        m_npWindow.clipRect.bottom = rect.y() + rect.height();

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
	
	bool PluginView::platformGetValueStatic(NPNVariable /*variable*/, void* /*value*/, NPError* /*result*/)	{
		return false;
	}
	
	bool PluginView::platformGetValue(NPNVariable variable, void* value, NPError* result)
	{
		switch (variable) {
			case NPNVsupportsCoreGraphicsBool:
				*reinterpret_cast<NPBool*>(value) = TRUE;
				*result = NPERR_NO_ERROR;
				return true;
#ifndef NP_NO_QUICKDRAW
			case NPNVsupportsQuickDrawBool:
				*reinterpret_cast<NPBool*>(value) = TRUE;
				*result = NPERR_NO_ERROR;
				return true;
#endif
			case NPNVsupportsOpenGLBool:
				*reinterpret_cast<NPBool*>(value) = FALSE;
				*result = NPERR_NO_ERROR;
				return true;
			case NPNVpluginDrawingModel:
				*reinterpret_cast<NPDrawingModel*>(value) = m_drawingModel;
				*result = NPERR_NO_ERROR;
				return true;
#ifdef NP_NO_CARBON
			case NPNVsupportsCocoaBool:
				*reinterpret_cast<NPBool*>(value) = TRUE;
				*result = NPERR_NO_ERROR;
				return true;
			case NPNVsupportsUpdatedCocoaTextInputBool:
				*reinterpret_cast<NPBool*>(value) = FALSE; // our WebKit doesn't support this . see comment in MacPluginPlayer.mm: PlatformPlayer::HandleEvent: case NPCocoaEventKeyDown
				*result = NPERR_NO_ERROR;
				return true;
#endif // NP_NO_CARBON
			default:
				return false;
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
	
    NPBool PluginView::convertPoint(double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace)
    {
        // This function converts ONLY from NPCoordinateSpacePlugin to NPCoordinateSpaceFlippedScreen as it is being called ONLY once by the Flash Player code
        // in PlatformPlayer::ConvertLocalPointToGlobal in MacPluginPlayer.mm
        
        (void)sourceSpace;
        (void)destSpace;
        
        ASSERT(sourceSpace == NPCoordinateSpacePlugin);
        ASSERT(destSpace == NPCoordinateSpaceFlippedScreen);
        
        IntRect geometryInContent(frameRect());
        ASSERT(m_element);
        ASSERT(m_element->renderer());
        
        // Take our element and get the clip rect from the enclosing layer.
        RenderLayer* const layer = m_element->renderer()->enclosingLayer();
        ASSERT(layer);
        IntRect layerClipRect(layer->childrenClipRect());
        geometryInContent.intersect(layerClipRect);
        
        ScrollView* parentScrollView = parent();
        
        IntPoint const locationInViewPort(parentScrollView->contentsToWindow(geometryInContent.location()));
        IntPoint const locationInWindow(viewportToWindow(locationInViewPort));
        
        *destX = sourceX + locationInWindow.x();
        *destY = sourceY + locationInWindow.y();
        
        return true;
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
	
	void PluginView::platformDestroy()
	{        
		if (m_npCGContext.context)
			CGContextRelease(m_npCGContext.context);
#ifndef NP_NO_QUICKDRAW
		if (m_npPort.port)
			QDDeprecated::DisposePortFromBytes(m_npPort.port);
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
	
	void PluginView::halt()
	{
	}
	
	void PluginView::restart()
	{
	}
	
	// --------------- Lifetime management -----------------
	
	bool PluginView::platformStart()
	{
			if (!m_plugin->quirks().contains(PluginQuirkDeferFirstSetWindowCall))
				setNPWindowRect(frameRect());
			
#ifndef NP_NO_QUICKDRAW
			m_appTSMDocID = 0;
#endif
		
			return true;
		}
		
#if PLATFORM(APOLLO)
		// ------------- Handle Edit events --------------------
		bool PluginView::handleOnCopy()
		{
			ASSERT(m_parentFrame);
			ASSERT(m_parentFrame->document());
			ASSERT(m_parentFrame->document()->defaultView());
			
			WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
													, 'c' /* winCharCode */
													, 0x43 /* Web_VK_C */ /* keyCode*/
													, false /* isShiftKey */
													, false /* isCtrlKey */
													, false /* isAltKey */ 
													, true  /* isMetaKey */
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
													, false /* isCtrlKey */
													, false /* isAltKey */ 
													, true  /* isMetaKey */
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
													, false /* isCtrlKey */
													, false /* isAltKey */ 
													, true  /* isMetaKey */
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
													, false /* isCtrlKey */
													, false /* isAltKey */ 
													, true  /* isMetaKey */
													, false  /* isAutoRepeat */);
			RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent, m_parentFrame->document()->defaultView());
			handleEvent(keyDown.get());
			return true;
		}
#endif
		
	} // namespace WebCore
	
	
