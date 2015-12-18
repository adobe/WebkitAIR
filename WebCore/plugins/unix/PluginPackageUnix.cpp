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
#include "PluginPackage.h"

#include "npruntime_impl.h"
#include "PluginDebug.h"
#include <wtf/text/CString.h>

#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <glib.h>

#if PLATFORM(APOLLO) && ENABLE(NETSCAPE_PLUGIN_API)
#include "PluginView.h"

extern "C"
{
	//	We make a special call to the plug-in with this structure;
	//	For the Apollo entry point, on an instance-by-instance bases.
	typedef void (*ApolloBeginModalProc)(NPP instance);
	typedef void (*ApolloEndModalProc)(NPP instance);

	#define	APOLLO_ENTRY_PARAM_VERSION			1

	typedef struct _ApolloEntryParam {
		unsigned long			size;
		unsigned long			apolloVersion;
		ApolloBeginModalProc	beginModal;
		ApolloEndModalProc		endModal;
	} ApolloEntryParamRec, *ApolloEntryParam;

	typedef NPError (*NP_ApolloEntryFuncPtr)( ApolloEntryParam apolloEntryParams );

}

static WebCore::PluginView* pluginViewForInstance(NPP instance)
{
    if (instance && instance->ndata)
        return static_cast<WebCore::PluginView*>(instance->ndata);
    return WebCore::PluginView::currentPluginView();
}

void NP_ApolloBeginModal(NPP instance)
{
    pluginViewForInstance(instance)->beginModal();
}

void NP_ApolloEndModal(NPP instance)
{
    pluginViewForInstance(instance)->endModal();
}

ApolloEntryParamRec sApolloEntryFuncs =
{
    sizeof(ApolloEntryParamRec),
    APOLLO_ENTRY_PARAM_VERSION,
    (ApolloBeginModalProc) &NP_ApolloBeginModal,
    (ApolloEndModalProc) &NP_ApolloEndModal
};
#endif

namespace WebCore {

// apollo integrate
// this is not done correctly in the PluginPackage.cpp
/*
int PluginPackage::compareFileVersion(const PlatformModuleVersion& compareVersion) const
{
    // mostSig and leastSig are not available on Linux - so we cannot compare 
    return 0;
}
 */

bool PluginPackage::isPluginBlacklisted()
{
    return false;
}

bool PluginPackage::fetchInfo()
{
    // Load the library
    void *module = dlopen(g_locale_from_utf8(m_path.utf8().data(), m_path.length(), NULL, NULL, NULL), RTLD_LAZY);
    if (!module)
    {
        char *error = dlerror();
        return false;
    }

    NP_GetValueFuncPtr NP_GetValue = (NP_GetValueFuncPtr)dlsym(module, "NP_GetValue");
    NP_GetMIMEDescriptionFuncPtr NP_GetMIMEDescription = (NP_GetMIMEDescriptionFuncPtr)dlsym(module, "NP_GetMIMEDescription");

    char *str_name, *str_description;

    NP_GetValue(0, NPPVpluginNameString, (void*)(&str_name));
    m_name = String(str_name);
    NP_GetValue(0, NPPVpluginDescriptionString, (void*)(&str_description));
    m_description = String(str_description);

    if (m_name.isNull() || m_description.isNull()) {
        dlclose(module);
        return false;
    }

    String mimeCompleteString = String(NP_GetMIMEDescription());

    Vector<String> mimeEntries;
    mimeCompleteString.split(';', mimeEntries);
    for (unsigned i = 0; i < mimeEntries.size(); i++) {

        String mimeEntry = mimeEntries[i];
        Vector<String> mimeEntryParts;
        mimeEntry.split(':', mimeEntryParts);

        Vector<String> mimeExtensions;
        mimeEntryParts[1].split(',', mimeExtensions);
        m_mimeToExtensions.add(mimeEntryParts[0], mimeExtensions);
        m_mimeToDescriptions.add(mimeEntryParts[0], mimeEntryParts[2]);

        // Determine the quirks for the MIME types this plug-in supports
        determineQuirks(mimeEntryParts[0]);
    }

    dlclose(module);
    return true;
}

bool PluginPackage::load()
{
    if (m_freeLibraryTimer.isActive()) {
        ASSERT(m_module);
        m_freeLibraryTimer.stop();
    }
    else if (m_isLoaded) {
        m_loadCount++;
        return true;
    }
    else
    {
        // Load the library
        m_module = dlopen(g_locale_from_utf8(m_path.utf8().data(), m_path.length(), NULL, NULL, NULL), RTLD_LAZY);
    }

    if (!m_module)
        return false;

    m_isLoaded = true;

    NP_InitializeFuncPtr NP_Initialize = 0;
    NPError npErr;

#if PLATFORM(APOLLO) && ENABLE(NETSCAPE_PLUGIN_API)
    NP_ApolloEntryFuncPtr NP_ApolloEntry = 0;
#endif

    NP_Initialize = (NP_InitializeFuncPtr)dlsym(m_module, "NP_Initialize");
    m_NPP_Shutdown = (NPP_ShutdownProcPtr)dlsym(m_module, "NP_Shutdown");

    if (!NP_Initialize || !m_NPP_Shutdown)
        goto abort;

#if PLATFORM(APOLLO) && ENABLE(NETSCAPE_PLUGIN_API)
    if(m_quirks.contains(PluginQuirkApolloPDFPlugin))
    {
        NP_ApolloEntry = (NP_ApolloEntryFuncPtr)dlsym(m_module, "NP_ApolloEntry");
        if(!NP_ApolloEntry)
            goto abort;

        if(NP_ApolloEntry(&sApolloEntryFuncs) != NPERR_NO_ERROR)
            goto abort;
    }
#endif

    memset(&m_pluginFuncs, 0, sizeof(m_pluginFuncs));
    m_pluginFuncs.size = sizeof(m_pluginFuncs);

#if ENABLE(NETSCAPE_PLUGIN_API)
	initializeBrowserFuncs();
#endif

    npErr = NP_Initialize(&m_browserFuncs, &m_pluginFuncs);
    LOG_NPERROR(npErr);

    if (npErr != NPERR_NO_ERROR)
        goto abort;

    m_loadCount++;
    return true;
abort:
    unloadWithoutShutdown();
    return false;
}

#if !ENABLE(PLUGIN_PACKAGE_SIMPLE_HASH)
unsigned PluginPackage::hash() const
{
    const unsigned hashCodes[] = {
        m_name.impl()->hash(),
        m_description.impl()->hash(),
        m_mimeToExtensions.size()
    };

    return StringImpl::computeHash(reinterpret_cast<const UChar*>(hashCodes), sizeof(hashCodes) / sizeof(UChar));
}

bool PluginPackage::equal(const PluginPackage& a, const PluginPackage& b)
{
    if (a.m_name != b.m_name)
        return false;

    if (a.m_description != b.m_description)
        return false;

    if (a.m_mimeToExtensions.size() != b.m_mimeToExtensions.size())
        return false;

    MIMEToExtensionsMap::const_iterator::Keys end = a.m_mimeToExtensions.end().keys();
    for (MIMEToExtensionsMap::const_iterator::Keys it = a.m_mimeToExtensions.begin().keys(); it != end; ++it) {
        if (!b.m_mimeToExtensions.contains(*it))
            return false;
    }

    return true;
}
#endif

uint16_t PluginPackage::NPVersion() const
{
    return NP_VERSION_MINOR;
}

}
