/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora, Ltd.  All rights reserved.
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
#include "PluginPackage.h"

#include "MIMETypeRegistry.h"
#include "PluginDatabase.h"
#include "PluginDebug.h"
#include "WebCoreNSStringExtras.h"
#include <wtf/text/CString.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

#if PLATFORM(APOLLO) && OS(DARWIN)
#include "TrappedCarbonFunctions.h"
#endif

#if PLATFORM(APOLLO)
#include "ApolloBundleResourcesScope.h"
#endif

namespace WebCore {

void PluginPackage::determineQuirks(const String& mimeType)
{
    // The flash plugin only requests windowless plugins if we return a mozilla user agent
    if (mimeType == "application/x-shockwave-flash") {
        m_quirks.add(PluginQuirkWantsMozillaUserAgent);
#if !PLATFORM(APOLLO)
        // Apollo already has an invalidation throttle system when drawing HTML
        // no need to do this here
        m_quirks.add(PluginQuirkThrottleInvalidate);
#endif
#if PLATFORM(APOLLO)
		m_quirks.add(PluginQuirkWantsSpecialFlashUserAgent);
		m_quirks.add(PluginQuirkNeedsCarbonAPITrapsForQD);
        m_quirks.add(PluginQuirkDontSetNullWindowHandleOnDestroy); // XXX - crashes, needs investigation
        m_quirks.add(PluginQuirkHasModalMessageLoop);
		m_quirks.add(PluginQuirkApolloSWFPlugin);
#endif
        m_quirks.add(PluginQuirkDetectUserInput);
#if CPU(PPC)
        m_quirks.add(PluginQuirkDoNotAllowWindowedMode);
#endif
    }
    else
    {
        //ASSERT(0); // don't have the rest of the quirks
    }

}

#if ! PLATFORM(APOLLO)
static WTF::RetainPtr<CFDictionaryRef> readPListFile(CFStringRef fileName, bool createFile, CFBundleRef bundle)
{
    if (createFile) {
        BP_CreatePluginMIMETypesPreferencesFuncPtr funcPtr =
        (BP_CreatePluginMIMETypesPreferencesFuncPtr)CFBundleGetFunctionPointerForName(bundle, CFSTR("BP_CreatePluginMIMETypesPreferences"));
        if (funcPtr)
            funcPtr();
    }
    
    WTF::RetainPtr<CFDictionaryRef> map;
    WTF::RetainPtr<CFURLRef> url =
    CFURLCreateWithFileSystemPath(kCFAllocatorDefault, fileName, kCFURLPOSIXPathStyle, false);
    
    CFDataRef resource = 0;
    SInt32 code;
    if (!CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, url.get(), &resource, 0, 0, &code))
        return map;
    
    WTF::RetainPtr<CFPropertyListRef> propertyList =
    CFPropertyListCreateFromXMLData(kCFAllocatorDefault, resource, kCFPropertyListImmutable, 0);
    
    CFRelease(resource);
    
    if (!propertyList)
        return map;
    
    if (CFGetTypeID(propertyList.get()) != CFDictionaryGetTypeID())
        return map;
    
    map = static_cast<CFDictionaryRef>(static_cast<CFPropertyListRef>(propertyList.get()));
    return map;
}

static Vector<String> stringListFromResourceId(SInt16 id)
{
    Vector<String> list;
    
    Handle handle = Get1Resource('STR#', id);
    if (!handle)
        return list;
    
    CFStringEncoding encoding = stringEncodingForResource(handle);
    
    unsigned char* p = (unsigned char*)*handle;
    if (!p)
        return list;
    
    SInt16 count = *(SInt16*)p;
    p += sizeof(SInt16);
    
    for (SInt16 i = 0; i < count; ++i) {
        unsigned char length = *p;
        WTF::RetainPtr<CFStringRef> str = CFStringCreateWithPascalString(0, p, encoding);
        list.append(str.get());
        p += 1 + length;
    }
    
    return list;
}
#endif // ! PLATFORM(APOLLO)
    
bool PluginPackage::fetchInfo()
{
    // m_module may not be loaded yet
    CString pluginPathUTF8 = m_path.utf8();
    WTF::RetainPtr<CFURLRef> flashPluginURL(CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)pluginPathUTF8.data(), pluginPathUTF8.length(), true));
    WTF::RetainPtr<CFBundleRef> const module(WTF::AdoptCF, CFBundleCreate(kCFAllocatorDefault, flashPluginURL.get()));
    
    BundleResourcesScope resources(module);
    if (!resources.isOk())
        return false;
    
    WTF::Vector<String> const nameDescriptionStrings(readResourceStringList(PluginNameOrDescriptionStringNumber));
    if (nameDescriptionStrings.size() != 2)
        return false;
    ASSERT(nameDescriptionStrings.size() == 2);
    
    m_description = nameDescriptionStrings[0];
    m_name = nameDescriptionStrings[1];
    if (m_name.isNull() || m_description.isNull())
        return false;

