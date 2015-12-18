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

#include "config.h"
#include "npapi.h"
#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
#include "PluginCarbonWindow.h"
#include <wtf/Assertions.h>
#include <Carbon/Carbon.h>
#include "DeprecatedCarbonFunctions/QDDeprecated.h"
#include "npapi.h"
#include "PluginView.h"

namespace WebCore {

PluginCarbonWindow::~PluginCarbonWindow()
{
    if (m_windowRef)
        DisposeWindow(m_windowRef);
    m_windowRef = 0;
}

namespace {

/*****************************************************
*
* TransparentWindowHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  The handlers for the 3 events that we intercept in MakeWindowTransparent
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static pascal OSStatus TransparentWindowHandler(EventHandlerCallRef /*inHandlerCallRef*/, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;
	
	switch(GetEventKind(inEvent))
	{
		case kEventWindowGetRegion:
		{
			WindowRegionCode code;
			RgnHandle rgn;
			
			// which region code is being queried?
			GetEventParameter(inEvent, kEventParamWindowRegionCode, typeWindowRegionCode, NULL, sizeof(code), NULL, &code);
            
			// if it is the opaque region code then set the region to Empty and return noErr to stop the propagation
			if (code == kWindowOpaqueRgn)
			{
				GetEventParameter(inEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof(rgn), NULL, &rgn);
				QDDeprecated::SetEmptyRgn(rgn);
				status = noErr;
			}
			break;
		}
            
		case kEventWindowDrawContent:
		{
			GrafPtr port;
			CGContextRef context;
			Rect portBounds;
			HIRect bounds;
			
			QDDeprecated::GetPort(&port);
			GetPortBounds(port, &portBounds);
			
			// we need a Quartz context so that we can use transparency
			QDBeginCGContext(port, &context);
			
			// make the whole content transparent
			bounds = CGRectMake(0, 0, portBounds.right, portBounds.bottom);
			CGContextClearRect(context, bounds);
			
			QDEndCGContext(port, &context);
			
			// we need to let the HIToolbox and the regular kEventWindowDrawContent handler do their work,
			// mainly draw the subviews, so we return eventNotHandledErr to propagate.
			status = eventNotHandledErr;
			break;
		}
            
		case kEventControlDraw:
		{
			CGContextRef context;
			HIRect bounds;
            
			GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
			HIViewGetBounds((HIViewRef)inUserData, &bounds);
            
			// make the whole content transparent
			CGContextClearRect(context, bounds);
            
			// we must not let the default draw handler of the content view be called (it would draw the default opaque theme)
			// so we return noErr to stop the propagation.
			status = noErr;
			break;
		}
	}
	
