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
#include <config.h>
#include <WebKitApollo/WebKit.h>
#include <wtf/Assertions.h>

#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif

#include "WebViewImpl.h"
#include "Collector.h"
#include "JSLock.h"

#include <Document.h>
#include <wtf/InstanceCounter.h>
#include <Cache.h>

#include "GCController.h"
#if PLATFORM(APOLLO_WINDOWS)
#include <WebCoreInstanceHandle.h>
#include <windows.h>
#endif


namespace WebKitApollo
{
void InitWebCoreSystemInterface(void);

WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 webKitUpdateDirtyDocuments();
WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 webKitUpdateDirtyDocuments()
{
    WebCore::Document::updateStyleForAllDocuments();
}


WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 webGarbageCollectNow();
WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 webGarbageCollectNow()
{
    JSC::JSLock jsLock(false);
	WebCore::gcController().garbageCollectNow();
}

WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 webDestroy();
WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 webDestroy()
{
    // clear the cache when HTMLLoader is destroyed
    WebCore::cache()->setDisabled(true);
    WebCore::cache()->setDisabled(false);
}

static const WebKitAPI s_WebKitAPI = 
{
    sizeof( WebKitAPI ),
    webViewCreate,
    webKitUpdateDirtyDocuments,
    webGarbageCollectNow,
    webDestroy
};

static WebKitAPIHostFunctions hostFunctions = {
    sizeof( WebKitAPIHostFunctions ),
    0, // freeBytes;
    0, // allocBytes
    0, // setSharedTimerFireTime
    0, // setSharedTimerFiredFunction
    0, // stopSharedTimer
    0, // screenGetDepth
    0, // screenGetDepthPerComponent
    0, // screenIsMonochrome
    0, // screenGetRect
    0, // screenGetUsableRect
    0, // getFlashPlayerPluginPathUTF8Bytes
    0, // getUserAgentForFlashNPP
    0, // initRandomNumberGenerator
    0, // getRandomNumber
    0, // setJavaScriptCookies
    0, // getJavaScriptCookies
    0, // getMIMETypeFromExtension
    0, // getLocalizedText
    0, // getFileSize
    0, // getFileModTime
    0, // getFileExists
    0, // deleteFile
    0, // deleteEmptyDirectory
    0, // addComponentToPath
    0, // makeAllDirectories
    0, // pathGetFileName
    0, // openTempFile
    0, // closeFile
    0, // writeToFile
    0, // unloadModule
    0, // getWebDebugListener
    0, // isDebuggingEnabled
    0, // logWarning    
    0, // getCapsLockState
    0, // getKeyModifiersState
    0, // getCopyKeyState
    0, // newBreakIterator
    0, // deleteBreakIterator
    0, // textBreakFirst
	0, // textBreakLast
	0, // textBreakPrev
    0, // textBreakNext
    0, // textBreakCurrent
    0, // textBreakPreceding
    0, // textBreakFollowing
	0, // textBreakSetText
    0, // isTextBreak
    0, // mapChar
    0, // mapString
    0, // getPDFPluginPathWebString
    0  // getWebProfilerListener
};

  

WebKitAPIHostFunctions* g_HostFunctions = 0;

// Similar values can be found in Safari/Chrome for 512MB of RAM.
static const unsigned CacheTotalCapacity = 32 * 1024 * 1024;
static const unsigned CacheMinDeadCapacity = CacheTotalCapacity / 4;
static const unsigned CacheMaxDeadCapacity = CacheTotalCapacity / 2;	

}

