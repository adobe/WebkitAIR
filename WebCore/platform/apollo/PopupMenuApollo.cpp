/*
 * This file is part of the popup menu implementation for <select> elements in WebCore.
 *
 * Copyright 2007 Adobe Systems Incorporated. All Rights Reserved.
 * Copyright (C) 2006 Apple Computer, Inc.
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

#include <config.h>
#include <PopupMenu.h>
#include <Document.h>
#include <HTMLSelectElement.h>
#include <HTMLOptionElement.h>
#include <HTMLOptGroupElement.h>
#include <HTMLNames.h>
#include <FrameView.h>
#include <FrameLoader.h>
#include <FrameLoaderClientApollo.h>
#include <WebKitApollo/WebHost.h>
#include <WebKitApollo/WebWindow.h>
#include <WebKitApollo/WebPopupWindow.h>
#include <WebKitApollo/WebPoint.h>

namespace WebCore {

PopupMenu::PopupMenu(PopupMenuClient* client)
: m_popupClient(client)
, m_webHost(0)
, m_webPopupWindow(0)
{
    ASSERT(m_popupClient);
}

void PopupMenu::hide()
{
	if (m_webPopupWindow) {
		m_webPopupWindow->m_pVTable->hide( m_webPopupWindow );
        m_webPopupWindow = 0;
	}
}

void PopupMenu::disconnectClient() 
{ 
    hide();
    m_popupClient = 0; 
}

void getOptionText( HTMLElement* element, RenderStyle* selectStyle, String& text, float& naturalTextWidth );
void getOptionText( HTMLElement* element, RenderStyle* selectStyle, String& text, float& naturalTextWidth )
{
	String itemText;

	if ( element->hasTagName(HTMLNames::optgroupTag)) {
		HTMLOptGroupElement* optgroupElement = static_cast<HTMLOptGroupElement*>(element);
		itemText = optgroupElement->getAttribute(HTMLNames::labelAttr);
	} else if ( element->hasTagName(HTMLNames::optionTag) ) {
        HTMLOptionElement* optionElement = static_cast<HTMLOptionElement*>(element);
        itemText = optionElement->text();
        if (itemText.isEmpty())
            itemText = optionElement->getAttribute(HTMLNames::labelAttr);                        
	}
	else {
		ASSERT(false);
	}
    
    itemText = element->document()->displayStringModifiedByEncoding(itemText);

	// In WinIE, an optgroup can't start or end with whitespace (other than the indent
	// we give it).  We match this behavior.
	itemText = itemText.stripWhiteSpace();
	// We want to collapse our whitespace too.  This will match other browsers.
	itemText = itemText.simplifyWhiteSpace();

	text = String(itemText);
	naturalTextWidth = 0.0f;
	if (!text.isEmpty()) {
		naturalTextWidth = selectStyle->font().floatWidth(TextRun(text.impl()));
	}
}

void PopupMenu::populate(WebPopupWindow* webPopupWindow)
{
	ASSERT(webPopupWindow);
	ASSERT(client());
	
    HTMLSelectElement* select = 0;
    HTMLElement* popupMenuElement = client()->popupMenuElement();
    if ( popupMenuElement->hasTagName(HTMLNames::selectTag) )
        select = static_cast<HTMLSelectElement*>(popupMenuElement);
    if (!select)
        return;
    const Vector<Element*>& items = select->listItems();
    size_t size = items.size();
    for (size_t i = 0; i < size; ++i) {
        Element* element = items[i];
        if (element->hasTagName(HTMLNames::optionTag)) {
			String itemText;
			float naturalTextWidth = 0.0f;
            HTMLElement* htmlElement = (HTMLElement*)(element);
			getOptionText( htmlElement, popupMenuElement->renderer()->style(), itemText, naturalTextWidth );
			if (!itemText.isEmpty()) {
				ASSERT(sizeof(uint16_t) == sizeof(UChar));
				const uint16_t* const pUTF16OptionText = reinterpret_cast<const uint16_t*>(itemText.characters());
				unsigned long const numItemTextCodeUnits = itemText.length();
				
				bool isSelected = client()->itemIsSelected(i);
				bool isDisabled = !element->isEnabledFormControl();
				bool isChildOfGroup = (element->parentNode() && element->parentNode()->hasTagName(HTMLNames::optgroupTag));
                if (isChildOfGroup)
				{
                    Element* parentElement = static_cast<Element*>(element->parentNode());
                    ASSERT(parentElement);
                    if (!parentElement->isEnabledFormControl())
					    isDisabled = true;
				}

				webPopupWindow->m_pVTable->addOption(webPopupWindow, pUTF16OptionText, numItemTextCodeUnits, i, isSelected, isDisabled, isChildOfGroup, naturalTextWidth);
			}
		}
		else if (element->hasTagName(HTMLNames::optgroupTag)) {
			String itemText;
			float naturalTextWidth = 0.0f;
            HTMLElement* htmlElement = (HTMLElement*)(element);
			getOptionText( htmlElement, popupMenuElement->renderer()->style(), itemText, naturalTextWidth );
			if (!itemText.isEmpty()) 
			{
				ASSERT(sizeof(uint16_t) == sizeof(UChar));
				const uint16_t* const pUTF16OptionText = reinterpret_cast<const uint16_t*>(itemText.characters());
				unsigned long const numItemTextCodeUnits = itemText.length();
				
				bool isDisabled = !element->isEnabledFormControl();

				webPopupWindow->m_pVTable->addGroupLabel(webPopupWindow, pUTF16OptionText, numItemTextCodeUnits, i, isDisabled, naturalTextWidth);
			}
		}
		else if (element->hasTagName(HTMLNames::hrTag))
			webPopupWindow->m_pVTable->addSeparator(webPopupWindow, i);
		else
            ASSERT(0);
    }
}

WebHost* PopupMenu::getWebHost(FrameView* frameView)
{
	if (!m_webHost) {
		ASSERT(frameView);
		Frame* frame = frameView->frame();
		ASSERT(frame);
		FrameLoaderClientApollo* loaderClient = FrameLoaderClientApollo::clientApollo(frame);
		ASSERT(loaderClient);
		m_webHost = loaderClient->webHost();
	}
	ASSERT(m_webHost);
	return m_webHost;
}

void PopupMenu::show(IntRect const& rect, FrameView* frameView, int /*index*/)
{
    ASSERT(m_popupClient);

	FrameLoaderClientApollo* const apolloClient(FrameLoaderClientApollo::clientApollo(frameView));
    HTMLElement* const popupMenuElement = client()->popupMenuElement();
    float const computedFontSize = popupMenuElement->renderer()->style()->font().size();
    
    m_webPopupWindow = apolloClient->createWebPopupMenu(client(), computedFontSize);
	if (m_webPopupWindow) {
		populate(m_webPopupWindow);

		int posX = 0;
		int posY = 0;
		getPopupPositionInClient(&posX, &posY);
		IntPoint pt( posX, posY );

        pt = frameView->contentsToWindow(rect.location());
        pt.move(0, rect.height());

        WebWindow * pWebWindow = apolloClient->webWindow();
        WebIntPoint point = { pt.x(), pt.y() };
        WebIntPoint resultPoint;
        pWebWindow->m_pVTable->viewportToWindow(pWebWindow, &point, &resultPoint);
        
        m_webPopupWindow->m_pVTable->show(m_webPopupWindow, resultPoint.m_x, resultPoint.m_y, rect.width());

	}
}

