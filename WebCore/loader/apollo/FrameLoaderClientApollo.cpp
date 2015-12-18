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
#include "FrameLoaderClientApollo.h"

#include <CachedResource.h>
#include <Document.h>
#include <FrameLoader.h>
#include <HTMLFrameOwnerElement.h>
#include <HTMLNames.h>
#include <Frame.h>
#include <FrameView.h>

namespace WebCore {

FrameLoaderClientApollo* FrameLoaderClientApollo::clientApollo(const FrameLoader* const frameLoader)
{
    ASSERT(frameLoader);
    FrameLoaderClient* const frameLoaderClient = frameLoader->client();
    ASSERT(frameLoaderClient);
    return clientApollo(frameLoaderClient);
}

FrameLoaderClientApollo* FrameLoaderClientApollo::clientApollo(const Frame* const frame)
{
    ASSERT(frame);
    FrameLoader* const frameLoader = frame->loader();
    ASSERT(frameLoader);
    return clientApollo(frameLoader);
}

FrameLoaderClientApollo* FrameLoaderClientApollo::clientApollo(const FrameView* const frameView)
{
    ASSERT(frameView);
    Frame* const frame = frameView->frame();
    ASSERT(frame);
    return clientApollo(frame);
}

FrameLoaderClientApollo::FrameLoaderClientApollo()
{
}

FrameLoaderClientApollo::~FrameLoaderClientApollo()
{
}

namespace {
static const char appResource[] = "app";
static const char appStorage[] = "app-storage";
static const char file[] = "file";

struct SchemeLoadInfo
{
    const char* const scheme;
    const char* const * const schemesThatCanLoadScheme; 
};

//app can only be loaded by app
static const char* const schemesThatCanAccessAppResource[] = { appResource, 0 };

//app-storage can be loaded by app-resourse or app-storage
static const char* const schemesThatCanAccessAppStorage[] = { appResource, appStorage, 0 };

//file can be loaded by app or file.
static const char* const schemesThatCanAccessFile[] = { appResource, file, 0 };

static SchemeLoadInfo const schemeLoadInfos [] = {
    { appResource, schemesThatCanAccessAppResource },  
    { appStorage, schemesThatCanAccessAppStorage }, 
    { file, schemesThatCanAccessFile } 
};

static const size_t numSchemeLoadInfos = sizeof(schemeLoadInfos) / sizeof(SchemeLoadInfo);

class CanLoadImpl {
public:
    
    CanLoadImpl() {
        for (unsigned i = 0; i < numSchemeLoadInfos; ++i)
        {
            AllowedReferrersSet* pAllowedReferrersSet = new AllowedReferrersSet();
            const char* const* currAllowedReferrer = schemeLoadInfos[i].schemesThatCanLoadScheme;
            while (*currAllowedReferrer) {
                pAllowedReferrersSet->add(String(*currAllowedReferrer));
                ++currAllowedReferrer;
            }
            localSchemes.add(schemeLoadInfos[i].scheme, pAllowedReferrersSet);
        }
    }
    
    ~CanLoadImpl() {
        deleteAllValues(localSchemes);
    }
    
