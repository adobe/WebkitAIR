/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#import "config.h"
#import "PluginData.h"

#if PLATFORM(APOLLO)
#include "PluginDatabase.h"
#include "PluginPackage.h"
#include "PluginInfoStore.h"
#else
#import "Logging.h"
#import "mac/WebCoreViewFactory.h"
#import "mac/BlockExceptions.h"
#endif

namespace WebCore {

void PluginData::initPlugins()
{
#if PLATFORM(APOLLO)
    PluginDatabase *db = PluginDatabase::installedPlugins();
    const Vector<PluginPackage*> &plugins = db->plugins();
    
    for (unsigned int i = 0; i < plugins.size(); ++i) {
        PluginInfo* info = new PluginInfo;
        PluginPackage* package = plugins[i];
        
        info->name = package->name();
        info->file = package->fileName();
        info->desc = package->description();
        
        const MIMEToDescriptionsMap& mimeToDescriptions = package->mimeToDescriptions();
        MIMEToDescriptionsMap::const_iterator end = mimeToDescriptions.end();
        for (MIMEToDescriptionsMap::const_iterator it = mimeToDescriptions.begin(); it != end; ++it) {
            MimeClassInfo* mime = new MimeClassInfo;
            info->mimes.append(mime);
            
            mime->type = it->first;
            mime->desc = it->second;
            mime->plugin = info;
            
            Vector<String> extensions = package->mimeToExtensions().get(mime->type);
            
            for (unsigned i = 0; i < extensions.size(); i++) {
                if (i > 0)
                    mime->suffixes += ",";
                
                mime->suffixes += extensions[i];
            }
        }
        
        m_plugins.append(info);
    }    
#else
    BEGIN_BLOCK_OBJC_EXCEPTIONS;

    NSArray* plugins = [[WebCoreViewFactory sharedFactory] pluginsInfo];
    for (unsigned int i = 0; i < [plugins count]; ++i) {
        PluginInfo* pluginInfo = new PluginInfo;

        id <WebCorePluginInfo> plugin = [plugins objectAtIndex:i];
    
        pluginInfo->name = [plugin name];
        pluginInfo->file = [plugin filename];
        pluginInfo->desc = [plugin pluginDescription];

        NSEnumerator* MIMETypeEnumerator = [plugin MIMETypeEnumerator];
        while (NSString* MIME = [MIMETypeEnumerator nextObject]) {
            MimeClassInfo* mime = new MimeClassInfo;
            pluginInfo->mimes.append(mime);
            mime->type = String(MIME).lower();
            mime->suffixes = [[plugin extensionsForMIMEType:MIME] componentsJoinedByString:@","];
            mime->desc = [plugin descriptionForMIMEType:MIME];
            mime->plugin = pluginInfo;
        }

        m_plugins.append(pluginInfo);
    }
    END_BLOCK_OBJC_EXCEPTIONS;
#endif

    return;
}

void PluginData::refresh()
{
#if PLATFORM(APOLLO)
    PluginDatabase *db = PluginDatabase::installedPlugins();
    db->refresh();
#else
    BEGIN_BLOCK_OBJC_EXCEPTIONS;
    [[WebCoreViewFactory sharedFactory] refreshPlugins];
    END_BLOCK_OBJC_EXCEPTIONS;
#endif
}

}

