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
#include "RenderThemeApollo.h"
#include <Document.h>
#include <GraphicsContext.h>

#if PLATFORM(APOLLO_UNIX)
//needed for defining 'cielf'
#include <math.h>
#endif

namespace WebCore {

const float baseFontSize = 11.0f;
const float baseArrowWidth = 5.0f;
#if 0
const float baseArrowHeight = 4.0f;
const float baseSpaceBetweenArrows = 2.0f;
#endif
const int arrowPaddingLeft = 6;
const int arrowPaddingRight = 6;
    
RenderTheme* theme();
RenderTheme* theme()
{
    static RenderThemeApollo apolloTheme;
    return &apolloTheme;
}
    
PassRefPtr<RenderTheme> RenderTheme::themeForPage(Page*)
{
    return adoptRef(new RenderThemeApollo());
}    

RenderThemeApollo::RenderThemeApollo()
{
}

RenderThemeApollo::~RenderThemeApollo()
{
}

Color RenderThemeApollo::platformActiveSelectionBackgroundColor() const
{
    return Color(187, 220, 242, 255); // Light blue
}

Color RenderThemeApollo::platformInactiveSelectionBackgroundColor() const
{
    return Color(200, 200, 200, 255); // Light gray, matching light blue above
}

Color RenderThemeApollo::platformActiveSelectionForegroundColor() const
{
    return Color(45, 45, 45, 255); // Dark gray, readable on light blue
}

Color RenderThemeApollo::platformInactiveSelectionForegroundColor() const
{
    return Color(91, 91, 91, 255); // Medium gray, readable on light blue
}


void RenderThemeApollo::adjustButtonStyle(CSSStyleSelector*, RenderStyle* /*style*/, Element*) const
{
}

bool RenderThemeApollo::paintCheckbox(RenderObject* /*o*/, const RenderObject::PaintInfo& /*i*/, const IntRect& /*r*/)
{
    return true;
}

IntRect getFocusRingSrcRect( const IntRect& srcRect, const IntSize& focusOffset );
IntRect getFocusRingSrcRect( const IntRect& srcRect, const IntSize& focusOffset )
{
    IntRect focusRingSrcRect = srcRect;
    focusRingSrcRect.move( focusOffset );
    return focusRingSrcRect;
}

bool RenderThemeApollo::paintButton(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    RefPtr<Image> image = Image::loadPlatformResource( "htmlButtonStates" );
    
    IntSize stateOffset( 0, 0 );
    if ( !isEnabled( o ) )
    {
        stateOffset = IntSize( 0, 66 ); 
    }
    else if ( isPressed( o ) )
    {
        stateOffset = IntSize( 0, 44 ); 
    }
    else if ( isHovered( o ) )
    {
        stateOffset = IntSize( 0, 22 ); 
    }
    IntSize focusOffset = IntSize( 0, 88 );
    const bool nodeIsFocused = isFocused( o );

    IntRect destRect( 0, 0, 0, 0 );
    IntRect srcRect( 0, 0, 0, 0 );
    IntRect focusRingSrcRect( 0, 0, 0, 0 );

    int cornerDestWidth        = 6;
    int cornerDestHeight    = 5;

    // top left
    srcRect = IntRect( 0, 0, 6, 5 );
    destRect = IntRect( r.x(), r.y(), cornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // top right
    srcRect = IntRect( 18, 0, 6, 5 );
    destRect = IntRect( r.x() + (r.width() - cornerDestWidth), r.y(), cornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // bottom left
    srcRect = IntRect( 0, 17, 6, 5 );
    destRect = IntRect( r.x(), r.y() + (r.height() - cornerDestHeight), cornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // bottom right
    srcRect = IntRect( 18, 17, 6, 5 );
    destRect = IntRect( r.x() + (r.width() - cornerDestWidth), r.y() + (r.height() - cornerDestHeight), cornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // top edge
    srcRect = IntRect( 7, 0, 8, 5 );
    destRect = IntRect( r.x() + cornerDestWidth, r.y(), (r.width() - (2 * cornerDestWidth)), cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // bottom edge
    srcRect = IntRect( 7, 17, 8, 5 );
    destRect = IntRect( r.x() + cornerDestWidth, r.y() + (r.height() - cornerDestHeight), (r.width() - (2 * cornerDestWidth)), cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 


    // left edge
    srcRect = IntRect( 0, 6, 6, 8 );
    destRect = IntRect( r.x(), r.y() + cornerDestHeight, cornerDestWidth, (r.height() - (2 * cornerDestHeight)) );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // right edge
    srcRect = IntRect( 18, 6, 6, 8 );
    destRect = IntRect( r.x() + (r.width() - cornerDestWidth), r.y() + cornerDestHeight , cornerDestWidth, (r.height() - (2 * cornerDestHeight)) );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        i.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // center
    srcRect = IntRect( 7, 6, 8, 8 );
    destRect = IntRect( r.x() + cornerDestWidth, r.y() + cornerDestHeight , r.width() - (2 * cornerDestWidth), (r.height() - (2 * cornerDestHeight)) );
    srcRect.move( stateOffset );
    i.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    return false;
}

void RenderThemeApollo::setCheckboxSize(RenderStyle* style) const
{
    // If the width and height are both specified, then we have nothing to do.
    if (!style->width().isIntrinsicOrAuto() && !style->height().isAuto())
        return;

    // FIXME:  A hard-coded size of 15 is used.
    // At different DPI settings on Windows, querying the theme gives you a larger size that accounts for
    // the higher DPI.  Until our entire engine honors a DPI setting other than 96, we can't rely on the theme's
    // metrics.
    if (style->width().isIntrinsicOrAuto())
        style->setWidth(Length(15, Fixed));
    if (style->height().isAuto())
        style->setHeight(Length(15, Fixed));
}

void RenderThemeApollo::setRadioSize(RenderStyle* style) const
{
    // If the width and height are both specified, then we have nothing to do.
    if (!style->width().isIntrinsicOrAuto() && !style->height().isAuto())
        return;

    if (style->width().isIntrinsicOrAuto())
        style->setWidth(Length(15, Fixed));
    if (style->height().isAuto())
        style->setHeight(Length(16, Fixed));
}

void RenderThemeApollo::adjustTextFieldStyle(CSSStyleSelector*, RenderStyle* /*style*/, Element*) const
{
}

bool RenderThemeApollo::paintTextField(RenderObject* /*o*/, const RenderObject::PaintInfo& /*i*/, const IntRect& /*r*/)
{
    return true;
}

void RenderThemeApollo::adjustTextAreaStyle(CSSStyleSelector*, RenderStyle* /*style*/, Element*) const
{
}

bool RenderThemeApollo::paintTextArea(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    return paintTextField(o, i, r);
}

RenderPopupMenu* RenderThemeApollo::createPopupMenu(RenderArena* /*arena*/, Document* /*doc*/, RenderMenuList* /*menuList*/)
{
    RenderPopupMenu* retVal = 0;
#if 0
    if ( doc )
    {
        retVal = new (arena) RenderPopupMenuApollo(doc, menuList);
    }
#endif
    return retVal;
}

void RenderThemeApollo::systemFont(int /*propId*/, FontDescription& /*fontDescription*/) const {}

#if 0
float RenderThemeApollo::getMenuListButtonWidth( RenderObject * o ) const
{
    float fontScale = o->style()->fontSize() / baseFontSize;
    return (baseArrowWidth * fontScale) + arrowPaddingRight + arrowPaddingLeft;
}
#endif

namespace {
static const int paddingBeforeSeparator = 4;
static const int styledPopupPaddingLeft = 8;
static const int styledPopupPaddingTop = 1;
static const int styledPopupPaddingBottom = 2;
}

void RenderThemeApollo::adjustMenuListButtonStyle(CSSStyleSelector* /*selector*/, RenderStyle* style, Element* /*e*/) const
{
    float fontScale = style->fontSize() / baseFontSize;
    float arrowWidth = baseArrowWidth * fontScale;

    // We're overriding the padding to allow for the arrow control.  WinIE doesn't honor padding on selects, so
    // this shouldn't cause problems on the web.  If IE7 changes that, we should reconsider this.
    style->setPaddingLeft(Length(styledPopupPaddingLeft, Fixed));
    style->setPaddingRight(Length(int(ceilf(arrowWidth + arrowPaddingLeft + arrowPaddingRight + paddingBeforeSeparator)), Fixed));
    style->setPaddingTop(Length(styledPopupPaddingTop, Fixed));
    style->setPaddingBottom(Length(styledPopupPaddingBottom, Fixed));

    const int minHeight = 15;
    style->setMinHeight(Length(minHeight, Fixed));
}

bool RenderThemeApollo::paintMenuListButton(RenderObject * o, const RenderObject::PaintInfo& paintInfo, const IntRect& r) 
{
    paintInfo.context->save();

#if 0 // old "move-to" "line-to" code
    paintInfo.context->setStrokeThickness(1);
    paintInfo.context->setStrokeStyle(SolidStroke);
    
    IntRect bounds = IntRect(r.x() + o->style()->borderLeftWidth(),
                             r.y() + o->style()->borderTopWidth(),
                             r.width() - o->style()->borderLeftWidth() - o->style()->borderRightWidth(),
                             r.height() - o->style()->borderTopWidth() - o->style()->borderBottomWidth());

    float fontScale = o->style()->fontSize() / baseFontSize;
    float centerY = bounds.y() + bounds.height() / 2.0;
    float arrowHeight = baseArrowHeight * fontScale;
    float arrowWidth = baseArrowWidth * fontScale;
    float leftEdge = bounds.right() - arrowPaddingRight - arrowWidth;
    float spaceBetweenArrows = baseSpaceBetweenArrows * fontScale;

    Color buttonColor = o->style()->color();
    if ( o->node()->hovered() )
    {
        buttonColor = Color( 0xff, 0xb5, 0x31 );
    }
    paintInfo.context->setFillColor(buttonColor);
    paintInfo.context->setStrokeColor(buttonColor);

    FloatPoint arrow1[3];
    arrow1[0] = FloatPoint(leftEdge, centerY - spaceBetweenArrows / 2.0);
    arrow1[1] = FloatPoint(leftEdge + arrowWidth, centerY - spaceBetweenArrows / 2.0);
    arrow1[2] = FloatPoint(leftEdge + arrowWidth / 2.0, centerY - spaceBetweenArrows / 2.0 - arrowHeight);

    // Draw the top arrow
    paintInfo.context->drawConvexPolygon(3, arrow1, true);

    FloatPoint arrow2[3];
    arrow2[0] = FloatPoint(leftEdge, centerY + spaceBetweenArrows / 2.0);
    arrow2[1] = FloatPoint(leftEdge + arrowWidth, centerY + spaceBetweenArrows / 2.0);
    arrow2[2] = FloatPoint(leftEdge + arrowWidth / 2.0, centerY + spaceBetweenArrows / 2.0 + arrowHeight);

    // Draw the bottom arrow
    paintInfo.context->drawConvexPolygon(3, arrow2, true);

    Color leftSeparatorColor(0, 0, 0, 40);
    Color rightSeparatorColor(255, 255, 255, 40);
    int separatorSpace = 2;
    int leftEdgeOfSeparator = static_cast<int>(leftEdge - arrowPaddingLeft); // FIXME: Round?

    // Draw the separator to the left of the arrows
    paintInfo.context->setStrokeColor(leftSeparatorColor);
    paintInfo.context->drawLine(IntPoint(leftEdgeOfSeparator, bounds.y()),
                                IntPoint(leftEdgeOfSeparator, bounds.bottom()));

    paintInfo.context->setStrokeColor(rightSeparatorColor);
    paintInfo.context->drawLine(IntPoint(leftEdgeOfSeparator + separatorSpace, bounds.y()),
                                IntPoint(leftEdgeOfSeparator + separatorSpace, bounds.bottom()));

// Debug
//    paintInfo.context->fillRect( r, Color( 0xff, 0x00, 0xff ) );
#endif

    RefPtr<Image> image = Image::loadPlatformResource( "popupStates" );
    
    IntSize stateOffset( 0, 0 );
    if ( !isEnabled( o ) )
    {
        stateOffset = IntSize( 0, 66 ); 
    }
    else if ( isPressed( o ) )
    {
        stateOffset = IntSize( 0, 44 ); 
    }
    else if ( isHovered( o ) )
    {
        stateOffset = IntSize( 0, 22 ); 
    }
    IntSize focusOffset = IntSize( 0, 88 );
    const bool nodeIsFocused = isFocused( o );

    IntRect destRect( 0, 0, 0, 0 );
    IntRect srcRect( 0, 0, 0, 0 );
    IntRect focusRingSrcRect( 0, 0, 0, 0 );

    int leftSideCornerDestWidth        = 5;
    int rightSideCornerDestWidth    = 22;
    int cornerDestHeight            = 4;

    // top left
    srcRect = IntRect( 0, 0, 5, 4 );
    destRect = IntRect( r.x(), r.y(), leftSideCornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // top right
    srcRect = IntRect( 22, 0, 22, 4 );
    destRect = IntRect( r.x() + (r.width() - rightSideCornerDestWidth), r.y(), rightSideCornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // bottom left
    srcRect = IntRect( 0, 18, 5, 4 );
    destRect = IntRect( r.x(), r.y() + (r.height() - cornerDestHeight), leftSideCornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // bottom right
    srcRect = IntRect( 22, 18, 22, 4 );
    destRect = IntRect( r.x() + (r.width() - rightSideCornerDestWidth), r.y() + (r.height() - cornerDestHeight), rightSideCornerDestWidth, cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // top edge
    srcRect = IntRect( 6, 0, 14, 4 );
    destRect = IntRect( r.x() + leftSideCornerDestWidth, r.y(), (r.width() - (leftSideCornerDestWidth + rightSideCornerDestWidth)), cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // bottom edge
    srcRect = IntRect( 6, 18, 14, 4 );
    destRect = IntRect( r.x() + leftSideCornerDestWidth, r.y() + (r.height() - cornerDestHeight), (r.width() - (leftSideCornerDestWidth + rightSideCornerDestWidth)), cornerDestHeight );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // left edge
    srcRect = IntRect( 0, 6, 5, 11 );
    destRect = IntRect( r.x(), r.y() + cornerDestHeight , leftSideCornerDestWidth, (r.height() - (2 * cornerDestHeight)) );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // right edge
    srcRect = IntRect( 22, 6, 22, 11 );
    focusRingSrcRect = getFocusRingSrcRect( srcRect, focusOffset );    
    destRect = IntRect( r.x() + (r.width() - rightSideCornerDestWidth), r.y() + cornerDestHeight , rightSideCornerDestWidth, (r.height() - (2 * cornerDestHeight)) );
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 
    if ( nodeIsFocused ) 
        paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, focusRingSrcRect ); 

    // center
    srcRect = IntRect( 6, 6, 11, 11 );
    destRect = IntRect( r.x() + leftSideCornerDestWidth, r.y() + cornerDestHeight , r.width() - (leftSideCornerDestWidth + rightSideCornerDestWidth), (r.height() - (2 * cornerDestHeight)) );
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 

    // down arrow
    srcRect = IntRect( 45, 5, 5, 12 );
    destRect = IntRect( r.x() + (r.width() - 13), r.y() + ((r.height() / 2) - ( 12/2 )), 5, 12 );
    srcRect.move( stateOffset );
    paintInfo.context->drawImage( image.get(), DeviceColorSpace, destRect, srcRect ); 


    paintInfo.context->restore();
    return true; 
}
void RenderThemeApollo::adjustMenuListStyle(CSSStyleSelector*, RenderStyle*, Element*) const 
{
}

static const int MIN_MENULIST_SIZE = 14;
int RenderThemeApollo::minimumMenuListSize(RenderStyle*) const
{
    // The mac version of this function checks the font size and adjust
    // the min width. Do we need to do this.
    //
    return MIN_MENULIST_SIZE;
}

bool RenderThemeApollo::paintMenuList(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    ASSERT(o);
    ASSERT(o->style());

    Color fillColor( 0xff, 0xff, 0xee );
    fillColor = o->style()->visitedDependentColor(CSSPropertyBackgroundColor);
    Color rectColor( 0x18, 0x52, 0x84 );
    if ( o->node() && !isEnabled(o))
    {
        rectColor = Color( 0xce, 0xce, 0xdb );
    }
    i.context->save();
    i.context->setStrokeThickness(1);
    i.context->setStrokeColor(rectColor.rgb(), DeviceColorSpace);
    i.context->setFillColor(fillColor, DeviceColorSpace);
    i.context->drawRect(r);
    i.context->restore();

    return false;
}

// At this moment, we return false for CheckBox and Radio controls. Returning false implies that the platform is in charge with control styling.
// Returning true means that the control will be styled by WebKit and will not have a custom appearance.    
// Styling the control means to implement the calls to adjust<control>Style. For instance, CheckBox and Radio controls have in RenderTheme.cpp
// calls to adjustCheckboxStyle and adjustRadioStyle. RenderThemeApollo implements setCheckBoxSize and setRadioSize that are called by adjustCheckboxStyle and adjustRadioStyle.
// For input controls of type Search, all the major current port are implementing the styling by means of adjust<Search..>Style methods. In these methods, the box shadow is
// disabled. In the future, we may want to implement styling of these controls. At that moment, we can change the implementation of isControlStyled to return false only for
// the controls we know we style and call base class RenderTheme::isControlStyled for the rest of controls.    
bool RenderThemeApollo::isControlStyled(    const WebCore::RenderStyle *style,
                                            const WebCore::BorderData &border,
                                            const FillLayer& background,
                                            const WebCore::Color &backgroundColor) const
{
    switch (style->appearance()) {
        case PushButtonPart:
        case SquareButtonPart:
        case ButtonPart:
            // Test the style to see if the UA border and background match.
            return (style->border() != border ||
                    *style->backgroundLayers() != background ||
                    style->visitedDependentColor(CSSPropertyBackgroundColor) != backgroundColor);
        case CheckboxPart:
        case RadioPart:
            return false;
        case MenulistPart:
        case MenulistButtonPart:
        default: 
            return true;
    }
    
     return true;
}

bool RenderThemeApollo::supportsFocusRing(const RenderStyle* style) const
{
    return (style->hasAppearance() && style->appearance() != TextFieldPart && style->appearance() != TextAreaPart && style->appearance() != ListboxPart);
}
 
    
// Disable shadow box on search input controls. AIR renders only Checkbox and Radio controls.
// Search input controls are not rendered by AIR. Because of that, they are not considered native controls, like Checkbox and Radio.
// Search input control must have their shadow disabled just like in Safari (which renders these input controls like we render Checkbox/Radio).
void RenderThemeApollo::disableBoxShadowForControl(ControlPart controlAppearance, RenderStyle* controlStyle)
{
    bool nativeControl = controlStyle->hasAppearance();
    
    bool isSearchControl = ((SearchFieldPart == controlAppearance) || 
                            (SearchFieldCancelButtonPart == controlAppearance) || 
                            (SearchFieldDecorationPart == controlAppearance) ||
                            (SearchFieldResultsDecorationPart == controlAppearance) ||
                            (SearchFieldResultsButtonPart == controlAppearance));
    if (nativeControl || isSearchControl)
        controlStyle->setBoxShadow(0);
}
    
}
