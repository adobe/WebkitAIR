/*
* Copyright (C) 2009 Adobe Systems Incorporated.  All rights reserved.
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
#include "ScrollbarThemeApollo.h"
#include "PlatformMouseEvent.h"
#include "Scrollbar.h"
#include "ImageBuffer.h"
#include "wtf/Vector.h"
#include "wtf/HashMap.h"
#include "wtf/HashSet.h"

namespace WebCore {

static HashSet<Scrollbar*>* gScrollbars;

//=============================================================================

//static const int s_DefaultScrollBarWidth = 15;
static const int s_DefaultScrollBarHeight = 15;
//static const int s_DefaultScrollBarButtonSize = 15;
//static const int s_MinThumbSize = 6;

//=============================================================================


ScrollbarTheme* ScrollbarTheme::nativeTheme()
{
    static ScrollbarThemeApollo theme;
    return &theme;
}

ScrollbarThemeApollo::ScrollbarThemeApollo()
{

}

ScrollbarThemeApollo::~ScrollbarThemeApollo()
{
}


void ScrollbarThemeApollo::registerScrollbar(Scrollbar* scrollbar)
{
    if (!gScrollbars)
        gScrollbars = new HashSet<Scrollbar*>;
    gScrollbars->add(scrollbar);
}

void ScrollbarThemeApollo::unregisterScrollbar(Scrollbar* scrollbar)
{
    gScrollbars->remove(scrollbar);
    if (gScrollbars->isEmpty()) {
        delete gScrollbars;
        gScrollbars = 0;
    }
}



int ScrollbarThemeApollo::scrollbarThickness(ScrollbarControlSize)
{
    return s_DefaultScrollBarHeight;
}

void ScrollbarThemeApollo::themeChanged()
{

}

bool ScrollbarThemeApollo::invalidateOnMouseEnterExit()
{
//    ASSERT(false);
    return true;
}

bool ScrollbarThemeApollo::hasThumb(Scrollbar* scrollbar)
{
    // This method is just called as a paint-time optimization to see if
    // painting the thumb can be skipped.  We don't have to be exact here.
    return thumbLength(scrollbar) > 0;
}

IntRect ScrollbarThemeApollo::backButtonRect(Scrollbar* scrollbar, ScrollbarPart part, bool /*painting*/)
{
    // Windows and Linux just have single arrows.
    if (part == BackButtonEndPart)
        return IntRect();

    IntSize size = buttonSize(scrollbar);
    return IntRect(scrollbar->x(), scrollbar->y(), size.width(), size.height());
}

IntRect ScrollbarThemeApollo::forwardButtonRect(Scrollbar* scrollbar, ScrollbarPart part, bool /*painting*/)
{
    // Windows and Linux just have single arrows.
    if (part == ForwardButtonStartPart)
        return IntRect();

    IntSize size = buttonSize(scrollbar);
    int x, y;
    if (scrollbar->orientation() == HorizontalScrollbar) {
        x = scrollbar->x() + scrollbar->width() - size.width();
        y = scrollbar->y();
    } else {
        x = scrollbar->x();
        y = scrollbar->y() + scrollbar->height() - size.height();
    }
    return IntRect(x, y, size.width(), size.height());
}

IntRect ScrollbarThemeApollo::trackRect(Scrollbar* scrollbar, bool /*painting*/)
{
    IntSize bs = buttonSize(scrollbar);
    int thickness = scrollbarThickness();
    if (scrollbar->orientation() == HorizontalScrollbar) {
        if (scrollbar->width() < 2 * thickness)
            return IntRect();
        return IntRect(scrollbar->x() + bs.width(), scrollbar->y(), scrollbar->width() - 2 * bs.width(), thickness);
    }
    if (scrollbar->height() < 2 * thickness)
        return IntRect();
    return IntRect(scrollbar->x(), scrollbar->y() + bs.height(), thickness, scrollbar->height() - 2 * bs.height());
}

IntSize ScrollbarThemeApollo::buttonSize(Scrollbar* scrollbar)
{
    // Our desired rect is essentially thickness by thickness.

    // Our actual rect will shrink to half the available space when we have < 2
    // times thickness pixels left.  This allows the scrollbar to scale down
    // and function even at tiny sizes.

    int thickness = scrollbarThickness();

    if (scrollbar->orientation() == HorizontalScrollbar) {
        int width = scrollbar->width() < 2 * thickness ? scrollbar->width() / 2 : thickness;
        return IntSize(width, thickness);
    }

    int height = scrollbar->height() < 2 * thickness ? scrollbar->height() / 2 : thickness;
    return IntSize(thickness, height);
} 

