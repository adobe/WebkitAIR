/*
 * Copyright (C) 2003, 2004, 2005, 2006 Apple Computer, Inc.  All rights reserved.
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
#include "GraphicsContext.h"

#include <WebKitApollo/WebBitmap.h>

#if OS(WINDOWS) ||  PLATFORM(APOLLO_UNIX)
#include "GraphicsContextPlatformPrivateCairo.h"
#elif OS(DARWIN) 	
#include <ApplicationServices/ApplicationServices.h>
#include "GraphicsContextPlatformPrivateCG.h"
#endif


namespace WebCore {




void GraphicsContext::drawFocusRing(const Vector<Path>& /*paths*/, int /*width*/, int /*offset*/, const Color& /*color*/)
{
    // FIXME: implement
}

void GraphicsContext::drawFocusRing(const Vector<IntRect>& rects, int /*width*/, int /*offset*/, const Color& /*color*/)
{
    if (paintingDisabled() || (rects.size() == 0)) {
        return;
    }

    RefPtr<Image> image = Image::loadPlatformResource( "focusRing" );
      
    IntSize stateOffset( 0, 0 );
    IntRect r = rects[0]; //FIXME: support multiple rectangles.

    r.inflate( 1 );
    
    IntRect destRect( 0, 0, 0, 0 );
    IntRect srcRect( 0, 0, 0, 0 );
    IntRect focusRingSrcRect( 0, 0, 0, 0 );

    int cornerDestWidth        = 6;
    int cornerDestHeight    = 5;

    // top left
    srcRect = IntRect( 0, 0, 6, 5 );
    destRect = IntRect( r.x(), r.y(), cornerDestWidth, cornerDestHeight );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // top right
    srcRect = IntRect( 18, 0, 6, 5 );
    destRect = IntRect( r.x() + (r.width() - cornerDestWidth), r.y(), cornerDestWidth, cornerDestHeight );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // bottom left
    srcRect = IntRect( 0, 17, 6, 5 );
    destRect = IntRect( r.x(), r.y() + (r.height() - cornerDestHeight), cornerDestWidth, cornerDestHeight );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // bottom right
    srcRect = IntRect( 18, 17, 6, 5 );
    destRect = IntRect( r.x() + (r.width() - cornerDestWidth), r.y() + (r.height() - cornerDestHeight), cornerDestWidth, cornerDestHeight );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // top edge
    srcRect = IntRect( 7, 0, 8, 5 );
    destRect = IntRect( r.x() + cornerDestWidth, r.y(), (r.width() - (2 * cornerDestWidth)), cornerDestHeight );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // bottom edge
    srcRect = IntRect( 7, 17, 8, 5 );
    destRect = IntRect( r.x() + cornerDestWidth, r.y() + (r.height() - cornerDestHeight), (r.width() - (2 * cornerDestWidth)), cornerDestHeight );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // left edge
    srcRect = IntRect( 0, 6, 6, 8 );
    destRect = IntRect( r.x(), r.y() + cornerDestHeight, cornerDestWidth, (r.height() - (2 * cornerDestHeight)) );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // right edge
    srcRect = IntRect( 18, 6, 6, 8 );
    destRect = IntRect( r.x() + (r.width() - cornerDestWidth), r.y() + cornerDestHeight , cornerDestWidth, (r.height() - (2 * cornerDestHeight)) );
    srcRect.move( stateOffset );
    drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
}

#if OS(WINDOWS) ||  PLATFORM(APOLLO_UNIX)
GraphicsContext::GraphicsContext(PlatformGraphicsContext* cr, WebBitmap* pTargetBitmap)
    : m_common(createGraphicsContextPrivate())
    , m_data(new GraphicsContextPlatformPrivate)
{
    m_data->cr = cairo_reference(cr);
    m_data->bitmap = pTargetBitmap;
    setPaintingDisabled(!cr);
}

