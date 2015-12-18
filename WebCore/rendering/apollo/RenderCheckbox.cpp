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
#include "RenderCheckbox.h"

#include <Document.h>
#include <GraphicsContext.h>
#include <HTMLInputElement.h>
#include <RenderText.h>
#include <HTMLNames.h>
#include <Image.h>

namespace WebCore {

using namespace HTMLNames;

RenderCheckbox::RenderCheckbox  (Node* node)
    : RenderBlock(node)
{
}

void RenderCheckbox::paintObject(PaintInfo& i, int _tx, int _ty)
{
    ASSERT(node());
    Element* element = static_cast<Element*>(node());
    InputElement* inputElement = toInputElement(element);
    bool isEnabled = element->isEnabledFormControl();
    bool isChecked = inputElement->isChecked();
    i.context->save();

	RefPtr<Image> image = Image::loadPlatformResource( "checkBoxStates" );
	IntPoint destPt( _tx, _ty );
	IntRect srcRect( 0, 0, 15, 15 );

    if (isEnabled)
	{
		if (node()->active())
		{
			if (isChecked)
			{
				srcRect.move( 80, 0 );
			}
			else
			{
				srcRect.move( 64, 0 );
			}
		}
		else if (node()->hovered())
		{
			if (isChecked)
			{
				srcRect.move( 48, 0 );
			}
			else
			{
				srcRect.move( 32, 0 );
			}
		}
		else
		{
			if (isChecked)
			{
				srcRect.move( 16, 0 );
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
			srcRect.move( 112, 0 );
		}
		else
		{
			srcRect.move( 96, 0 );
		}
	}

	i.context->drawImage( image.get(), DeviceColorSpace, destPt, srcRect ); 

    // draw the focus ring.
    //
    if (node()->focused())
    {
        IntRect focusRingSrcRect( 128, 0, 16, 16 );
        i.context->drawImage( image.get(), DeviceColorSpace, destPt, focusRingSrcRect ); 
    }


#if 0 
	// this is the old "moveto - lineto" drawing code

	IntRect checkRect(_tx + borderLeft()
                     , _ty + borderTop()
                     , width() - borderLeft() - borderRight()
                     , height() - borderBottom() - borderTop());
    Color fillColor(0xff, 0xff, 0xff);
    Color rectColor(0x18, 0x52, 0x84);
    Color checkColor(0x21, 0xa5, 0x21);
    if (!node()->isEnabled()) {
        rectColor = Color(0xce, 0xce, 0xdb);
        checkColor = Color(0xce, 0xce, 0xdb);
    }
    i.context->setStrokeThickness(1);
    i.context->setStrokeStyle(SolidStroke);
    i.context->setStrokeColor(rectColor.rgb());
    i.context->setFillColor(fillColor);
    i.context->drawRect(checkRect);

    if(node()->isEnabled() && node()->hovered()) {
        Color hoverColor = Color(0xff, 0xb5, 0x31);
        i.context->setStrokeColor(hoverColor.rgb());

        IntRect hoverRect = checkRect;
        hoverRect.inflateX(-1);
        hoverRect.inflateY(-1);
        i.context->drawRect(hoverRect);
        hoverRect.inflateX(-1);
        hoverRect.inflateY(-1);
        i.context->drawRect(hoverRect);
    }

    if (node()->isChecked()) {
        i.context->setStrokeThickness(2);
        i.context->setStrokeColor(checkColor.rgb());

        IntPoint p1( _tx + borderLeft() + (width() * 3.0/12.0), _ty + borderTop() + (height() * 7.0/12.0) );
        IntPoint p2( _tx + borderLeft() + (width() * 6.0/12.0), _ty + borderTop() + (height() * 10.0/12.0) );
        IntPoint p3( _tx + borderLeft() + (width() * 11.0/12.0), _ty + borderTop() + (height() * 5.0/12.0) );
        i.context->drawLine( p1, p2 );
        i.context->drawLine( p2, p3 );
    }
#endif 
    i.context->restore();
}

bool RenderCheckbox::hasControlClip() const 
{
    return true;
}

}
