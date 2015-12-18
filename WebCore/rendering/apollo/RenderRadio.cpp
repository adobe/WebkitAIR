/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright 2007 Adobe Systems Incorporated. All Rights Reserved.
 * Copyright (C) 2005 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"
#include "RenderRadio.h"
#include <HTMLInputElement.h>
#include "GraphicsContext.h"
#include <math.h>

namespace WebCore {

RenderRadio::RenderRadio(Node* node)
    : RenderBlock(node)
{
}

void RenderRadio::paintObject(PaintInfo& i, int _tx, int _ty)
{
    ASSERT(node());
    Element* element = static_cast<Element*>(node());
    InputElement* inputElement = toInputElement(element);
    bool isEnabled = element->isEnabledFormControl();
    bool isChecked = inputElement->isChecked();
    
    i.context->save();

	RefPtr<Image> image = Image::loadPlatformResource( "radioButtonStates" ).get();
	IntPoint destPt( _tx, _ty );
	IntRect srcRect( 0, 0, 14, 16 );

    if (isEnabled)
	{
		if (node()->active())
		{
			if (isChecked)
			{
				srcRect.move( 75, 0 );
			}
			else
			{
				srcRect.move( 60, 0 );
			}
		}
		else if (node()->hovered())
		{
			if (isChecked)
			{
				srcRect.move( 45, 0 );
			}
			else
			{
				srcRect.move( 30, 0 );
			}
		}
		else
		{
			if (isChecked)
			{
				srcRect.move( 15, 0 );
			}
			else
			{
			}
		}
	}
	else 
	{
	    if (isChecked)
		{
			srcRect.move( 105, 0 );
		}
		else
		{
			srcRect.move( 90, 0 );
		}
	}

	i.context->drawImage( image.get(), DeviceColorSpace, destPt, srcRect ); 

    // draw the focus ring.
    //
    if (node()->focused())
    {
        IntRect focusRingSrcRect( 120, 0, 15, 17 );
        i.context->drawImage( image.get(), DeviceColorSpace, destPt, focusRingSrcRect ); 
    }

#if 0 
	// this is the old "moveto - lineto" drawing code

	IntRect checkRect(_tx + borderLeft()
                     , _ty + borderTop()
                     , width() - borderLeft() - borderRight()
                     , height() - borderBottom() - borderTop());
    Color fillColor(0xff, 0xff, 0xff, 0x00);
    Color rectColor(0x18, 0x52, 0x84);
    Color checkColor(0x21, 0xa5, 0x21);
    if (!node()->isEnabled()) {
        rectColor = Color(0xce, 0xce, 0xdb);
        checkColor = Color(0xce, 0xce, 0xdb);
    }
    i.context->setStrokeThickness(1);
    i.context->setStrokeStyle(SolidStroke);
    i.context->setStrokeColor(rectColor);
    i.context->setFillColor(fillColor);
    i.context->drawEllipse(checkRect);

    if(node()->isEnabled() && node()->hovered()) {
        i.context->setStrokeColor(Color(0xff, 0xb5, 0x31));
        
        IntRect hoverRect = checkRect;
        hoverRect.inflateX(-1);
        hoverRect.inflateY(-1);
        i.context->drawEllipse( hoverRect );
        hoverRect.inflateX(-1);
        hoverRect.inflateY(-1);
        i.context->drawEllipse( hoverRect );

    }

    if (isChecked) {
        i.context->setStrokeThickness(2);
        i.context->setStrokeColor(checkColor);
        i.context->setFillColor(checkColor);

        IntRect fillRect = checkRect;
        fillRect.inflateX(-2);
        fillRect.inflateY(-2);
        i.context->drawEllipse(fillRect);

    }
#endif

    i.context->restore();
}

bool RenderRadio::hasControlClip() const 
{
    return true;
}

}
