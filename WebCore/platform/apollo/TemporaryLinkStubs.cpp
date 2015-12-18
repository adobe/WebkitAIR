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
#include <BitmapImage.h>
#include <Editor.h>
#include <DocumentLoader.h>
#include <EventHandler.h>
#include <FileChooser.h>
#include <Icon.h>
#include <MIMETypeRegistry.h>
#include <PageCache.h>
#include <DocumentFragment.h>
#include <PluginInfoStore.h>
#include <PopupMenu.h>
#include <SearchPopupMenu.h>
#include <RenderTheme.h>
#include <Screen.h>

#include <Document.h>
#include <Frame.h>
#include <FrameLoader.h>
#include <FrameLoaderClientApollo.h>
#include <StringTruncator.h>
#include "NotImplemented.h"
#include <ScrollView.h>
#include <PlatformWheelEvent.h>

#if PLATFORM(APOLLO_WINDOWS)
#include <Urlmon.h>
#elif PLATFORM(APOLLO_DARWIN)
#include "mac/WebCoreSystemInterface.h"
#elif PLATFORM(APOLLO_UNIX)
#include <wtf/text/CString.h>
#include <WebKitApollo/WebKit.h>
namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions;}
#endif

#include "Pasteboard.h"
#include <Page.h>

#include <WebKitApollo/WebKit.h>
#include "FileSystem.h"
#include "FrameLoaderClientApollo.h"

