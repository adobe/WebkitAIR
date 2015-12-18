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
#include <Frame.h>
#include <FrameView.h>
#include "ContextMenuItemApollo.h"
#include "FrameLoaderClientApollo.h"

WebMenuItem::WebMenuItem()
    : m_type((int)WebCore::ActionType), 
	m_action((int)WebCore::ContextMenuItemTagNoAction), 
	m_title(0), 
	m_nTitleCharLen(0),
	m_bEnabled(false), 
	m_bChecked(false), 
	m_subMenu(0), 
	m_nRefCount(1)
{
}

WebMenuItem::WebMenuItem(int type, int action, const short* title, WebMenu* subMenu)
    : m_type(type), 
	m_action(action), 
	m_title(0), 
	m_nTitleCharLen(0),
	m_bEnabled(true), 
	m_bChecked(false), 
	m_subMenu(0),
	m_nRefCount(1)
{
    setTitle(title);
    setSubMenu(subMenu);
}

namespace WebCore {

PlatformMenuItemDescription ContextMenuItem::releasePlatformDescription()
{
    WebMenuItem* ret = m_platformDescription;
    m_platformDescription = 0;

    return ret;
}

ContextMenuItem::ContextMenuItem(ContextMenuItemType type, ContextMenuAction action, const String& title, ContextMenu* subMenu)
: m_platformDescription( new WebMenuItem(type, action, (const short*)const_cast<String&>(title).charactersWithNullTermination(), subMenu ? subMenu->platformDescription() : 0) )
{
}

void ContextMenuItem::setSubMenu(ContextMenu* menu)
{
    WebMenu* subMenu = menu->platformDescription();
    m_platformDescription->setSubMenu(subMenu);
}

ContextMenuItem::~ContextMenuItem()
{
    if( m_platformDescription )
    {
        delete m_platformDescription;
        m_platformDescription = 0;
    }
}

ContextMenuItemType ContextMenuItem::type() const
{
    return (ContextMenuItemType)m_platformDescription->getType();
}

ContextMenuAction ContextMenuItem::action() const
{
	return (ContextMenuAction)m_platformDescription->getAction();
}

String ContextMenuItem::title() const
{
	return String((const UChar*)m_platformDescription->getTitle());
}

void ContextMenuItem::setChecked(bool val)
{
	m_platformDescription->setChecked(val);
}

void ContextMenuItem::setEnabled(bool val)
{
	m_platformDescription->setEnabled(val);
}

bool ContextMenuItem::enabled() const
{
	return m_platformDescription->getEnabled();
}

bool ContextMenuItem::selected() const
{
	return m_platformDescription->getChecked();
}

void ContextMenuItem::setTitle(const String &)
{
}


}

