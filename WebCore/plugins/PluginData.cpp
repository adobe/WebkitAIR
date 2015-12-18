/*
    Copyright (C) 2000 Harri Porten (porten@kde.org)
    Copyright (C) 2000 Daniel Molkentin (molkentin@kde.org)
    Copyright (C) 2000 Stefan Schimanski (schimmi@kde.org)
    Copyright (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All Rights Reserved.
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"


#include "PluginData.h"
#include "PluginInfoStore.h"
#include "MimeType.h"
#include "Plugin.h"


namespace WebCore {

PluginData::PluginData(const Page* page)
    : m_page(page)
{
    initPlugins();

    for (unsigned i = 0; i < m_plugins.size(); ++i) {
        const PluginInfo* plugin = m_plugins[i];
        for (unsigned j = 0; j < plugin->mimes.size(); ++j)
            m_mimes.append(plugin->mimes[j]);
    }
}

PluginData::~PluginData()
{
    deleteAllValues(m_plugins);
    deleteAllValues(m_mimes);
}

bool PluginData::supportsMimeType(const String& mimeType) const
{
    for (unsigned i = 0; i < m_mimes.size(); ++i) {
        if (m_mimes[i]->type == mimeType)
            return true;
    }

#if PLATFORM(APOLLO)
    // we can remove this when we move the PDF plug-in code to use PluginView
    if(PluginInfoStore::isPDFMIMEType(mimeType))
        return true;
#endif

    return false;
}

String PluginData::pluginNameForMimeType(const String& mimeType) const
{
    for (unsigned i = 0; i < m_mimes.size(); ++i)
        if (m_mimes[i]->type == mimeType)
            return m_mimes[i]->plugin->name;
    return String();
}

PassRefPtr<MimeType> PluginData::getMimeType(unsigned const index)
{
    unsigned const cacheKey = index + 1;
    WTF::HashMap<unsigned, RefPtr<MimeType> >::iterator const existingEntry =
        m_cachedMimeTypes.find(cacheKey);
    if (existingEntry != m_cachedMimeTypes.end()) {
        return existingEntry->second;
    }
    else {
        RefPtr<MimeType> const newMimeType(MimeType::create(this, index));
        m_cachedMimeTypes.add(cacheKey, newMimeType);
        return newMimeType;
    }
}

void PluginData::disconnectPage()
{
    m_page = 0;
    m_cachedMimeTypes.clear();
    m_cachedPlugins.clear();
}


PassRefPtr<Plugin> PluginData::getPlugin(unsigned const index)
{
    unsigned const cacheKey = index + 1;
    WTF::HashMap<unsigned, RefPtr<Plugin> >::iterator const existingEntry =
        m_cachedPlugins.find(cacheKey);
    if (existingEntry != m_cachedPlugins.end()) {
        return existingEntry->second;
    }
    else {
        RefPtr<Plugin> const newPlugin(Plugin::create(this, index));
        m_cachedPlugins.add(cacheKey, newPlugin);
        return newPlugin;
    }
}

}
