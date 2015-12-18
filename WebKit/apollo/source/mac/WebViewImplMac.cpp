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
#include "../WebViewImpl.h"
#include <wtf/Assertions.h>
#include <GraphicsContext.h>
#include <Frame.h>
#include <FrameView.h>
#include <ApplicationServices/ApplicationServices.h>

#include "../WebFrameImpl.h"

#include <sampler/SamplerContext.h>

namespace WebKitApollo
{

	static const size_t bitsPerComponent = 8;
	void WebViewImpl::paint( const WebIntRect* const pPaintRect )
	{
		WTF::RefPtr< WebCore::Frame > const mainWebCoreFrame = mainFrame();
        ASSERT(mainWebCoreFrame);
        WebCore::Document* const mainFrameDoc = mainWebCoreFrame->document();
        if ((mainFrameDoc) && (mainFrameDoc->renderer())) {

#if ENABLE(APOLLO_PROFILER)
            SAMPLER_CONTEXT( &JSC::SamplerContextPool::builtins.htmlPaint );
#endif
				                        
            ASSERT(m_pTargetWindow);
            ASSERT(m_pTargetWindow->m_pVTable);
            WebBitmap* const pTargetBitmap = m_pTargetWindow->m_pVTable->getBitmapSurface( m_pTargetWindow );
            ASSERT(pTargetBitmap);
            ASSERT(pTargetBitmap->m_pVTable);
            unsigned char* const pTargetPixelData = reinterpret_cast<unsigned char*>( pTargetBitmap->m_pVTable->getPixelData( pTargetBitmap ) );
            int const targetBitmapWidth = static_cast< int >( pTargetBitmap->m_pVTable->getWidth( pTargetBitmap ) );
            int const targetBitmapHeight = static_cast< int >( pTargetBitmap->m_pVTable->getHeight( pTargetBitmap ) );
            int const targetBitmapStride = static_cast< int >( pTargetBitmap->m_pVTable->getStride( pTargetBitmap ) );

            CGColorSpaceRef const colorSpace = CGColorSpaceCreateDeviceRGB();
            
            CGContextRef const cgContext = CGBitmapContextCreate(	pTargetPixelData,
                                                                    targetBitmapWidth,
                                                                    targetBitmapHeight,
                                                                    bitsPerComponent,
                                                                    targetBitmapStride,
                                                                    colorSpace,
                                                                    kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host );
                                                                    
            CGAffineTransform const mirrorTransform = { 1, 0, 0, -1, 0, targetBitmapHeight };
            CGContextConcatCTM(cgContext, mirrorTransform);
            
            WebCore::GraphicsContext webCoreContext( cgContext );
            WebCore::IntRect dirtyRect(	pPaintRect->m_left
                                      , pPaintRect->m_top
                                      , pPaintRect->m_right - pPaintRect->m_left
                                      , pPaintRect->m_bottom - pPaintRect->m_top );
            
            WTF::RefPtr< WebCore::FrameView > const pMainFrameView( mainWebCoreFrame->view() );
            
            pMainFrameView->layoutIfNeededRecursive();

            webCoreContext.save();

            webCoreContext.clip(dirtyRect);
            pMainFrameView->paint( &webCoreContext, dirtyRect );
            webCoreContext.restore();
            
            CGContextRelease( cgContext );
            CGColorSpaceRelease( colorSpace );
		}
	}
  
    void WebViewImpl::platformSelectionChanged()
    {
    }   
}