bool ScrollbarThemeApollo::shouldCenterOnThumb(Scrollbar*, const PlatformMouseEvent& evt)
{
    return evt.shiftKey() && evt.button() == LeftButton;
}

void ScrollbarThemeApollo::paintTrackBackground(GraphicsContext* context, Scrollbar* scrollbar, const IntRect& rect)
{
    // Just assume a forward track part.  We only paint the track as a single piece when there is no thumb.
    if (!hasThumb(scrollbar))
        paintTrackPiece(context, scrollbar, rect, ForwardTrackPart);
}

void ScrollbarThemeApollo::paintTrackPiece(GraphicsContext* gCtx, Scrollbar* scrollbar, const IntRect& rect, ScrollbarPart part)
{
    paintThemeImage(gCtx, scrollbar, rect, part);
}
    
void ScrollbarThemeApollo::paintButton(GraphicsContext* gCtx, Scrollbar* scrollbar, const IntRect& rect, ScrollbarPart part)
{
    paintThemeImage(gCtx, scrollbar, rect, part);
}

void ScrollbarThemeApollo::paintThumb(GraphicsContext* gCtx, Scrollbar* scrollbar, const IntRect& rect)
{
    paintThemeImage(gCtx, scrollbar, rect, ThumbPart);
}

enum ScrollbarPartApollo {
    // these should be kept in sync with ScrollbarPart from ScrollTypes.h
    
    // these are for the thumb part
    ThumbPartTop = 1 << 9,
    ThumbPartMiddle = 1 << 10,
    ThumbPartBottom = 1 << 11,
    ThumbPartGrip = 1 << 12
};

enum PartState
{
    PartStateEnabled = 0,
    PartStateDisabled = 1,
    PartStateDown = 2,
    PartStateHover = 3
};

typedef WTF::HashMap<int, WTF::RefPtr<Image> > ScrollbarCroppedImages;
typedef WTF::Vector<ScrollbarCroppedImages, 4> ScrollbarImages;

class ScrollbarImagesHelper: public RefCounted<ScrollbarImagesHelper>
{
public:
    ScrollbarImagesHelper()
        : m_horz(4)
        , m_vert(4)
    {
        initImages();
    }

    ScrollbarImages* horizontal() { return &m_horz; }
    ScrollbarImages* vertical() { return &m_vert; }

private:

    PassRefPtr<Image> getSubimage(Image* image, IntRect& rect)
    {
        OwnPtr<ImageBuffer> result = ImageBuffer::create(rect.size());        
        result->context()->drawImage(image, DeviceColorSpace, IntPoint(), rect);
        return PassRefPtr<Image>(result->image());
    }

    void getSubimages(ScrollbarImages* data, int part, Image* image, IntRect srcRect, IntSize& disabledOffset, IntSize& downOffset, IntSize& overOffset)
    {
        
        IntRect rect = srcRect;
        data->at(PartStateEnabled).add(part, getSubimage(image, rect));
        
        rect = srcRect;
        rect.move(disabledOffset);
        data->at(PartStateDisabled).add(part, getSubimage(image, rect));
        
        rect = srcRect;
        rect.move(downOffset);
        data->at(PartStateDown).add(part, getSubimage(image, rect));

        rect = srcRect;
        rect.move(overOffset);
        data->at(PartStateHover).add(part, getSubimage(image, rect));
        
    }

