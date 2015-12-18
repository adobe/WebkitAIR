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
#include "GraphicsContext.h"
#include "GraphicsContextPlatformPrivateCairo.h"

#if PLATFORM(APOLLO)

namespace WebCore {

HDC GraphicsContext::getWindowsContext(const IntRect& dstRect, bool supportAlphaBlend, bool mayCreateBitmap)
{
        if (dstRect.isEmpty())
            return 0;

        if(!m_data->hdc_refcount++)
        {
            m_data->hdc_surface = cairo_win32_surface_create_with_dib(CAIRO_FORMAT_RGB24, dstRect.width(), dstRect.height());

            // see http://cairographics.org/FAQ/#paint_from_a_surface
            cairo_surface_t* source = cairo_get_target( m_data->cr );
            cairo_t *cr = cairo_create ( m_data->hdc_surface );
            cairo_set_source_surface (cr, source, -dstRect.x(), -dstRect.y() );
            cairo_paint(cr);
            cairo_destroy(cr);

            m_data->m_hdc = cairo_win32_surface_get_dc(m_data->hdc_surface);

            SaveDC(m_data->m_hdc);
        }

        HDC hdc = m_data->m_hdc;
        return hdc;
}

}

#endif

