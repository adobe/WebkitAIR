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

#ifndef ContextMenuItemApollo_H
#define ContextMenuItemApollo_H

static inline unsigned int shortStringCharLen(const short* str)
{
    if(!str)
        return 0;

    const short *end = str;
    while(*end) end++;

    return static_cast<int>(end-str);
}

class WebMenuItem;

class WebMenu 
{
    int nCapacity;
    WebMenuItem** items;
    int nItems;

public:
    WebMenu();

    // incs refcount on items
    WebMenu(const WebMenu* rhs);
    ~WebMenu();

    WebMenuItem** getItems() { return items; }
    int getNumItems() { return nItems; }
    void append(WebMenuItem* item, bool bTakeOwnership);
private:
    WebMenu(const WebMenu&);
    WebMenu& operator=(const WebMenu&);
};

class WebMenuItem
{
    int m_type;
    int m_action;
    short* m_title;
    unsigned int m_nTitleCharLen;
    bool m_bEnabled;
    bool m_bChecked;

    WebMenu* m_subMenu;

    unsigned int m_nRefCount;
public:
    WebMenuItem();
    WebMenuItem(int type, int action, const short* title, WebMenu* subMenu);

    int getType() { return m_type; }
    int getAction() {  return m_action; }
    short* getTitle() { return m_title; }
    unsigned int getTitleCharLen() { return m_nTitleCharLen; }
    bool getChecked() { return m_bChecked; }
    bool getEnabled() { return m_bEnabled; }
    void setChecked(bool val) { m_bChecked = val; }
    void setEnabled(bool val) { m_bEnabled = val; }

    ~WebMenuItem() 
    {
        if(m_title)
        {
            delete [] m_title;
            m_title = 0;
        }

        releaseSubMenu(); 
    }

    void incRefCount() { m_nRefCount++; }
    void decRefCount()
    {

        if(!--m_nRefCount)
            delete this;
    }

    WebMenuItem** getSubMenuItems() { return m_subMenu ? m_subMenu->getItems() : 0; }
    int getNumSubMenuItems() { return m_subMenu ? m_subMenu->getNumItems() : 0; }

    void setTitle(const short* title)
    {
        m_nTitleCharLen = shortStringCharLen(title);
        if(m_nTitleCharLen)
        {
            m_title = new short[m_nTitleCharLen];
            memcpy(m_title, title, m_nTitleCharLen*sizeof(short));
        }
    }

    void setSubMenu(WebMenu* subMenu)
    {
        releaseSubMenu();

        if(!subMenu)
            return;

        m_subMenu = new WebMenu(subMenu);
    }

    void releaseSubMenu()
    {
        if(!m_subMenu)
            return;

        delete m_subMenu;
        m_subMenu = 0;
    }
private:
    WebMenuItem(const WebMenuItem& rhs);
    WebMenuItem& operator=(const WebMenuItem& rhs);

};

#endif /* ContextMenuItemApollo_H */
