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
#include <Frame.h>
#include <FrameView.h>
#include <FrameLoader.h>
#include <FrameLoaderClientApollo.h>
#include <Document.h>
#include <WebKitApollo/WebPoint.h>
#include <WebKitApollo/WebRect.h>
#include <WebKitApollo/WebBitmap.h>
#include <WebKitApollo/WebHost.h>
#include <WebKitApollo/WebWindow.h>
#include <FrameView.h>
#include <GraphicsContext.h>
#include <Page.h>
#include <Chrome.h>

namespace WebCore {


void Frame::keepAlive()
{
    ASSERT(loader());
    FrameLoaderClientApollo* const client = FrameLoaderClientApollo::clientApollo(this);
    ASSERT(client);
    client->putFrameOnLifeSupport(this);
}

WebBitmap* Frame::imageFromRect(const FloatRect& rect) const
{
    // We should not try to create a bitmap of zero height or width as it is not supported.
	IntRect irect(rect);
    if(irect.size().width() == 0 || irect.size().height() == 0) 
        return NULL;

    FrameLoaderClientApollo* const clientApollo = FrameLoaderClientApollo::clientApollo(this);
    WebHost* webHost = clientApollo->webHost();
    ASSERT(webHost);

    WebBitmap* resultBitmap = webHost->m_pVTable->createBitmap(webHost, irect.size().width(), irect.size().height());
    
	// clear pixels with transparent black (0)
	void *pixelData = resultBitmap->m_pVTable->getPixelData(resultBitmap);
    unsigned long stride = resultBitmap->m_pVTable->getStride(resultBitmap);
    memset( pixelData, 0, stride * irect.height() );
	

	// now, paint the selection in a graphics context
	WebIntRect sourceRect;
		
	sourceRect.m_left = 0; 
	sourceRect.m_top = 0; 
	sourceRect.m_right = irect.width();
	sourceRect.m_bottom = irect.height();
	
	GraphicsContext gc (resultBitmap,&sourceRect);

	IntSize offset = view()->scrollOffset();
    irect.move(-offset.width(), -offset.height());
    irect = view()->convertToContainingWindow(irect);

#if OS(WINDOWS) ||  OS(DARWIN)
	// changing origin from top-left to bottom-left 
	gc.concatCTM(TransformationMatrix(1, 0, 0, -1, 0, irect.height()).toAffineTransform());
#endif 

	// this transformation moves the clip into the origin of the user space
	gc.concatCTM(TransformationMatrix().translate(-irect.x(), -irect.y()).toAffineTransform());
	
	view()->paint(&gc,irect);
	
    return resultBitmap;
}

WebBitmap* Frame::selectionImage(bool forceWhiteText) const
{
    m_view->setPaintBehavior(PaintBehaviorSelectionOnly | (forceWhiteText ? 0 : PaintBehaviorForceBlackText));
    WebBitmap* result = imageFromRect(selectionBounds(true));
    m_view->setPaintBehavior(PaintBehaviorNormal);
    return result;
}

WebBitmap* Frame::snapshotDragImage(Node* node, IntRect& imageRect, IntRect& elementRect) const
{
    RenderObject* renderer = node->renderer();
    if (!renderer)
        return NULL;

    renderer->updateDragState(true);    // mark dragged nodes (so they pick up the right CSS)
    m_doc->updateLayout();        // forces style recalc - needed since changing the drag state might
                                        // imply new styles, plus JS could have changed other things
    IntRect topLevelRect;
    IntRect paintingRect = renderer->paintingRootRect(topLevelRect);

    m_view->setNodeToDraw(node);              // invoke special sub-tree drawing mode
    WebBitmap* result = imageFromRect(paintingRect);
    renderer->updateDragState(false);
    m_doc->updateLayout();
    m_view->setNodeToDraw(0);

    elementRect = topLevelRect;
	imageRect = paintingRect;
    return result;
}

DragImageRef Frame::dragImageForSelection() 
{
    if (!selection()->isRange())
        return NULL;
    return selectionImage();	
}

//PassScriptInstance Frame::createScriptInstanceForWidget(WebCore::Widget* widget)
//{
//	const WTF::RefPtr< IWidgetApolloImpl >&impl =  widget->getApolloImpl();
//
//	return impl->getInstance(this);
//}
	
	

}