void PopupMenu::getPopupPositionInClient(int* clientX, int* clientY)
{
	ASSERT( clientX && clientY );

	(*clientX) = 0;
	(*clientY) = 0;
    if(m_webPopupWindow) {
        HTMLElement* elem = m_popupClient->popupMenuElement();
        ASSERT(elem);
        RenderObject* ro = elem->renderer();        
        ASSERT(ro);
        RenderBox* rb = toRenderBox(ro);
        ASSERT(rb);
      
        int realXPos = rb->x();
        int realYPos = rb->y();

		realYPos += rb->height();
        RenderBox* item = NULL;
        if (rb->parent()->isRenderBlock())
            item = toRenderBox(rb->parent());
        while (item) {
            if (!item->isTableRow()) {
				realXPos -= item->scrollLeft();
                realXPos += item->x();
				realYPos -= item->scrollTop();
                realYPos += item->y();
            }
            if (item->parent() && item->parent()->isRenderBlock())
                item = toRenderBox(item->parent());
            else
                item = NULL;
        }

		(*clientX) = realXPos;
		(*clientY) = realYPos;
    }    
}

void PopupMenu::updateFromElement()
{
	ASSERT(client());
	int selectedIndex = client()->selectedIndex();
	client()->setTextFromItem(selectedIndex);
	if (m_webPopupWindow)
		m_webPopupWindow->m_pVTable->updateSelectedItem(m_webPopupWindow, selectedIndex);
}

PopupMenu::~PopupMenu()
{
    if(m_webPopupWindow)
        m_webPopupWindow = 0;
}

bool PopupMenu::itemWritingDirectionIsNatural()
{
    return true;
}

}

