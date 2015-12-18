/*
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

#ifndef PluginData_h
#define PluginData_h

#include <wtf/RefCounted.h>
#include <wtf/Vector.h>
#include "PlatformString.h"
#if PLATFORM(APOLLO)
    #include <wtf/HashMap.h>
#endif

namespace WebCore {

    class Page;
	struct PluginInfo;
#if PLATFORM(APOLLO)
        class MimeType;
        class Plugin;
#endif

    struct MimeClassInfo : Noncopyable {
        String type;
        String desc;
        String suffixes;
        PluginInfo* plugin;
    };

    struct PluginInfo : Noncopyable {
        String name;
        String file;
        String desc;
        Vector<MimeClassInfo*> mimes;
    };

    // FIXME: merge with PluginDatabase in the future
    class PluginData : public RefCounted<PluginData> {
    public:
        static PassRefPtr<PluginData> create(const Page* page) { return adoptRef(new PluginData(page)); }
        ~PluginData();
#if PLATFORM(APOLLO)
        void disconnectPage();
#else
		void disconnectPage() { m_page = 0; }
#endif
        const Page* page() const { return m_page; }

        const Vector<PluginInfo*>& plugins() const { return m_plugins; }
        const Vector<MimeClassInfo*>& mimes() const { return m_mimes; }
        
#if PLATFORM(APOLLO)	
        PassRefPtr<MimeType> getMimeType(unsigned const index);
        PassRefPtr<Plugin> getPlugin(unsigned const index);
#endif

        bool supportsMimeType(const String& mimeType) const;
        String pluginNameForMimeType(const String& mimeType) const;

        static void refresh();

    private:
        PluginData(const Page*);
        void initPlugins();

        Vector<PluginInfo*> m_plugins;
        Vector<MimeClassInfo*> m_mimes;
#if PLATFORM(APOLLO)
        WTF::HashMap<unsigned, RefPtr<MimeType> > m_cachedMimeTypes;
        WTF::HashMap<unsigned, RefPtr<Plugin> > m_cachedPlugins;
#endif
        const Page* m_page;
    };

}

#endif
