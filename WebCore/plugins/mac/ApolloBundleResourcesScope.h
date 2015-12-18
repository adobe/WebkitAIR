/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
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
#ifndef APOLLOBUNDLERESOURCESSCOPE_H
#define APOLLOBUNDLERESOURCESSCOPE_H

using std::auto_ptr;

#include <wtf/RetainPtr.h>

namespace WebCore {
    
    namespace {
        
        std::auto_ptr<char> cfStringToUTF8(const WTF::RetainPtr<CFStringRef>& cfString)
        {
            size_t const numCharacters = CFStringGetLength(cfString.get());
            size_t const numUTF8Bytes = CFStringGetMaximumSizeForEncoding(numCharacters, kCFStringEncodingUTF8) + 1; // +1 for null terminator.
            std::auto_ptr<char> result;
            if (numUTF8Bytes > 0) {
                result = std::auto_ptr<char>(new char[numUTF8Bytes]);
                ASSERT(result.get());
                Boolean const getCStrRet = CFStringGetCString(cfString.get(), result.get(), numUTF8Bytes, kCFStringEncodingUTF8);
                ASSERT(getCStrRet); (void)getCStrRet;
            }
            return result;
        }
        
        CFStringEncoding encodingForResource(Handle const resource)
        {
            short resRef = HomeResFile(resource);
            if (ResError() != noErr)
                return kCFStringEncodingMacRoman;
            
            // Get the FSRef for the current resource file
            FSRef fref;
            OSStatus error = FSGetForkCBInfo(resRef, 0, 0, 0, 0, &fref, 0);
            if (error != noErr)
                return kCFStringEncodingMacRoman;
            
            WTF::RetainPtr<CFURLRef> const URL(WTF::AdoptCF, CFURLCreateFromFSRef(kCFAllocatorDefault, &fref));
            if (!URL)
                return kCFStringEncodingMacRoman;
            
            WTF::RetainPtr<CFURLRef> const lprojURL(CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, URL.get()));
            if (!lprojURL)
                return kCFStringEncodingMacRoman;
            
            // Get the lproj directory name
            WTF::RetainPtr<CFStringRef> const extension(WTF::AdoptCF, CFURLCopyPathExtension(lprojURL.get()));
            if ((!extension) || (CFStringCompare(extension.get(), CFSTR("lproj"), kCFCompareCaseInsensitive) != kCFCompareEqualTo))
                return kCFStringEncodingMacRoman;
            
            WTF::RetainPtr<CFURLRef> const lprojWithoutExtension(WTF::AdoptCF, CFURLCreateCopyDeletingPathExtension(kCFAllocatorDefault, lprojURL.get()));
            if (!lprojWithoutExtension)
                return kCFStringEncodingMacRoman;
            
            WTF::RetainPtr<CFStringRef> const lprojName(WTF::AdoptCF, CFURLCopyLastPathComponent(lprojWithoutExtension.get()));
            if (!lprojName)
                return kCFStringEncodingMacRoman;
            
            WTF::RetainPtr<CFStringRef> const locale(WTF::AdoptCF, CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, lprojName.get()));
            if (!locale)
                return kCFStringEncodingMacRoman;
            
            std::auto_ptr<char> const localUTF8(cfStringToUTF8(locale));
            
            LangCode lang;
            RegionCode region;
            error = LocaleStringToLangAndRegionCodes(localUTF8.get(), &lang, &region);
            if (error != noErr)
                return kCFStringEncodingMacRoman;
            
            TextEncoding encoding;
            error = UpgradeScriptInfoToTextEncoding(kTextScriptDontCare, lang, region, 0, &encoding);
            if (error != noErr)
                return kCFStringEncodingMacRoman;
            
            return encoding;
        }
        
        WTF::Vector<String> readResourceStringList(SInt16 stringListId)
        {
            WTF::Vector<String> result;
            Handle const stringListHandle = Get1Resource('STR#', stringListId);
            if (stringListHandle) {
                CFStringEncoding const encodingOfStringResource = encodingForResource(stringListHandle);
                const unsigned char* const stringListStart = reinterpret_cast<const unsigned char*>(*stringListHandle);
                SInt16 listLen = *reinterpret_cast<const SInt16*>(stringListStart);
                if (listLen > 0) {
                    result.reserveCapacity(listLen);
                    const unsigned char* currStringListPos = stringListStart + sizeof(SInt16);
                    for ( unsigned char i = 0 ; i < listLen ; ++i ) {
                        WTF::RetainPtr<CFStringRef> const currString(WTF::AdoptCF, CFStringCreateWithPascalString(kCFAllocatorDefault, currStringListPos, encodingOfStringResource));
                        result.append(String(currString.get()));
                        unsigned char const currStringByteLength = *currStringListPos;
                        currStringListPos += currStringByteLength + 1; //+1 to skip over length byte.
                    }
                }
            }
            return result;
        }
        
        static const SInt16 PluginNameOrDescriptionStringNumber = 126;
        static const SInt16 MIMEDescriptionStringNumber = 127;
        static const SInt16 MIMEListStringStringNumber = 128;
        
        class BundleResourcesScope
            {
            public:
                BundleResourcesScope(const WTF::RetainPtr<CFBundleRef>& bundle)
                : m_savedResources(-1)
                , m_module(bundle)
                , m_resources(-1)
                {
                    m_savedResources = CurResFile();
                    if (ResError() == noErr) {
                        m_resources = CFBundleOpenBundleResourceMap(m_module.get());
                        if (m_resources != -1) {
                            UseResFile(m_resources);
                            if (ResError() == noErr) {
                                ASSERT(isOk());
                            }
                            else {
                                UseResFile(m_savedResources);
                                CFBundleCloseBundleResourceMap(m_module.get(), m_resources);
                                m_resources = -1;
                            }
                        }
                        else {
                            ASSERT(!isOk());
                        }
                    }
                    else {
                        m_savedResources = -1;
                        ASSERT(!isOk());
                    }
                }
                
                ~BundleResourcesScope()
                {
                    if (m_savedResources != -1)
                        UseResFile(m_savedResources);
                    if (m_resources != -1)
                        CFBundleCloseBundleResourceMap(m_module.get(), m_resources);
                }
                
                bool isOk() { return m_resources != -1; }
                
            private:
                BundleResourcesScope(const BundleResourcesScope&);
                BundleResourcesScope& operator=(const BundleResourcesScope&);
                
                SInt16 m_savedResources;
                WTF::RetainPtr<CFBundleRef> const m_module;
                SInt16 m_resources;
            };
    }
    
    
}

#endif
