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
#include "ScrollView.h"

#if PLATFORM(APOLLO)

using std::min;
using std::max;

namespace WebCore {

bool calculateScroll(ScrollDirection direction, ScrollGranularity granularity, ScrollbarOrientation orientation,
                     int currentPos, int totalSize, int visibleSize, int pageStep, int& newPos);

bool calculateScroll(ScrollDirection direction, ScrollGranularity granularity, ScrollbarOrientation orientation,
                 int currentPos, int totalSize, int visibleSize, int pageStep, int& newPos)
{
    static const int lineStep = Scrollbar::pixelsPerLineStep();
    static const int pixelStep = 1;

    float multiplier = 1.0f;
    float step = 0;
    if ((direction == ScrollUp && orientation == VerticalScrollbar) || (direction == ScrollLeft && orientation == HorizontalScrollbar))
        step = -1;
    else if ((direction == ScrollDown && orientation == VerticalScrollbar) || (direction == ScrollRight && orientation == HorizontalScrollbar)) 
        step = 1;
    
    if (granularity == ScrollByLine)
        step *= lineStep;
    else if (granularity == ScrollByPage)
        step *= pageStep;
    else if (granularity == ScrollByDocument)
        step *= totalSize;
    else if (granularity == ScrollByPixel)
        step *= pixelStep;
        
    float floatNewPos = currentPos + step * multiplier;
    float maxPos = totalSize - visibleSize;
    newPos = lroundf(max(min(floatNewPos, maxPos), 0.0f));

    return (newPos!=currentPos);
}

bool ScrollView::scrollRootContent(ScrollDirection direction, ScrollGranularity granularity)
{
    IntSize newOffset = m_scrollOffset;
    
    int pageStep = max(max<int>(visibleWidth() * Scrollbar::minFractionToStepWhenPaging(), visibleWidth() - Scrollbar::maxOverlapBetweenPages()), 1);

    if (pageStep < 0)
        pageStep = visibleWidth();

    int newValue;

    if(calculateScroll(direction, granularity, HorizontalScrollbar, m_scrollOffset.width(), 
        contentsWidth(), visibleWidth(), pageStep, newValue))
    {
        newOffset.setWidth(newValue);

    }else if(calculateScroll(direction, granularity, VerticalScrollbar, m_scrollOffset.height(), 
        contentsHeight(), visibleHeight(), pageStep, newValue))
    {
        newOffset.setHeight(newValue);
    }

    IntSize scrollDelta = newOffset - m_scrollOffset;
    if (scrollDelta == IntSize())
        return false;
    updateScrollbars(newOffset);
    return true;
}

}

#endif

