/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
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
#include "BitmapImage.h"

#include "FloatRect.h"
#include "GraphicsContext.h"
#include "PlatformString.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "FrameLoaderClientApollo.h"
#include <WebKitApollo/WebKit.h>

#if PLATFORM(CG)
#include <ApplicationServices/ApplicationServices.h>
#elif PLATFORM(CAIRO)
#include <cairo.h>
#endif

namespace WebCore {

WebBitmap* BitmapImage::getWebBitmap(Frame* frame)
{
    ASSERT(frame);
    FrameLoaderClientApollo* clientApollo = FrameLoaderClientApollo::clientApollo(frame);
    WebHost* webHost = clientApollo->webHost();
    ASSERT(webHost);

    IntSize imageSize = size();
    WebBitmap* bitmap = webHost->m_pVTable->createBitmap(webHost, (unsigned long) imageSize.width(), (unsigned long) imageSize.height());

    void *pixelData = bitmap->m_pVTable->getPixelData(bitmap);
    unsigned long stride = bitmap->m_pVTable->getStride(bitmap);
    memset( pixelData, 0, stride * imageSize.height() );

    // Render bitmap image into web bitmap.
#if PLATFORM(CG)
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGContextRef context = CGBitmapContextCreate(pixelData, imageSize.width(), imageSize.height(), 8, stride, colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
    CGRect rect = CGRectMake(0, 0, imageSize.width(), imageSize.height());
    CGContextDrawImage(context, rect, frameAtIndex(0));
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
#elif PLATFORM(CAIRO)
    cairo_surface_t* surface = cairo_image_surface_create_for_data((unsigned char *) pixelData, CAIRO_FORMAT_ARGB32, imageSize.width(), imageSize.height(), stride);
    cairo_t* context = cairo_create(surface);
    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(frameAtIndex(0));
    cairo_set_source(context, pattern);
    cairo_matrix_t patternMatrix;
#if PLATFORM(APOLLO_UNIX)
    /*
     * The image on the surface has the correct orientation, however
     * by applying the matrix incorrectly inverts the image. This works
     * fine on Windows as the underlying bitmap object expects it to be
     * upside down. However it fails on Linux which uses a GdkPixbuf
     * underneath and that expects the image in the correct orientation.
     */
    cairo_matrix_init(&patternMatrix, 1, 0, 0, 1, 0, 0);
#else
    cairo_matrix_init(&patternMatrix, 1, 0, 0, -1, 0, imageSize.height());
#endif
    cairo_pattern_set_matrix(pattern, &patternMatrix);
    cairo_rectangle(context, 0, 0, imageSize.width(), imageSize.height());
    cairo_fill(context);
    cairo_destroy(context);
    cairo_pattern_destroy(pattern);
    cairo_surface_destroy(surface);
#endif

    return bitmap;
}

}