extern "C" WEBKIT_APOLLO_PUBLIC WEBKIT_APOLLO_PROTO1 const WebKitAPI* WEBKIT_APOLLO_PROTO2 WebKitGetAPI(WebKitAPIHostFunctions* pHostFunctions, WebError* pErrorInfo);
extern "C" WEBKIT_APOLLO_PUBLIC WEBKIT_APOLLO_PROTO1 const WebKitAPI* WEBKIT_APOLLO_PROTO2 WebKitGetAPI(WebKitAPIHostFunctions* pHostFunctions, WebError* pErrorInfo)
{
    ASSERT(pErrorInfo);
    
    ASSERT((!WebKitApollo::g_HostFunctions) || (memcmp(pHostFunctions, WebKitApollo::g_HostFunctions, std::min<unsigned long>(pHostFunctions->m_size, sizeof(WebKitAPIHostFunctions))) == 0));
    
    const WebKitAPI* result = NULL;
    if (!WebKitApollo::g_HostFunctions)
    {
        WebKitApollo::InitWebCoreSystemInterface();
        if (pHostFunctions->m_size >= sizeof(WebKitAPIHostFunctions)) {
            WebKitApollo::hostFunctions.freeBytes = pHostFunctions->freeBytes;
            WebKitApollo::hostFunctions.allocBytes = pHostFunctions->allocBytes;
            WebKitApollo::hostFunctions.setSharedTimerFireTime = pHostFunctions->setSharedTimerFireTime;
            WebKitApollo::hostFunctions.setSharedTimerFiredFunction = pHostFunctions->setSharedTimerFiredFunction;
            WebKitApollo::hostFunctions.stopSharedTimer = pHostFunctions->stopSharedTimer;
            WebKitApollo::hostFunctions.screenGetDepth = pHostFunctions->screenGetDepth;
            WebKitApollo::hostFunctions.screenGetDepthPerComponent = pHostFunctions->screenGetDepthPerComponent;
            WebKitApollo::hostFunctions.screenIsMonochrome = pHostFunctions->screenIsMonochrome;
            WebKitApollo::hostFunctions.screenGetRect = pHostFunctions->screenGetRect;
            WebKitApollo::hostFunctions.screenGetUsableRect = pHostFunctions->screenGetUsableRect;
            WebKitApollo::hostFunctions.getFlashPlayerPluginPathUTF8Bytes = pHostFunctions->getFlashPlayerPluginPathUTF8Bytes;
            WebKitApollo::hostFunctions.getUserAgentForFlashNPP = pHostFunctions->getUserAgentForFlashNPP;
            WebKitApollo::hostFunctions.initRandomNumberGenerator = pHostFunctions->initRandomNumberGenerator;
            WebKitApollo::hostFunctions.getRandomNumber = pHostFunctions->getRandomNumber;
            WebKitApollo::hostFunctions.setJavaScriptCookies = pHostFunctions->setJavaScriptCookies;
            WebKitApollo::hostFunctions.getJavaScriptCookies = pHostFunctions->getJavaScriptCookies;
            WebKitApollo::hostFunctions.getMIMETypeFromExtension = pHostFunctions->getMIMETypeFromExtension;
            WebKitApollo::hostFunctions.getLocalizedText = pHostFunctions->getLocalizedText;
            WebKitApollo::hostFunctions.getFileSize = pHostFunctions->getFileSize;
            WebKitApollo::hostFunctions.getFileModTime = pHostFunctions->getFileModTime;
            WebKitApollo::hostFunctions.getFileExists = pHostFunctions->getFileExists;
            WebKitApollo::hostFunctions.deleteFile = pHostFunctions->deleteFile;
            WebKitApollo::hostFunctions.deleteEmptyDirectory = pHostFunctions->deleteEmptyDirectory;
            WebKitApollo::hostFunctions.addComponentToPath = pHostFunctions->addComponentToPath;
            WebKitApollo::hostFunctions.makeAllDirectories = pHostFunctions->makeAllDirectories;
            WebKitApollo::hostFunctions.pathGetFileName = pHostFunctions->pathGetFileName;
            WebKitApollo::hostFunctions.openTempFile = pHostFunctions->openTempFile;
            WebKitApollo::hostFunctions.closeFile = pHostFunctions->closeFile;
            WebKitApollo::hostFunctions.writeToFile = pHostFunctions->writeToFile;
            WebKitApollo::hostFunctions.unloadModule = pHostFunctions->unloadModule;
            WebKitApollo::hostFunctions.getWebDebugListener = pHostFunctions->getWebDebugListener;
            WebKitApollo::hostFunctions.isDebuggingEnabled = pHostFunctions->isDebuggingEnabled;
            WebKitApollo::hostFunctions.getCapsLockState = pHostFunctions->getCapsLockState;
            WebKitApollo::hostFunctions.getCopyKeyState = pHostFunctions->getCopyKeyState;
            WebKitApollo::hostFunctions.getKeyModifiersState = pHostFunctions->getKeyModifiersState;
            WebKitApollo::hostFunctions.logWarning = pHostFunctions->logWarning;

            WebKitApollo::hostFunctions.newBreakIterator = pHostFunctions->newBreakIterator;
            WebKitApollo::hostFunctions.deleteBreakIterator = pHostFunctions->deleteBreakIterator;
            WebKitApollo::hostFunctions.textBreakFirst = pHostFunctions->textBreakFirst;
			WebKitApollo::hostFunctions.textBreakLast = pHostFunctions->textBreakLast;
            WebKitApollo::hostFunctions.textBreakPrev = pHostFunctions->textBreakPrev;
            WebKitApollo::hostFunctions.textBreakNext = pHostFunctions->textBreakNext;
            WebKitApollo::hostFunctions.textBreakCurrent = pHostFunctions->textBreakCurrent;
            WebKitApollo::hostFunctions.textBreakPreceding = pHostFunctions->textBreakPreceding;
            WebKitApollo::hostFunctions.textBreakFollowing = pHostFunctions->textBreakFollowing;
			WebKitApollo::hostFunctions.textBreakSetText = pHostFunctions->textBreakSetText;
            WebKitApollo::hostFunctions.isTextBreak = pHostFunctions->isTextBreak;

            WebKitApollo::hostFunctions.mapChar = pHostFunctions->mapChar;
            WebKitApollo::hostFunctions.mapString = pHostFunctions->mapString;

            WebKitApollo::hostFunctions.getPDFPluginPathWebString = pHostFunctions->getPDFPluginPathWebString;
            
            WebKitApollo::hostFunctions.getWebSamplerListener = pHostFunctions->getWebSamplerListener;

            WebKitApollo::g_HostFunctions = &WebKitApollo::hostFunctions;

            WebCore::cache()->setCapacities(WebKitApollo::CacheMinDeadCapacity, WebKitApollo::CacheMaxDeadCapacity, WebKitApollo::CacheTotalCapacity);
            WebCore::cache()->setDeadDecodedDataDeletionInterval(60);
            WTF::InstanceCounter::enableLogging("instanceCounts.bin");
        }
        
    }
    
    if (WebKitApollo::g_HostFunctions) {
        pErrorInfo->m_Type = WebErrorNoError;
        result = &WebKitApollo::s_WebKitAPI;
    }
    else
        pErrorInfo->m_Type = WebErrorWrongVersion;
    
    return result;
}

#if PLATFORM(APOLLO_WINDOWS)
BOOL WINAPI DllMain (HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            WebCore::setInstanceHandle((HINSTANCE)hinstDLL);
            return TRUE;

        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return FALSE;
}
#endif