	return status;
}   // TransparentWindowHandler


static OSStatus MakeWindowTransparent(WindowRef aWindowRef)
{
	OSStatus status = paramErr;
	require(aWindowRef != NULL, paramErr);
	
	// is the window compositing or not?
	WindowAttributes attributes;
	status = GetWindowAttributes(aWindowRef, &attributes);
	require_noerr(status, GetWindowAttributes);
	
	if (attributes & kWindowCompositingAttribute)
	{
		// it is compositing so we intercept the kEventWindowGetRegion event to be able to specify an empty opaque region
		EventTypeSpec wCompositingEvents = { kEventClassWindow, kEventWindowGetRegion };
		status = InstallWindowEventHandler(aWindowRef, TransparentWindowHandler, 1, &wCompositingEvents, aWindowRef, NULL);
		require_noerr(status, InstallWindowEventHandler);
		
		HIViewRef contentView;
		status = HIViewFindByID(HIViewGetRoot(aWindowRef), kHIViewWindowContentID, &contentView);
		require_noerr(status, HIViewFindByID);
        
		// and we intercept the kEventControlDraw event of our content view so that we can make it transparent
		EventTypeSpec cCompositingEvents = { kEventClassControl, kEventControlDraw };
		status = InstallControlEventHandler(contentView, TransparentWindowHandler, 1, &cCompositingEvents, contentView, NULL);
		require_noerr(status, InstallControlEventHandler);
	}
	else
	{
		// it is non-compositing so we intercept the kEventWindowGetRegion event to be able to specify an empty opaque region
		// and we intercept the kEventWindowDrawContent event of our window so that we can make it transparent
		EventTypeSpec wNonCompositingEvents[] = {
            { kEventClassWindow, kEventWindowGetRegion },
            { kEventClassWindow, kEventWindowDrawContent } };
		status = InstallWindowEventHandler(aWindowRef, TransparentWindowHandler, GetEventTypeCount(wNonCompositingEvents), wNonCompositingEvents, aWindowRef, NULL);
		require_noerr(status, InstallWindowEventHandler);
	}
    
	// telling the HIToolbox that our window is not opaque so that we will be asked for the opaque region
	UInt32 features;
	status = GetWindowFeatures(aWindowRef, &features);
	require_noerr(status, GetWindowFeatures);
	if ( ( features & kWindowIsOpaque ) != 0 )
	{
		status = HIWindowChangeFeatures(aWindowRef, 0, kWindowIsOpaque);
		require_noerr(status, HIWindowChangeFeatures);
	}
    
	// force opaque shape to be recalculated
	status = ReshapeCustomWindow(aWindowRef);
	require_noerr(status, ReshapeCustomWindow);
    
	// ensure that HIToolbox doesn't use standard shadow style, which defeats custom opaque shape
	status = SetWindowAlpha(aWindowRef, 0.999f);
	require_noerr(status, SetWindowAlpha);
    
SetWindowAlpha:
ReshapeCustomWindow:
HIWindowChangeFeatures:
GetWindowFeatures:
InstallControlEventHandler:
HIViewFindByID:
InstallWindowEventHandler:
GetWindowAttributes:
paramErr:
        
        return status;
}   // MakeWindowTransparent

static const Rect initialWindowBounds = { 0, 0, 0, 0 };

}

WindowRef PluginCarbonWindow::createTransparentWindow()
{
    WindowRef window;
    OSStatus const createWindowError = CreateNewWindow(kUtilityWindowClass
                                                      , kWindowNoShadowAttribute | kWindowStandardHandlerAttribute | kWindowNoTitleBarAttribute
                                                      , &initialWindowBounds
                                                      , &window);
    if (createWindowError != noErr)
        return 0;
    ASSERT(window);
    RepositionWindow(window, 0, kWindowCenterOnMainScreen);
    OSStatus const transparentError = MakeWindowTransparent(window);
    if (transparentError != noErr) {
        DisposeWindow(window);
        return 0;
    }
    ShowWindow(window);
    return window;
}

PluginCarbonWindow::PluginCarbonWindow(PluginView* pluginView)
    : m_windowRef(createTransparentWindow())
    , m_ownerPluginView(pluginView)
{
    ASSERT(m_windowRef);
}

IntRect PluginCarbonWindow::syncWindowAndGetGeometry()
{
    IntRect widgetGeometry(m_ownerPluginView->frameRect());
    Rect const newWindowBounds = { 0, 0, widgetGeometry.height(), widgetGeometry.width() };
    SetWindowBounds(m_windowRef, kWindowContentRgn, &newWindowBounds);
    return widgetGeometry;
}

std::auto_ptr<PluginCarbonWindow> PluginCarbonWindow::construct(PluginView* pluginView)
{
    return std::auto_ptr<PluginCarbonWindow>(new PluginCarbonWindow(pluginView));
}

void PluginCarbonWindow::translatePoint(Point* const translatedPoint, int eventX, int eventY)
{
    ASSERT(translatedPoint);
    IntRect const frameGeometry(syncWindowAndGetGeometry());
    translatedPoint->h = eventX - frameGeometry.x();
    translatedPoint->v = eventY - frameGeometry.y();
}