    void initImages()
    {
        RefPtr<Image> imageHorz = Image::loadPlatformResource( "scrollBarStatesHorz" );
        RefPtr<Image> imageVert = Image::loadPlatformResource( "scrollBarStatesVert" );
        
        // vertical
        Image* current = imageVert.get();
        ScrollbarImages* currentList = &m_vert;

        IntSize disabledOffset( 48, 0 );  // index 1
        IntSize downOffset( 32, 0 );      // index 2
        IntSize overOffset( 16, 0 );      // index 3

#define PART_REGION(part, rect) \
        getSubimages(currentList, static_cast<int>(part), current, IntRect rect, disabledOffset, downOffset, overOffset);

        PART_REGION(BackButtonStartPart, (0, 0, 15, 15))
        PART_REGION(ForwardButtonStartPart, (0, 0, 15, 15))

        PART_REGION(BackTrackPart, (0, 29, 15, 3))
        PART_REGION(ForwardTrackPart, (0, 29, 15, 3))

        PART_REGION(BackButtonEndPart, (0, 29, 15, 15))
        PART_REGION(ForwardButtonEndPart, (0, 29, 15, 15))
        
        PART_REGION(ThumbPartTop, (0, 16, 15, 1))
        PART_REGION(ThumbPartMiddle, (0, 22, 15, 3))
        PART_REGION(ThumbPartBottom, (0, 27, 15, 1))
        PART_REGION(ThumbPartGrip, (0, 18, 15, 3))
        
        // horizontal
        current = imageHorz.get();
        currentList = &m_horz;

        disabledOffset = IntSize( 0, 48 );  // index 1
        downOffset = IntSize( 0, 32 );      // index 2
        overOffset = IntSize( 0, 16 );      // index 3

        PART_REGION(BackButtonStartPart, (0, 0, 15, 15))
        PART_REGION(ForwardButtonStartPart, (0, 0, 15, 15))

        PART_REGION(BackTrackPart, (29, 0, 3, 15))
        PART_REGION(ForwardTrackPart, (29, 0, 3, 15))

        PART_REGION(BackButtonEndPart, (29, 0, 15, 15))
        PART_REGION(ForwardButtonEndPart, (29, 0, 15, 15))
        
        PART_REGION(ThumbPartTop, (16, 0, 1, 15))
        PART_REGION(ThumbPartMiddle, (22, 0, 3, 15))
        PART_REGION(ThumbPartBottom, (27, 0, 1, 15))
        PART_REGION(ThumbPartGrip, (18, 0, 3, 15))

#undef PART_REGION
    }

    ScrollbarImages m_horz;
    ScrollbarImages m_vert;
};

void ScrollbarThemeApollo::paintThemeImage(GraphicsContext* gCtx, Scrollbar* scrollbar, const IntRect& rect, ScrollbarPart part)
{
    static RefPtr<ScrollbarImagesHelper> images(adoptRef(new ScrollbarImagesHelper()));
    
    if (part == NoPart || part == ScrollbarBGPart || part == TrackBGPart || part == AllParts) {
        ASSERT(false);
        return;
    }

    ScrollbarImages* scrollbarImages;

    if (scrollbar->orientation() == VerticalScrollbar)
        scrollbarImages = images->vertical();
    else
        scrollbarImages = images->horizontal();

    int state = PartStateEnabled;

    if (!scrollbar->enabled())
        state = PartStateDisabled;
    else if ( scrollbar->pressedPart() == part ) 
        state = PartStateDown;
    else if ( scrollbar->hoveredPart() == part )
        state = PartStateHover;

    ScrollbarCroppedImages* stateImages = &scrollbarImages->at(state);

    if (part != ThumbPart) {

        Image* image = stateImages->get(part).get();
        gCtx->drawImage(image, DeviceColorSpace, rect);

    } else {
        IntRect thumbTopDestRect = rect;
        IntRect thumbBottomDestRect = rect;
        IntRect thumbGripDestRect = rect;
        
        // set up for vertical 
        if (scrollbar->orientation() == VerticalScrollbar)
        {
            thumbTopDestRect.setHeight( 1 );

            thumbBottomDestRect.setY( thumbBottomDestRect.bottom() - 1 );
            thumbBottomDestRect.setHeight( 1 );

            thumbGripDestRect.setY( rect.y() + ((thumbGripDestRect.height() / 2) - ( 3 / 2 )) );
            thumbGripDestRect.setHeight( 3 );

        } else {
            thumbTopDestRect.setWidth( 1 );

            thumbBottomDestRect.setX( thumbBottomDestRect.right() - 1 );
            thumbBottomDestRect.setWidth( 1 );

            thumbGripDestRect.setX( rect.x() + ((thumbGripDestRect.width() / 2) - ( 3 / 2 )) );
            thumbGripDestRect.setWidth( 3 );
        }

        Image* image = stateImages->get(ThumbPartMiddle).get();
        gCtx->drawImage(image, DeviceColorSpace, rect);

        image = stateImages->get(ThumbPartTop).get();
        gCtx->drawImage(image, DeviceColorSpace, thumbTopDestRect);

        image = stateImages->get(ThumbPartBottom).get();
        gCtx->drawImage(image, DeviceColorSpace, thumbBottomDestRect);

        if ( rect.height() >= 14 ) {
            image = stateImages->get(ThumbPartGrip).get();
            gCtx->drawImage( image, DeviceColorSpace, thumbGripDestRect);
        }
    }

}

}
