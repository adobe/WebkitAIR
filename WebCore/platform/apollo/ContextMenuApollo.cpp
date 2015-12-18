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
#include <ContextMenu.h>
#include <ContextMenuItemApollo.h>


WebMenu::WebMenu() :
	nCapacity(10), 
	items(new WebMenuItem*[nCapacity]), 
	nItems(0)
{
}

WebMenu::WebMenu(const WebMenu* rhs) : 
	nCapacity(rhs->nCapacity),
	items(new WebMenuItem*[nCapacity]),
	nItems(rhs->nItems)
{
    for(int i = 0; i < nItems; ++i)
    {
        items[i] = rhs->items[i];
        items[i]->incRefCount();
    }
}

WebMenu::~WebMenu()
{
    if(!items)
        return;

    for(int i = 0; i < nItems; ++i)
        items[i]->decRefCount();

    delete [] items;
}

void WebMenu::append(WebMenuItem* item, bool bTakeOwnership)
{
    ASSERT( item );
    if(!item)
        return;

    if(nItems == nCapacity)
    {
        unsigned int nNewCapacity = nCapacity << 1;
        WebMenuItem** newItems = new WebMenuItem*[nNewCapacity];
        memcpy(newItems, items, sizeof(WebMenuItem*) * nCapacity);
        delete [] items;

        nCapacity = nNewCapacity;
        items = newItems;
    }

    items[nItems++] = item;

    if(!bTakeOwnership)
        item->incRefCount();
}

namespace WebCore {

ContextMenu::ContextMenu(HitTestResult const& hitTestResult)
    :   m_hitTestResult(hitTestResult),
        m_platformDescription( NULL )
{
}

void ContextMenu::appendItem(ContextMenuItem& item)
{
    switch(item.action())
    {
        case WebCore::ContextMenuItemTagCopyImageToClipboard:
        case WebCore::ContextMenuItemTagCopy:
        case WebCore::ContextMenuItemTagCut:
        case WebCore::ContextMenuItemTagPaste:
        case WebCore::ContextMenuItemTagFontMenu:
        case WebCore::ContextMenuItemTagShowFonts:
        case WebCore::ContextMenuItemTagBold:
        case WebCore::ContextMenuItemTagItalic:
        case WebCore::ContextMenuItemTagUnderline:
        case WebCore::ContextMenuItemTagOutline:
        case WebCore::ContextMenuItemTagStyles:
        case WebCore::ContextMenuItemTagShowColors:
        case WebCore::ContextMenuItemTagWritingDirectionMenu:
        case WebCore::ContextMenuItemTagDefaultDirection:
        case WebCore::ContextMenuItemTagLeftToRight:
        case WebCore::ContextMenuItemTagRightToLeft:
            break;
        default: return; // not supported
    }

    if(!m_platformDescription)
        m_platformDescription = new WebMenu;

    checkOrEnableIfNeeded(item);

    PlatformMenuItemDescription platformItem = item.releasePlatformDescription();

    m_platformDescription->append( platformItem, true );
}

void ContextMenu::setPlatformDescription(PlatformMenuDescription menu)
{
    if(menu == m_platformDescription)
        return;

    delete m_platformDescription;
    m_platformDescription = menu;
}

ContextMenu::~ContextMenu()
{
    delete m_platformDescription;
    m_platformDescription = 0;
}

PlatformMenuDescription ContextMenu::platformDescription() const
{
    return m_platformDescription;
}

}