GraphicsContext::GraphicsContext(WebBitmap* pTargetBitmap, const WebIntRect* const pPaintRect)
    : m_common(createGraphicsContextPrivate())
    , m_data(new GraphicsContextPlatformPrivate)
{
    if (pTargetBitmap)
    {
        ASSERT(pTargetBitmap->m_pVTable);

        unsigned char* const pTargetPixelData = reinterpret_cast<unsigned char*>( pTargetBitmap->m_pVTable->getPixelData( pTargetBitmap ) );
        int const targetBitmapWidth = static_cast< int >( pTargetBitmap->m_pVTable->getWidth( pTargetBitmap ) );
        int const targetBitmapHeight = static_cast< int >( pTargetBitmap->m_pVTable->getHeight( pTargetBitmap ) );
        int const targetBitmapStride = static_cast< int >( pTargetBitmap->m_pVTable->getStride( pTargetBitmap ) );

        cairo_surface_t* const pTargetSurface = cairo_image_surface_create_for_data(    pTargetPixelData,
                                                                                        CAIRO_FORMAT_ARGB32,
                                                                                        targetBitmapWidth,
                                                                                        targetBitmapHeight,
                                                                                        targetBitmapStride );
        cairo_t* pCairoContext = cairo_create( pTargetSurface );
        cairo_rectangle( pCairoContext, pPaintRect->m_left, pPaintRect->m_top, pPaintRect->m_right - pPaintRect->m_left, pPaintRect->m_bottom - pPaintRect->m_top );
        cairo_clip( pCairoContext );

        static cairo_font_options_t* fontOptions = cairo_font_options_create();
#if OS(WINDOWS)
		// In Apollo use the anti-aliasing mode selected by the user. See bug 2404418.
		cairo_antialias_t antialias = CAIRO_ANTIALIAS_DEFAULT;
#elif PLATFORM(APOLLO_UNIX)
		cairo_antialias_t antialias = CAIRO_ANTIALIAS_SUBPIXEL;
#endif
		cairo_font_options_set_antialias(fontOptions, antialias);
        cairo_set_font_options( pCairoContext, fontOptions );

		m_data->cr = cairo_reference(pCairoContext);
    }
    else
        setPaintingDisabled(true);

	m_data->bitmap = pTargetBitmap;
}

// #endif OS(WINDOWS) || (OS(UNIX) && !OS(DARWIN))
#elif OS(DARWIN) 	
GraphicsContext::GraphicsContext(WebBitmap* pTargetBitmap, const WebIntRect* const pPaintRect)
	: m_common(createGraphicsContextPrivate())
    , m_data(0)
{
	if (pTargetBitmap)
	{
		ASSERT(pTargetBitmap->m_pVTable);
		
		unsigned char* const pTargetPixelData = reinterpret_cast<unsigned char*>( pTargetBitmap->m_pVTable->getPixelData( pTargetBitmap ) );
		int const targetBitmapWidth = static_cast< int >( pTargetBitmap->m_pVTable->getWidth( pTargetBitmap ) );
		int const targetBitmapHeight = static_cast< int >( pTargetBitmap->m_pVTable->getHeight( pTargetBitmap ) );
		int const targetBitmapStride = static_cast< int >( pTargetBitmap->m_pVTable->getStride( pTargetBitmap ) );
		
		CGColorSpaceRef const colorSpace = CGColorSpaceCreateWithName ( kCGColorSpaceGenericRGB );
		
		CGContextRef const cgContext = CGBitmapContextCreate(	pTargetPixelData,
															 targetBitmapWidth,
															 targetBitmapHeight,
															 8,
															 targetBitmapStride,
															 colorSpace,
															 kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host );
		
		m_data = new GraphicsContextPlatformPrivate(cgContext);
		
		WebCore::IntRect dirtyRect(	pPaintRect->m_left
								   , pPaintRect->m_top
								   , pPaintRect->m_right - pPaintRect->m_left
								   , pPaintRect->m_bottom - pPaintRect->m_top );
		clip(dirtyRect);

	}
	else
		setPaintingDisabled(true);
		
}
#endif

}
