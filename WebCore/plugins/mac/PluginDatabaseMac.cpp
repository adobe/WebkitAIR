/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
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

#include "config.h"
#include "PluginDatabase.h"
#include <WebKitApollo/WebKit.h>
#include "KURL.h"
#include "PluginView.h"

#include "TextEncoding.h"

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions; }

namespace WebCore {

    void PluginDatabase::getPluginPathsInDirectories(HashSet<String>& paths) const
    {
        unsigned long flashPlayerPluginPathUTF8ByteLen = 0;
        unsigned char* const flashPlayerPluginPathUTF8Bytes = WebKitApollo::g_HostFunctions->getFlashPlayerPluginPathUTF8Bytes(&flashPlayerPluginPathUTF8ByteLen);
        ASSERT(flashPlayerPluginPathUTF8Bytes);
        ASSERT(flashPlayerPluginPathUTF8ByteLen > 0);
        
        String flashPlayerPluginPath(UTF8Encoding().decode(reinterpret_cast<const char*>(flashPlayerPluginPathUTF8Bytes), flashPlayerPluginPathUTF8ByteLen));
        WebKitApollo::g_HostFunctions->freeBytes(flashPlayerPluginPathUTF8Bytes);
    
        time_t lastModified;
        if (!getFileModificationTime(flashPlayerPluginPath, lastModified))
            return;
        paths.add(flashPlayerPluginPath);
    }
    
    static inline void addMacromediaPluginDirectories(Vector<String>& directories)
    {
        unsigned long flashPlayerPluginPathUTF8ByteLen = 0;
        unsigned char* const flashPlayerPluginPathUTF8Bytes = WebKitApollo::g_HostFunctions->getFlashPlayerPluginPathUTF8Bytes(&flashPlayerPluginPathUTF8ByteLen);
        ASSERT(flashPlayerPluginPathUTF8Bytes);
        ASSERT(flashPlayerPluginPathUTF8ByteLen > 0);
        
        String path = UTF8Encoding().decode(reinterpret_cast<const char*>(flashPlayerPluginPathUTF8Bytes), flashPlayerPluginPathUTF8ByteLen);
        directories.append(path);
        WebKitApollo::g_HostFunctions->freeBytes(flashPlayerPluginPathUTF8Bytes);
    }
    
    Vector<String> PluginDatabase::defaultPluginDirectories()
    {
        Vector<String> directories;
        
        addMacromediaPluginDirectories(directories);
        return directories;
    }
    
    bool PluginDatabase::isPreferredPluginDirectory(const String& /*directory*/)
    {
        return false;
    }
    
}
