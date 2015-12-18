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
#include "Gradient.h"

#if PLATFORM(APOLLO)

namespace WebCore {

static bool compareColorStops(const Gradient::ColorStop& a, const Gradient::ColorStop& b)
{
    return a.stop < b.stop;
}

cairo_pattern_t* Gradient::platformGradient()
{
    if (m_gradient)
        return m_gradient;

    // cairo_pattern_create_radial does not perform as expected if the first radius is
    // bigger than the second radius. If this is the case, we should switch the end point as
    // well as the radiuses
    bool switchEndPointsForRadialGradient = false;
    if (m_radial) {
        if (m_r0 <= m_r1) {
            m_gradient = cairo_pattern_create_radial(m_p0.x(), m_p0.y(), m_r0, m_p1.x(), m_p1.y(), m_r1);
        }
        else {
            m_gradient = cairo_pattern_create_radial(m_p1.x(), m_p1.y(), m_r1, m_p0.x(), m_p0.y(), m_r0);
            switchEndPointsForRadialGradient = true;
        }
    }
    else
        m_gradient = cairo_pattern_create_linear(m_p0.x(), m_p0.y(), m_p1.x(), m_p1.y());

    if (switchEndPointsForRadialGradient) {
        // if we switch the end points for radial gradients, than we have to modify the color stop offsets to reverse the interpolation
        // switching only the first() and end() of the m_stops does not achieve the expected result since there may be also
        // intermediate color stops that must be modified also
        Vector<ColorStop>::iterator stopIter = m_stops.begin();
        while (stopIter != m_stops.end()) {
            stopIter->stop = 1.0 - stopIter->stop;
            ++stopIter;
        }

        // although cairo itself knows how to deal with unsorted stops, it is better to sort them
        // stable_sort grants that the relative order of the elements with equivalent values is preserved.
        // this is important, since for the same stop values, the order of colors must be preserved
        if (m_stops.size())
            std::stable_sort(m_stops.begin(), m_stops.end(), compareColorStops);
    }

    Vector<ColorStop>::iterator stopIterator = m_stops.begin();
    while (stopIterator != m_stops.end()) {
        cairo_pattern_add_color_stop_rgba(m_gradient, stopIterator->stop, stopIterator->red, stopIterator->green, stopIterator->blue, stopIterator->alpha);
        ++stopIterator;
    }

    switch (m_spreadMethod) {
    case SpreadMethodPad:
        cairo_pattern_set_extend(m_gradient, CAIRO_EXTEND_PAD);
        break;
    case SpreadMethodReflect:
        cairo_pattern_set_extend(m_gradient, CAIRO_EXTEND_REFLECT);
        break;
    case SpreadMethodRepeat:
        cairo_pattern_set_extend(m_gradient, CAIRO_EXTEND_REPEAT);
        break;
    }

    cairo_matrix_t matrix = m_gradientSpaceTransformation;
    cairo_matrix_invert(&matrix);
    cairo_pattern_set_matrix(m_gradient, &matrix);

    return m_gradient;
}


}

#endif