bool PluginCarbonWindow::fillInNPWindowPositionInfoForEvents(NPWindow* npWindow)
{
    ASSERT(npWindow);
    IntRect const frameGeometry(syncWindowAndGetGeometry());
    bool dirty = false;
    if ((dirty) || (npWindow->x != 0)) {
        dirty = true;
        npWindow->x = 0;
    }
    
    if ((dirty) || (npWindow->y != 0)) {
        dirty = true;
        npWindow->y = 0;
    }
    if ((dirty) || (npWindow->width != static_cast<unsigned int>(frameGeometry.width()))) {
        dirty = true;
        npWindow->width = static_cast<unsigned int>(frameGeometry.width());
    }
    if ((dirty) || (npWindow->height != static_cast<unsigned int>(frameGeometry.height()))) {
        dirty = true;
        npWindow->height = static_cast<unsigned int>(frameGeometry.height());
    }
    if ((dirty) || (npWindow->clipRect.left != 0)) {
        dirty = true;
        npWindow->clipRect.left = 0;
    }
    if ((dirty) || (npWindow->clipRect.top != 0)) {
        dirty = true;
        npWindow->clipRect.top = 0;
    }
    if ((dirty) || (npWindow->clipRect.right != frameGeometry.width())) {
        dirty = true;
        npWindow->clipRect.right = frameGeometry.width();
    }
    if ((dirty) || (npWindow->clipRect.bottom != frameGeometry.height())) {
        dirty = true;
        npWindow->clipRect.bottom = frameGeometry.height();
    }
    return dirty;
}

#if 0
void PluginView::eventPointToPluginCarbonPointUsingHIPoint(const IntPoint& eventPoint, WindowRef const carbonWindow, Point* carbonPoint)
{
    ASSERT(carbonPoint);
    IntRect const frameGeom(frameGeometry());
    IntPoint const frameLocation(frameGeom.location());
    IntSize const frameOffset(IntPoint(m_npWindow.x, m_npWindow.y) - frameLocation);
    IntPoint const pluginPoint(eventPoint + frameOffset);
    
    HIPoint mouseHIPoint;
    mouseHIPoint.x = pluginPoint.x();
    mouseHIPoint.y = pluginPoint.y();
    
    if (m_isWindowed) {
        ASSERT(carbonWindow);
        if (carbonWindow) {
            // ** EVIL **
            // In apollo, the HTML rendering can be rotated,
            // blurred, etc.  This means the relation ship between
            // the coordinate space of the plugin and the screen is not straight forward.
            // Fortunately, we are already making the carbon event from whole cloth.
            // The FlashPlayer uses HIPointConvert to convert the coordinates of the
            // carbon event to the coordinates of the plugin by calling HIPointConvert.
            // We just call HIPointConvert to convert the mouse point the opposite direction
            // so the FlashPlayer will end up with the correct coordinates for the mouse point.
            HIPointConvert(&mouseHIPoint
                          , kHICoordSpaceWindow
                          , carbonWindow
                          , kHICoordSpaceScreenPixel
                          , 0);
            
        }
    }
    
    carbonPoint->h = static_cast<short>(mouseHIPoint.x);
    carbonPoint->v = static_cast<short>(mouseHIPoint.y);
}

void PluginView::eventPointToPluginCarbonPointUsingQuickDraw(const IntPoint& eventPoint, WindowRef const carbonWindow, Point* carbonPoint)
{
    ASSERT(carbonPoint);
    IntRect const frameGeom(frameGeometry());
    IntPoint const frameLocation(frameGeom.location());
    IntSize const frameOffset(IntPoint(m_npWindow.x, m_npWindow.y) - frameLocation);
    IntPoint const pluginPoint(eventPoint + frameOffset);
    
    HIPoint mouseHIPoint;
    mouseHIPoint.x = pluginPoint.x();
    mouseHIPoint.y = pluginPoint.y();
    
}

#endif

}
#endif // PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
