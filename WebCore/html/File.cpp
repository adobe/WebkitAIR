/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "File.h"

#include "FileSystem.h"
#include "MIMETypeRegistry.h"

#if PLATFORM(APOLLO)
#include "Document.h"
#include "FrameLoaderClientApollo.h"
#include <WebKitApollo/WebKit.h>
#include "Frame.h"
#endif

namespace WebCore {

#if PLATFORM(APOLLO)
File::File(const String& path, Document* doc)
    : Blob(path)
    , m_name(pathGetFileName(path))
{
    // We don't use MIMETypeRegistry::getMIMETypeForPath() because it returns "application/octet-stream" upon failure.
    int index = m_name.reverseFind('.');
    if (index != -1)
        m_type = MIMETypeRegistry::getMIMETypeForExtension(m_name.substring(index + 1));

    bool hasApplicationPrivileges = false;
    bool versionIsAtLeast20 = false;

	// doc can be null when postMessage is used and serialization is needed (SerializedScriptValue), 
	// and we don't have a document/frame available at that time
    if (doc && doc->frame()) {
        hasApplicationPrivileges = doc->frame()->loader()->client()->hasApplicationPrivileges();
        WebHost* webHost = FrameLoaderClientApollo::clientApollo(doc->frame())->webHost();
        versionIsAtLeast20 = webHost->m_pVTable->isPlatformAPIVersion(webHost, WEBKIT_APOLLO_AIR_2_0);
    }
    
    m_shouldReturnFileNameOnly =  versionIsAtLeast20 && !hasApplicationPrivileges;   
}
#else
File::File(const String& path)
    : Blob(path)
    , m_name(pathGetFileName(path))
{
    // We don't use MIMETypeRegistry::getMIMETypeForPath() because it returns "application/octet-stream" upon failure.
    int index = m_name.reverseFind('.');
    if (index != -1)
        m_type = MIMETypeRegistry::getMIMETypeForExtension(m_name.substring(index + 1));
}
#endif
    
#if PLATFORM(APOLLO)
// from application sandbox return the full path
// from remote sandbox
//      for 1.5 return full path
//      for 2.0 return file name

const String& File::name() const
{
    return m_shouldReturnFileNameOnly ? m_name : path();
}

const String& File::fileName() const
{
    return m_shouldReturnFileNameOnly ? m_name : path();
}
#endif

} // namespace WebCore