    static bool canLoad(const KURL& url, const KURL& referrer)
    {
        static CanLoadImpl instance;
        String const urlScheme(url.protocol().lower());
        
		//	Anything can load emtpy URL, and empty URL cannot load anything else.
		if (url.isEmpty())
			return true;
		else if (referrer.isEmpty())
			return false;

        // The map below can't deal with empty schemes, and these result from invalid URLs.
        if (urlScheme.isEmpty())
            return false;

        SchemeToSchemesMap::const_iterator pSchemeInfo(instance.localSchemes.find(urlScheme));
        SchemeToSchemesMap::const_iterator const pSchemeInfoEnd = instance.localSchemes.end();
        if (pSchemeInfo == pSchemeInfoEnd) {
            // The scheme of the url that the referrer wants to load was not in our map
            // which means it is a scheme that can be accessed
            // from any other scheme.
            ASSERT(!SecurityOrigin::shouldTreatURLAsLocal(url.string()));
            return true;
        }
        
        ASSERT(pSchemeInfo != pSchemeInfoEnd);
        // The scheme of the url the referrer wants to load was in our map
        // which means only schemes in the HashSet can load the
        // the url in question.
        
        String const referrerScheme(referrer.protocol().lower());
        ASSERT(pSchemeInfo->second);
        bool const result = pSchemeInfo->second->contains(referrerScheme);
        return result;
    }

private:
    typedef WTF::HashSet<String, CaseFoldingHash > AllowedReferrersSet;
    typedef WTF::HashMap<String, AllowedReferrersSet*, CaseFoldingHash > SchemeToSchemesMap;
    SchemeToSchemesMap localSchemes;
};

}
	
bool FrameLoaderClientApollo::canLoad(const KURL& url, const String& referrer, const Document* doc)
{
	if(doc)
		return FrameLoaderClientApollo::canLoad(url, doc);
	
	return FrameLoaderClientApollo::canLoad(url, referrer);
}	

bool FrameLoaderClientApollo::canLoad(const KURL& url, const String& referrer)
{
    KURL const referrerURL(ParsedURLString, referrer);
    return CanLoadImpl::canLoad(url, referrerURL);
}

static bool documentHasApplicationRestrictions(const Document* doc)
{
    ASSERT(doc);

    const Frame* pFrame = doc->frame();

    if (!pFrame)
        return false;
    
    ASSERT(pFrame->loader());
    ASSERT(pFrame->loader()->client());
    return pFrame->loader()->client()->hasApplicationRestrictions();
}

bool FrameLoaderClientApollo::canLoad(const KURL& url, const Document* doc)
{
    ASSERT(doc);

    if (documentHasApplicationRestrictions(doc))
        return true;

    KURL const referrerURL(doc->url());
    return CanLoadImpl::canLoad(url, referrerURL);
}

/*	
bool FrameLoaderClientApollo::canLoad(const CachedResource& resource, const Document* doc)
{
    ASSERT(doc);

    if (documentHasApplicationRestrictions(doc))
        return true;

    KURL const referrerURL(doc->url());
    KURL const url(resource.url());
    return CanLoadImpl::canLoad(url, referrerURL);
}
*/

namespace {
    static bool isUrlPrefixOfUrl(const char* prefix, const char* other, int *rest)
    {
        // ad-hoc URL parsing is evil, but this is what makes sense here
        int pos = 0, otherpos = 0;
        *rest = -1;

        UChar ch;
        // find colon
        while ((ch=prefix[pos]) != 0 && ch != ':') {
            // compare case-insensitively
            if (WTF::toASCIILower(ch) != WTF::toASCIILower(other[otherpos++]))
                return false;
            pos++;
        }
        // pos now indexes ':' or eos

        if (ch != ':')    // There needs to be a colon
            return false;
        if (other[otherpos] != ':')
            return false;

        if (prefix[pos+1] != '/') {
            // just a protocol and some data that does not begin with a slash; advance pos to the data or eos.
            otherpos++;
            pos++;
        }
        else if (prefix[pos+2] == '/') {
            // if next part has two slashes...
            if (other[otherpos+1] != '/' || other[otherpos+2] != '/')
                return false;
            otherpos += 3;
            pos += 3;

            // look ahead to see if there's an @ (which ends user:pw)
            int otherpos2 = otherpos;
            while ((ch=other[otherpos2]) != 0 && ch != '/' ) {
                otherpos2++;
                if (ch == '@') {
                    // if we find an @, skip this part
                    otherpos = otherpos2;
                    break;
                }
            }

            // look for next slash
            while ((ch=prefix[pos]) != 0 && ch != '/' ) {
                // compare case-insensitively
                if (WTF::toASCIILower(ch) != WTF::toASCIILower(other[otherpos++]))
                    return false;
                pos++;
            }
            // pos now indexes the slash or eos
        }

        // OK, now compare the rest, considering case
        while ((ch=prefix[pos]) != 0 && ch == other[otherpos]) {
            otherpos++;
            pos++;
        }

        // if part of the prefix is unmatched, then nope
        if (ch != 0)
            return false;

        // if the last character matched is a slash or
        // if the unmatched part of other ends a path component, then good
        if (prefix[pos-1] == '/' || (ch = other[otherpos]) == 0 || ch == '/' || ch == '#' || ch == '?')
        {
            *rest = otherpos;
            return true;
        }

        // otherwise nope.
        return false;
    }
}

/*static*/
bool FrameLoaderClientApollo::mapFrameUrl(Frame* frame, const KURL& url, KURL* mappedUrl)
{
    String urlString(url.string());

    while (frame) {
        HTMLFrameOwnerElement* frameElement = frame->ownerElement();
        // If there's no frame owner, then consider the opener hierarchy
        while (!frameElement) {
            WebCore::Frame* const frameOpener = frame->loader()->opener();
            if ((!frameOpener) || (frameOpener == frame))
                break;
            frame = frameOpener;
            frameElement = frame->ownerElement();
        }
        if (!frameElement)
            break;

        String fromURL = frameElement->getAttribute(HTMLNames::sandboxrootAttr);
        String toURL = frameElement->getAttribute(HTMLNames::documentrootAttr);
        if (!fromURL.isEmpty() && !toURL.isEmpty()) {
            if (frameElement->document()->url().string().startsWith("app:/")) {
                int rest = -1;
                if (isUrlPrefixOfUrl(fromURL.utf8().data(), urlString.utf8().data(), &rest)) {
                    // Ensure that network content does not get treated
                    // as local or privileged content.
                    // Don't require a / on the toURL; it'll be added if necessary.
                    if ((fromURL.find("http:/") == 0 || fromURL.find("https:/") == 0 || fromURL.find("file:/") == 0)
                        && (toURL.find("app:") == 0 || toURL.find("app-storage:") == 0 || toURL.find("file:") == 0)) {
                        String newURL = toURL;
                        // may need to insert a slash, or get rid of one
                        if (newURL[newURL.length()-1] == '/') {
                            if (urlString[rest] == '/')  // already have a slash, skip this one
                                rest++;
                        }
                        else if (urlString[rest] != '/'
                             && urlString[rest] != '#'
                             && urlString[rest] != '?'
                             && urlString[rest] != 0) {
                            newURL.append('/');
                        }
                        newURL.append(urlString.substring(rest));

                        // Most header fields not interesting since result is always a local resource.
                        *mappedUrl = KURL(ParsedURLString, newURL);
                        return true;
                    }
                }
            }
        }
        // Keep looking up the frame hierarchy
        frame = frameElement->document()->frame();
    }

    // use original
    *mappedUrl = url;
    return false;
}

/*static*/
bool FrameLoaderClientApollo::unmapFrameUrl(Frame* frame, const KURL& url, KURL* unmappedUrl)
{
	String urlString(url.string());

	while (frame) {
		HTMLFrameOwnerElement* frameElement = frame->ownerElement();
		// If there's no frame owner, then consider the opener hierarchy
		while (!frameElement) {
			WebCore::Frame* const frameOpener = frame->loader()->opener();
			if ((!frameOpener) || (frameOpener == frame))
				break;
			frame = frameOpener;
			frameElement = frame->ownerElement();
		}
		if (!frameElement)
			break;

		String fromURL = frameElement->getAttribute(HTMLNames::sandboxrootAttr);
		String toURL = frameElement->getAttribute(HTMLNames::documentrootAttr);
		if (!fromURL.isEmpty() && !toURL.isEmpty()) {
			if (frameElement->document()->url().string().startsWith("app:/")) {
				int rest = -1;
				if (isUrlPrefixOfUrl(toURL.utf8().data(), urlString.utf8().data(), &rest)) {
					// Ensure that network content does not get treated
					// as local or privileged content.
					// Don't require a / on the toURL; it'll be added if necessary.
					if ((fromURL.find("http:/") == 0 || fromURL.find("https:/") == 0 || fromURL.find("file:/") == 0)
						&& (toURL.find("app:") == 0 || toURL.find("app-storage:") == 0 || toURL.find("file:") == 0)) {
							String newURL = fromURL;
							// may need to insert a slash, or get rid of one
							if (newURL[newURL.length()-1] == '/') {
								if (urlString[rest] == '/')  // already have a slash, skip this one
									rest++;
							}
							else if (urlString[rest] != '/'
								&& urlString[rest] != '#'
								&& urlString[rest] != '?'
								&& urlString[rest] != 0) {
									newURL.append('/');
							}
							newURL.append(urlString.substring(rest));

							// Most header fields not interesting since result is always a local resource.
							*unmappedUrl = KURL(ParsedURLString, newURL);
							return true;
					}
				}
			}
		}
		// Keep looking up the frame hierarchy
		frame = frameElement->document()->frame();
	}

	// use original
	*unmappedUrl = url;
	return false;
}


/* static */
bool FrameLoaderClientApollo::frameHasAllowCrossDomainXHRAttr(Frame* frame)
{
    // This routine exists only because the HTMLNames header file is not available in the WebKit layer apparently
    WebCore::HTMLFrameOwnerElement* frameElement = frame->ownerElement();
    if (frameElement) {
        // Really should treat anything non-null as true, but people make too many mistakes with that.
        const WebCore::AtomicString &allowAttr = frameElement->getAttribute(HTMLNames::allowcrossdomainxhrAttr);
        if (allowAttr.isNull()) // null is false
            return false;
        if (allowAttr == "0") // 0 is false
            return false;
        if (allowAttr.startsWith("f", false /*caseSensitive*/)) // f* is false
            return false;
        if (allowAttr.startsWith("n", false /*caseSensitive*/)) // n* is false
            return false;
        // OK, this will do.
        return true;
    }
    return false;
}

}



