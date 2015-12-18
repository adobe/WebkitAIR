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
#include "PluginPackage.h"
#include "Frame.h"
#include <WebKitApollo/WebKit.h>
#include "TextEncoding.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// Adobe Reader Specific
#define MAX_VERSION_LEN     256
#define MAX_PATH            1024

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions; }

namespace WebCore {

static void addAdobeAcrobatPluginDirectory(Vector<String>& directories);

#if !PLATFORM(APOLLO)
void PluginDatabase::getPluginPathsInDirectories(HashSet<String>& paths) const
{
    // FIXME: This should be a case insensitive set.
    HashSet<String> uniqueFilenames;

    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW findFileData;

    String oldWMPPluginPath;
    String newWMPPluginPath;

    Vector<String>::const_iterator end = m_pluginDirectories.end();
    for (Vector<String>::const_iterator it = m_pluginDirectories.begin(); it != end; ++it) {
        String pattern = *it + "\\*";

        hFind = FindFirstFileW(pattern.charactersWithNullTermination(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE)
            continue;

        do {
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            String filename = String(findFileData.cFileName, wcslen(findFileData.cFileName));
            if ((!filename.startsWith("np", false) || !filename.endsWith("dll", false)) &&
                (!equalIgnoringCase(filename, "Plugin.dll") || !it->endsWith("Shockwave 10", false)))
                continue;

            String fullPath = *it + "\\" + filename;
            if (!uniqueFilenames.add(fullPath).second)
                continue;

            paths.add(fullPath);

            if (equalIgnoringCase(filename, "npdsplay.dll"))
                oldWMPPluginPath = fullPath;
            else if (equalIgnoringCase(filename, "np-mswmp.dll"))
                newWMPPluginPath = fullPath;

        } while (FindNextFileW(hFind, &findFileData) != 0);

        FindClose(hFind);
    }

    addPluginPathsFromRegistry(HKEY_LOCAL_MACHINE, paths);
    addPluginPathsFromRegistry(HKEY_CURRENT_USER, paths);

    // If both the old and new WMP plugin are present in the plugins set, 
    // we remove the old one so we don't end up choosing the old one.
    if (!oldWMPPluginPath.isEmpty() && !newWMPPluginPath.isEmpty())
        paths.remove(oldWMPPluginPath);
}
#else
void PluginDatabase::getPluginPathsInDirectories(HashSet<String>& paths) const
{
    unsigned long flashPlayerPluginPathUTF8ByteLen = 0;
    unsigned char* const flashPlayerPluginPathUTF8Bytes = WebKitApollo::g_HostFunctions->getFlashPlayerPluginPathUTF8Bytes(&flashPlayerPluginPathUTF8ByteLen);
    ASSERT(flashPlayerPluginPathUTF8Bytes);
    ASSERT(flashPlayerPluginPathUTF8ByteLen > 0);

    String flashPlayerPluginPath(UTF8Encoding().decode(reinterpret_cast<const char*>(flashPlayerPluginPathUTF8Bytes), flashPlayerPluginPathUTF8ByteLen));
    WebKitApollo::g_HostFunctions->freeBytes(flashPlayerPluginPathUTF8Bytes);

    time_t lastModified;
    if (getFileModificationTime(flashPlayerPluginPath, lastModified))
    {
        paths.add(flashPlayerPluginPath);
    }

    // Adobe Reader
    
    WebString* pdfPluginPath = WebKitApollo::g_HostFunctions->getPDFPluginPathWebString();
    if(pdfPluginPath)
    {
        paths.add(adoptWebString(pdfPluginPath));
    }
    
}
#endif

static inline Vector<int> parseVersionString(const String& versionString)
{
    Vector<int> version;

    unsigned startPos = 0;
    unsigned endPos;
    
    while (startPos < versionString.length()) {
        for (endPos = startPos; endPos < versionString.length(); ++endPos)
            if (versionString[endPos] == '.' || versionString[endPos] == '_')
                break;

        int versionComponent = versionString.substring(startPos, endPos - startPos).toInt();
        version.append(versionComponent);

        startPos = endPos + 1;
    }

    return version;
}

// This returns whether versionA is higher than versionB
static inline bool compareVersions(const Vector<int>& versionA, const Vector<int>& versionB)
{
    for (unsigned i = 0; i < versionA.size(); i++) {
        if (i >= versionB.size())
            return true;

        if (versionA[i] > versionB[i])
            return true;
        else if (versionA[i] < versionB[i])
            return false;
    }

    // If we come here, the versions are either the same or versionB has an extra component, just return false
    return false;
}

static inline void addMozillaPluginDirectories(Vector<String>& directories)
{
    // Fix this when plugin directories are supported
/*
    // Enumerate all Mozilla plugin directories in the registry
    HKEY key;
    LONG result;
    
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Mozilla"), 0, KEY_READ, &key);
    if (result == ERROR_SUCCESS) {
        WCHAR name[128];
        FILETIME lastModified;

        // Enumerate subkeys
        for (int i = 0;; i++) {
            DWORD nameLen = sizeof(name) / sizeof(WCHAR);
            result = RegEnumKeyExW(key, i, name, &nameLen, 0, 0, 0, &lastModified);

            if (result != ERROR_SUCCESS)
                break;

            String extensionsPath = String(name, nameLen) + "\\Extensions";
            HKEY extensionsKey;

            // Try opening the key
            result = RegOpenKeyEx(key, extensionsPath.charactersWithNullTermination(), 0, KEY_READ, &extensionsKey);

            if (result == ERROR_SUCCESS) {
                // Now get the plugins directory
                WCHAR pluginsDirectoryStr[_MAX_PATH];
                DWORD pluginsDirectorySize = sizeof(pluginsDirectoryStr);
                DWORD type;

                result = RegQueryValueEx(extensionsKey, TEXT("Plugins"), 0, &type, (LPBYTE)&pluginsDirectoryStr, &pluginsDirectorySize);

                if (result == ERROR_SUCCESS && type == REG_SZ)
                    directories.append(String(pluginsDirectoryStr, pluginsDirectorySize / sizeof(WCHAR) - 1));

                RegCloseKey(extensionsKey);
            }
        }
        
        RegCloseKey(key);
    }
*/
}

static bool FindAcrobatInUserPath (char *readerPath)
{
    bool    found = false;

    // We would go and get the PATH set by the user and see if there exists acroread 
    gchar    *absPath = g_find_program_in_path ("acroread");

    if(!absPath)
        return found;

    gchar    linkPath[MAX_PATH];
    strncpy(linkPath, absPath, MAX_PATH);
    while(1)
    {
        struct stat    fileStat;
        char         buf[MAX_PATH];

        lstat(linkPath, &fileStat);
        if((fileStat.st_mode & S_IFLNK) == S_IFLNK)
        {
            // If the link is invalid
            if(readlink(linkPath, buf, MAX_PATH) == -1)
                break;

            // If the link is valid, go further
            strncpy(linkPath, buf, MAX_PATH);
        }
        else
        {
            // If the path is not a link, go 3 levels up to get to the base dir
            char *till_bin = g_path_get_dirname (linkPath);
            char *till_reader = g_path_get_dirname (till_bin);
            char *till_adobe = g_path_get_dirname (till_reader);

            strncpy(readerPath, till_adobe, MAX_PATH);
            found = true;

            free(till_bin);
            free(till_reader);
            free(till_adobe);

            break;
        }
    }

    free(absPath);
    return found;
}

static WebCore::String FindAcrobat(const char *baseFolder)
{    
    const char *minVersionRequired = "8.1.1";
    WebCore::String latestFind;

    // Check all the directories in the base folder provided
    // TODO: locale specific changes for localized input
    DIR* baseDir = opendir(baseFolder);
    struct dirent* dirEntry;
    if (baseDir)
    {
        while ( (dirEntry = readdir(baseDir)) != NULL )
        {
            // Each dirEntry here would correspond to a different version of Reader

            // First check the AcroVersion file and see if the version is proper
            char readerVersionFile[MAX_PATH];
            strncpy(readerVersionFile, baseFolder, MAX_PATH);
            strncat(readerVersionFile, "/", MAX_PATH);
            strncat(readerVersionFile, dirEntry->d_name, MAX_PATH);
            strncat(readerVersionFile, "/Reader/AcroVersion", MAX_PATH);

            FILE *versionFD = fopen(readerVersionFile, "r");
            if(versionFD)    // File exists
            {
                char versionString[MAX_VERSION_LEN];
                fread(&versionString, MAX_VERSION_LEN, 1, versionFD);
                fclose(versionFD);
                // TODO : numeric comparison
                if(strcmp(versionString, minVersionRequired) < 0)
                    continue;
            }
            else
                continue;
            // Now create the plugin path
            char pluginFilePath[MAX_PATH];
            strncpy(pluginFilePath, baseFolder, MAX_PATH);
            strncat(pluginFilePath, "/", MAX_PATH);
            strncat(pluginFilePath, dirEntry->d_name, MAX_PATH);
            strncat(pluginFilePath, "/Browser/intellinux/nppdf.so", MAX_PATH);
            // TODO : TBD :  Later, give AIR instead of Browser in the folder path

            latestFind = WebCore::String(pluginFilePath);
            break;
        }
    }
    closedir(baseDir);

    return latestFind;
}


static inline void addAdobeAcrobatPluginDirectory(Vector<String>& directories)
{
    WebCore::String path;

    char readerPathBase[MAX_PATH];
    if(FindAcrobatInUserPath(readerPathBase))
    {
        path = FindAcrobat(readerPathBase);
        if(!path.isEmpty())
            directories.append(path);
    }

    path = FindAcrobat("/opt/Adobe");
    if(!path.isEmpty())
        directories.append(path);
}

static inline String safariPluginsDirectory()
{
    // Fix this when plugin directories are supported
    static String pluginsDirectory;
/*
    WCHAR moduleFileNameStr[_MAX_PATH];
    static bool cachedPluginDirectory = false;

    if (!cachedPluginDirectory) {
        cachedPluginDirectory = true;

    int moduleFileNameLen = GetModuleFileName(0, moduleFileNameStr, _MAX_PATH);

        if (!moduleFileNameLen || moduleFileNameLen == _MAX_PATH)
            goto exit;

        if (!PathRemoveFileSpec(moduleFileNameStr))
            goto exit;

        pluginsDirectory = String(moduleFileNameStr) + "\\Plugins";
    }
exit:
*/
    return pluginsDirectory;
}

static inline void addMacromediaPluginDirectories(Vector<String>& directories)
{
#if PLATFORM(APOLLO)
    unsigned long flashPlayerPluginPathUTF8ByteLen = 0;
    unsigned char* const flashPlayerPluginPathUTF8Bytes = WebKitApollo::g_HostFunctions->getFlashPlayerPluginPathUTF8Bytes(&flashPlayerPluginPathUTF8ByteLen);
    ASSERT(flashPlayerPluginPathUTF8Bytes);
    ASSERT(flashPlayerPluginPathUTF8ByteLen > 0);

    String path = UTF8Encoding().decode(reinterpret_cast<const char*>(flashPlayerPluginPathUTF8Bytes), flashPlayerPluginPathUTF8ByteLen);
    directories.append(path);
    WebKitApollo::g_HostFunctions->freeBytes(flashPlayerPluginPathUTF8Bytes);
    
#else
    // Fix this when plugin directories are supported
/*
    WCHAR systemDirectoryStr[MAX_PATH];

    if (GetSystemDirectory(systemDirectoryStr, _countof(systemDirectoryStr)) == 0)
        return;

    WCHAR macromediaDirectoryStr[MAX_PATH];

    PathCombine(macromediaDirectoryStr, systemDirectoryStr, TEXT("macromed\\Flash"));
    directories.append(macromediaDirectoryStr);

    PathCombine(macromediaDirectoryStr, systemDirectoryStr, TEXT("macromed\\Shockwave 10"));
    directories.append(macromediaDirectoryStr);
*/
#endif
}

Vector<String> PluginDatabase::defaultPluginDirectories()
{
    Vector<String> directories;
#if !PLATFORM(APOLLO)
    String ourDirectory = safariPluginsDirectory();

    if (!ourDirectory.isNull())
        directories.append(ourDirectory);
    addAdobeAcrobatPluginDirectory(directories);
    addMozillaPluginDirectories(directories);
#endif

    addMacromediaPluginDirectories(directories);

    return directories;
}

bool PluginDatabase::isPreferredPluginDirectory(const String& directory)
{
    String ourDirectory = safariPluginsDirectory();

    if (!ourDirectory.isNull() && !directory.isNull())
        return ourDirectory == directory;

    return false;
}

}