#if ! PLATFORM(APOLLO)
    WTF::RetainPtr<CFDictionaryRef> mimeDict;

    WTF::RetainPtr<CFTypeRef> mimeTypesFileName = CFBundleGetValueForInfoDictionaryKey(m_module, CFSTR("WebPluginMIMETypesFilename"));
    if (mimeTypesFileName && CFGetTypeID(mimeTypesFileName.get()) == CFStringGetTypeID()) {

        WTF::RetainPtr<CFStringRef> fileName = (CFStringRef)mimeTypesFileName.get();
        WTF::RetainPtr<CFStringRef> homeDir = homeDirectoryPath().createCFString();
        WTF::RetainPtr<CFStringRef> path = CFStringCreateWithFormat(0, 0, CFSTR("%@/Library/Preferences/%@"), homeDir.get(), fileName.get());

        WTF::RetainPtr<CFDictionaryRef> plist = readPListFile(path.get(), /*createFile*/ false, m_module);
        if (plist) {
            // If the plist isn't localized, have the plug-in recreate it in the preferred language.
            WTF::RetainPtr<CFStringRef> localizationName =
                (CFStringRef)CFDictionaryGetValue(plist.get(), CFSTR("WebPluginLocalizationName"));
            CFLocaleRef locale = CFLocaleCopyCurrent();
            if (localizationName != CFLocaleGetIdentifier(locale))
                plist = readPListFile(path.get(), /*createFile*/ true, m_module);

            CFRelease(locale);
        } else {
            // Plist doesn't exist, ask the plug-in to create it.
            plist = readPListFile(path.get(), /*createFile*/ true, m_module);
        }

        if (plist)
            mimeDict = (CFDictionaryRef)CFDictionaryGetValue(plist.get(), CFSTR("WebPluginMIMETypes"));
    }

    if (!mimeDict)
        mimeDict = (CFDictionaryRef)CFBundleGetValueForInfoDictionaryKey(m_module, CFSTR("WebPluginMIMETypes"));

    if (mimeDict) {
        CFIndex propCount = CFDictionaryGetCount(mimeDict.get());
        Vector<const void*, 128> keys(propCount);
        Vector<const void*, 128> values(propCount);
        CFDictionaryGetKeysAndValues(mimeDict.get(), keys.data(), values.data());
        for (int i = 0; i < propCount; ++i) {
            String mimeType = (CFStringRef)keys[i];
            mimeType = mimeType.lower();

            WTF::RetainPtr<CFDictionaryRef> extensionsDict = (CFDictionaryRef)values[i];

            WTF::RetainPtr<CFNumberRef> enabled = (CFNumberRef)CFDictionaryGetValue(extensionsDict.get(), CFSTR("WebPluginTypeEnabled"));
            if (enabled) {
                int enabledValue = 0;
                if (CFNumberGetValue(enabled.get(), kCFNumberIntType, &enabledValue) && enabledValue == 0)
                    continue;
            }

            Vector<String> mimeExtensions;
            WTF::RetainPtr<CFArrayRef> extensions = (CFArrayRef)CFDictionaryGetValue(extensionsDict.get(), CFSTR("WebPluginExtensions"));
            if (extensions) {
                CFIndex extensionCount = CFArrayGetCount(extensions.get());
                for (CFIndex i = 0; i < extensionCount; ++i) {
                    String extension =(CFStringRef)CFArrayGetValueAtIndex(extensions.get(), i);
                    extension = extension.lower();
                    mimeExtensions.append(extension);
                }
            }
            m_mimeToExtensions.set(mimeType, mimeExtensions);

            String description = (CFStringRef)CFDictionaryGetValue(extensionsDict.get(), CFSTR("WebPluginTypeDescription"));
            m_mimeToDescriptions.set(mimeType, description);
        }

        m_name = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(m_module, CFSTR("WebPluginName"));
        m_description = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(m_module, CFSTR("WebPluginDescription"));

    } else {
        int resFile = CFBundleOpenBundleResourceMap(m_module);

        UseResFile(resFile);

        Vector<String> mimes = stringListFromResourceId(MIMEListStringStringNumber);

        if (mimes.size() % 2 != 0)
            return false;

        Vector<String> descriptions = stringListFromResourceId(MIMEDescriptionStringNumber);
        if (descriptions.size() != mimes.size() / 2)
            return false;

        for (size_t i = 0;  i < mimes.size(); i += 2) {
            String mime = mimes[i].lower();
            Vector<String> extensions;
            mimes[i + 1].lower().split(UChar(','), extensions);

            m_mimeToExtensions.set(mime, extensions);

            m_mimeToDescriptions.set(mime, descriptions[i / 2]);
        }

        Vector<String> names = stringListFromResourceId(PluginNameOrDescriptionStringNumber);
        if (names.size() == 2) {
            m_description = names[0];
            m_name = names[1];
        }

        CFBundleCloseBundleResourceMap(m_module, resFile);
    }

    LOG(Plugins, "PluginPackage::fetchInfo(): Found plug-in '%s'", m_name.utf8().data());
    if (isPluginBlacklisted()) {
        LOG(Plugins, "\tPlug-in is blacklisted!");
        return false;
    }