namespace WebCore {

//#if PLATFORM(APOLLO_WINDOWS)
//HINSTANCE Page::s_instanceHandle = NULL;
//#endif


// This code should be in its own file.
//
PlatformWheelEvent::PlatformWheelEvent( IntPoint position
                                         , IntPoint globalPosition
                                         , float deltaX
                                         , float deltaY
                                         , bool isAccepted
                                         , bool shiftKey
                                         , bool ctrlKey
                                         , bool altKey
                                         , bool metaKey
                                         , bool /*isContinuous*/
                                      )
    : m_position(position)
    , m_globalPosition(globalPosition)
    , m_deltaX(deltaX)
    , m_deltaY(deltaY)
    , m_isAccepted(isAccepted)
    , m_shiftKey(shiftKey)
    , m_ctrlKey(ctrlKey)
    , m_altKey(altKey)
    , m_metaKey(metaKey)
{
    m_granularity = ScrollByPixelWheelEvent;
	//
	m_wheelTicksX = m_deltaX;
	m_wheelTicksY = m_deltaY;
    // see WebKit bug 24407: Windows wheel scrolls by too little
    m_deltaX *= static_cast<float>(Scrollbar::pixelsPerLineStep());
    m_deltaY *= static_cast<float>(Scrollbar::pixelsPerLineStep());
}

bool EventHandler::tabsToAllControls(KeyboardEvent*) const
{
    return true;
}

String FileChooser::basenameForWidth(Font const& font, int width, Document* doc) const
{
    if (width <= 0)
        return String();

    ASSERT(doc);

    String strToTruncate;
	if(m_filenames.size() == 1) {
        
        ASSERT(doc);
        ASSERT(doc->frame());
        
        bool hasApplicationPrivileges = false;
        bool versionIsAtLeast20 = false;
        
        if (doc && doc->frame()) {
            hasApplicationPrivileges = doc->frame()->loader()->client()->hasApplicationPrivileges();
            WebHost* webHost = FrameLoaderClientApollo::clientApollo(doc->frame())->webHost();
            versionIsAtLeast20 = webHost->m_pVTable->isPlatformAPIVersion(webHost, WEBKIT_APOLLO_AIR_2_0);
        }        
        
        // if we are in application sandbox or remote sandbox with 1.5.x return full path
        // otherwise return file name
        if (hasApplicationPrivileges || !versionIsAtLeast20)
		strToTruncate = m_filenames[0];
        else
            strToTruncate = pathGetFileName(m_filenames[0]);
    }
		

    return StringTruncator::centerTruncate(strToTruncate, width, font, false);	
}


void Icon::paint(GraphicsContext*, IntRect const&)
{
}


String MIMETypeRegistry::getMIMETypeForExtension(String const& ext)
{
    String extTmp(ext.lower());
    int pos = extTmp.find('#');
    if(pos <= 0) {
        pos = extTmp.find('?');
    }

    if (pos > 0) {
        extTmp.truncate(pos);
    }

    if (extTmp == "pdf") {
        return "application/pdf";
    }
    else if (extTmp == "fdf") {
        return "application/vnd.fdf";
    }
    else if (extTmp == "xfdf") {
        return "application/vnd.adobe.xfdf";
    }
    else if (extTmp == "xdp") {
        return "application/vnd.adobe.xdp+xml";
    }
    else if (extTmp == "xfd") {
        return "application/vnd.adobe.xfd+xml";
    }
    else if (extTmp == "mars") {
        return "application/vnd.adobe.x-mars";
    }
    else if(extTmp == "htm" || ext == "html") {
        return "text/html";
    }
    else if(extTmp == "txt") {
        return "text/plain";
    }
    else if(extTmp == "css") {
        return "text/css";
    }
    else if(extTmp == "gif") {
        return "image/gif";
    }
    else if(extTmp == "jpeg") {
        return "image/jpeg";
    }
    else if(extTmp == "jpg") {
        return "image/jpeg";
    }
    else if(extTmp == "png") {
        return "image/png";
    }
    else if (extTmp == "bmp") {
        return "image/bmp";
    }

#if PLATFORM(APOLLO_WINDOWS)
    if(extTmp == "xbm") {
        return "image/x-xbitmap";
    }

    extTmp = String("foo.") + extTmp;

    LPWSTR mime = 0;
    if(FindMimeFromData(NULL, (LPCWSTR)extTmp.charactersWithNullTermination(), 0, 0, 0, 0, &mime, 0) == NOERROR)
    {
        String ret(mime);
        CoTaskMemFree(mime);
        return ret;
    }
#elif PLATFORM(APOLLO_DARWIN)
    //NSString* nsExt = [[NSString alloc] initWthCharacters: ext.characters() length: ext.length() ];
    CFStringRef cfsExt = ext.createCFString();
    NSString* cfsMIME =  wkGetMIMETypeForExtension((NSString*)cfsExt);
    String ret((CFStringRef)cfsMIME);
    CFRelease(cfsExt);
    
    return ret;
#elif PLATFORM(APOLLO_UNIX)
    CString tempString = extTmp.utf8();
    const char *extension = tempString.data();
    char *mimetype = NULL;
    if(extension)
    {
        mimetype = WebKitApollo::g_HostFunctions->getMIMETypeFromExtension(extension);
    }

    String ret(mimetype ? mimetype : "");
    free(mimetype);
    return ret;
#else
#error
#endif

    return "";
}

SearchPopupMenu::SearchPopupMenu(PopupMenuClient* pClient)
    : PopupMenu(pClient)
{
}

bool SearchPopupMenu::enabled()
{
    return true;
}

void SearchPopupMenu::loadRecentSearches(AtomicString const&, WTF::Vector<String, 0ul>&)
{
}

void SearchPopupMenu::saveRecentSearches(AtomicString const&, WTF::Vector<String, 0ul> const&)
{
}

int findNextSentenceFromIndex(UChar const*, int, int, bool);
int findNextSentenceFromIndex(UChar const*, int, int, bool)
{
    return 0;
}

void findSentenceBoundary(UChar const*, int, int, int*, int*);
void findSentenceBoundary(UChar const*, int, int, int*, int*)
{
}

// stolen from StringImpl.cpp (sigh, highly inadequate)
static inline bool isSpace(UChar c)
{
    // Use isspace() for basic Latin-1.
    // This will include newlines, which aren't included in unicode DirWS.
    return c <= 0x7F ? WTF::isASCIISpace(c) : WTF::Unicode::direction(c) == WTF::Unicode::WhiteSpaceNeutral;
}    

#if !PLATFORM(CG)
Color focusRingColor()
{
    return 0xFF0000FF;
}
#endif

#if PLATFORM(CG)
String getMIMETypeForUTI(const String & uti);
String getMIMETypeForUTI(const String & uti)
{
    CFStringRef utiref = uti.createCFString();
    CFStringRef mime = UTTypeCopyPreferredTagWithClass(utiref, kUTTagClassMIMEType);
    String mimeType = mime;
    if (mime)
        CFRelease(mime);
    CFRelease(utiref);
    return mimeType;
}
#endif

#if !PLATFORM(CG)
void setFocusRingColorChangeFunction(void (*)())
{
}
#endif



String signedPublicKeyAndChallengeString(unsigned int, String const&, KURL const&);
String signedPublicKeyAndChallengeString(unsigned int, String const&, KURL const&)
{
    return String();
}

void getSupportedKeySizes(WTF::Vector<String>& supportedKeySizes);
void getSupportedKeySizes(WTF::Vector<String>& supportedKeySizes)
{
   supportedKeySizes.clear();
}

void systemBeep();
void systemBeep()
{
}

Icon::~Icon()
{
}
	
void prefetchDNS(const String& /*hostname*/);
void prefetchDNS(const String& /*hostname*/)
{
	notImplemented();
}

PassRefPtr<Icon> Icon::createIconForFiles(const Vector<String>&)
{
    notImplemented();
    return NULL;
}

}
