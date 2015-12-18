/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Collabora, Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FileSystem.h"
#include "PlatformString.h"
#include "SharedBuffer.h"
#include <WebKitApollo/WebKit.h>
#include <wtf/text/CString.h>

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions;}

namespace WebCore {

namespace {
class WebStringHolder
{
public:
    WebStringHolder(WebString* const webString) throw();
    ~WebStringHolder() throw();
    operator WebString*() const throw();
private:
    WebString* const m_webString;
    WebStringHolder(const WebStringHolder&);
    WebStringHolder& operator=(const WebStringHolder&);
};

inline WebStringHolder::WebStringHolder(WebString* const webString) throw()
    : m_webString(webString)
{
    ASSERT(m_webString);
}

inline WebStringHolder::~WebStringHolder() throw()
{
    ASSERT(m_webString);
    ASSERT(m_webString->m_vTable);
    ASSERT(m_webString->m_vTable->free);
    m_webString->m_vTable->free(m_webString);
}

inline WebStringHolder::operator WebString*() const throw()
{
    return m_webString;
}

}

bool getFileSize(const String& path, long long& result)
{
    WebStringHolder const pathWebStr(path.webString());
    return WebKitApollo::g_HostFunctions->getFileSize(pathWebStr, &result) != 0;
}

bool getFileModificationTime(const String& path, time_t& time)
{
    WebStringHolder const pathWebStr(path.webString());
    WebTime modTime = 0;
    bool success = WebKitApollo::g_HostFunctions->getFileModTime(pathWebStr, &modTime) != 0;
    
    time = static_cast<time_t>(modTime);
    return success;
}

bool fileExists(const String& path) 
{
    WebStringHolder const pathWebStr(path.webString());
    return WebKitApollo::g_HostFunctions->getFileExists(pathWebStr) != 0;
}

bool deleteFile(const String& path)
{
    WebStringHolder const pathWebStr(path.webString());
    return WebKitApollo::g_HostFunctions->deleteFile(pathWebStr) != 0;
}

bool deleteEmptyDirectory(const String& path)
{
    WebStringHolder const pathWebStr(path.webString());
    return WebKitApollo::g_HostFunctions->deleteEmptyDirectory(pathWebStr) != 0;
}

String pathByAppendingComponent(const String& path, const String& /*component*/)
{
    WebStringHolder const pathWebStr(path.webString());
    WebStringHolder const componentWebStr(path.webString());

    return adoptWebString(WebKitApollo::g_HostFunctions->addComponentToPath(pathWebStr, componentWebStr));
}

CString fileSystemRepresentation(const String&)
{
    ASSERT(0);
    return "";
}

bool makeAllDirectories(const String& path)
{
    WebStringHolder const pathWebStr(path.webString());
    return WebKitApollo::g_HostFunctions->makeAllDirectories(pathWebStr);
}

String homeDirectoryPath()
{
    ASSERT(0);
    return "";
}

String pathGetFileName(const String& path)
{
    WebStringHolder const pathWebStr(path.webString());
    return adoptWebString(WebKitApollo::g_HostFunctions->pathGetFileName(pathWebStr));
}

String directoryName(const String& /*path*/)
{
    ASSERT(0);
    return String();
}

CString openTemporaryFile(const char* prefix, PlatformFileHandle& handle)
{
    WebStringHolder const prefixWebStr(String(prefix).webString());
    return adoptWebString(WebKitApollo::g_HostFunctions->openTempFile(prefixWebStr, &handle)).utf8();
}

void closeFile(PlatformFileHandle& handle)
{
    WebKitApollo::g_HostFunctions->closeFile(handle);
}

int writeToFile(PlatformFileHandle handle, const char* data, int length)
{
    return WebKitApollo::g_HostFunctions->writeToFile(handle, data, length);
}

bool unloadModule(PlatformModule module)
{
    return WebKitApollo::g_HostFunctions->unloadModule(module);
}

String localUserSpecificStorageDirectory();
String localUserSpecificStorageDirectory()
{
    ASSERT(0);
    return String();
}

String roamingUserSpecificStorageDirectory();
String roamingUserSpecificStorageDirectory()
{
    ASSERT(0);
    return String();
}

PassRefPtr<SharedBuffer> SharedBuffer::createWithContentsOfFile(const String& filePath)
{
    if (filePath.isEmpty())
        return 0;

    ASSERT(0);

    return 0;
}

} // namespace WebCore