#endif // ! PLATFORM(APOLLO)
        
    WTF::Vector<String> const mimeList(readResourceStringList(MIMEListStringStringNumber));
    if (mimeList.isEmpty() || ((mimeList.size() % 2) != 0))
        return false;
    WTF::Vector<String> const mimeDescriptionsList(readResourceStringList(MIMEDescriptionStringNumber));
    if (mimeList.size() != (mimeDescriptionsList.size() * 2 ))
        return false;
    
    for (unsigned i = 0; i < mimeDescriptionsList.size(); i++) {
        ASSERT(((i * 2) + 1) <  mimeList.size());
        String const mimeType(mimeList[i * 2]);
        String const extensionsString(mimeList[(i * 2) + 1]);
        WTF::Vector<String> extensions;
        extensionsString.split(',', extensions);
        m_mimeToExtensions.add(mimeType, extensions);
        
        String const mimeDescription(mimeDescriptionsList[i]);
        m_mimeToDescriptions.add(mimeType, mimeDescription);
        
        determineQuirks(mimeType);
    }
    
    return true;
}

bool PluginPackage::isPluginBlacklisted()
{
    return false;
}

#if PLATFORM(APOLLO) && OS(DARWIN) && !defined(NP_NO_CARBON)
bool PluginPackage::load(bool const trapCarbonAPI)
#else
bool PluginPackage::load()
#endif
{
    if (m_freeLibraryTimer.isActive()) {
        ASSERT(m_module);
        m_freeLibraryTimer.stop();
    } else if (m_isLoaded) {
        if (m_quirks.contains(PluginQuirkDontAllowMultipleInstances))
            return false;
        m_loadCount++;
        return true;
    }
    else {
        CString pluginPathUTF8 = m_path.utf8();
        WTF::RetainPtr<CFURLRef> flashPluginURL(AdoptCF, CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)pluginPathUTF8.data(), pluginPathUTF8.length(), false));
        m_module = CFBundleCreate(kCFAllocatorDefault, flashPluginURL.get());
        ASSERT(m_module);
        
        if (!CFBundleLoadExecutable(m_module)) {
            CFRelease(m_module);
            m_module = 0;
            return false;
        }
    }
    
    m_isLoaded = true;
    
    NP_GetEntryPointsFuncPtr NP_GetEntryPoints = 0;
    NP_InitializeFuncPtr NP_Initialize = 0;
    NPError npErr;
    
    NP_Initialize = (NP_InitializeFuncPtr)CFBundleGetFunctionPointerForName(m_module, CFSTR("NP_Initialize"));
    NP_GetEntryPoints = (NP_GetEntryPointsFuncPtr)CFBundleGetFunctionPointerForName(m_module, CFSTR("NP_GetEntryPoints"));
    m_NPP_Shutdown = (NPP_ShutdownProcPtr)CFBundleGetFunctionPointerForName(m_module, CFSTR("NP_Shutdown"));
    
    if (!NP_Initialize || !NP_GetEntryPoints || !m_NPP_Shutdown)
        goto abort;

#if PLATFORM(APOLLO) && OS(DARWIN) && !defined(NP_NO_CARBON)
    if (trapCarbonAPI) {
        Dl_info dlInfo;
        dladdr((const void*)NP_Initialize, &dlInfo);
        trapCarbonFunctions(&dlInfo);
    }
#endif
	
    memset(&m_pluginFuncs, 0, sizeof(m_pluginFuncs));
    m_pluginFuncs.size = sizeof(m_pluginFuncs);
    
    npErr = NP_GetEntryPoints(&m_pluginFuncs);
    LOG_NPERROR(npErr);
    if (npErr != NPERR_NO_ERROR)
        goto abort;

// apollo - compile - integrate - 58803
#if ENABLE(NETSCAPE_PLUGIN_API)
	initializeBrowserFuncs();
#endif
	
    npErr = NP_Initialize(&m_browserFuncs);
    LOG_NPERROR(npErr);
    
    if (npErr != NPERR_NO_ERROR)
        goto abort;
    
    m_loadCount++;
    return true;
abort:
    unloadWithoutShutdown();
    return false;
}

uint16_t PluginPackage::NPVersion() const
{
    return NP_VERSION_MINOR;
}
    
}
