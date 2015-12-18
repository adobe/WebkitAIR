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
#include "config.h"
#include "CookieJar.h"
#include <WebKitApollo/WebKit.h>

#include "DeprecatedString.h"
#include "KURL.h"
#include "PlatformString.h"
#include "StringHash.h"
#include <wtf/HashMap.h>

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions;}

namespace WebCore {

#define SIZE_OF_LATIN1_BUFFERS 4096
#define SIZE_OF_COLONSLASHSLASH 3

void setCookies(const KURL& url, const KURL& policyURL, const String& value)
{
    DeprecatedString str = url.url();
    
    DeprecatedString result("Set-Cookie:");

    DeprecatedString val = value.deprecatedString();

    //add everything in the cookie
    result.append(val);

    //Check if it had appended domain, path
    bool noDomainSet = !val.contains(DeprecatedString("domain="));
    bool noPathSet = !val.contains(DeprecatedString("path="));


    if(noDomainSet || noPathSet)
    {
        char pathBuffer[SIZE_OF_LATIN1_BUFFERS];
        char domainBuffer[SIZE_OF_LATIN1_BUFFERS];

        int tl_domainLocationInUrlBegins = str.find("://") + SIZE_OF_COLONSLASHSLASH;
        int tl_domainLocationInUrlEnds = str.find("/",tl_domainLocationInUrlBegins) - 1;
        result.append(';');
        
        if(noDomainSet)
        {
            str.copyLatin1(domainBuffer, tl_domainLocationInUrlBegins, 
                           tl_domainLocationInUrlEnds - tl_domainLocationInUrlBegins + 1);
            domainBuffer[tl_domainLocationInUrlEnds - tl_domainLocationInUrlBegins + 1] = '\0';
            result.append(DeprecatedString(" domain="));
            result.append(DeprecatedString::fromLatin1(domainBuffer));
            result.append(';');
        }

        if(noPathSet)
        {
            int tl_pathLocationInUrlEnds = str.findRev("/");
            str.copyLatin1(pathBuffer, tl_domainLocationInUrlEnds + 1, // 1 because it'll include '/'
                           tl_pathLocationInUrlEnds - tl_domainLocationInUrlEnds ); 
            pathBuffer[tl_pathLocationInUrlEnds = tl_domainLocationInUrlEnds] = '\0';
            result.append(DeprecatedString(" path="));
            result.append(DeprecatedString::fromLatin1(pathBuffer));
            result.append(';');
        }
    }

    if(val.startsWith(DeprecatedString("https://")))
    {
        result.append(DeprecatedString(" secure"));
    }

    //Connect with Runtime.    
    //
    WebKitApollo::g_HostFunctions->setJavaScriptCookie(result.latin1());
}


String cookies(const KURL& url)
{
    DeprecatedString str = url.url();
    char domainBuffer[SIZE_OF_LATIN1_BUFFERS];
    char pathBuffer[SIZE_OF_LATIN1_BUFFERS];
    int tl_domainLocationInUrlBegins = str.find("://") + SIZE_OF_COLONSLASHSLASH;
    int tl_domainLocationInUrlEnds = str.find("/",tl_domainLocationInUrlBegins) - 1;
    int tl_pathLocationInUrlEnds = str.findRev("/");
    
    strncpy(domainBuffer, str.ascii() + tl_domainLocationInUrlBegins, tl_domainLocationInUrlEnds - tl_domainLocationInUrlBegins + 1);
    domainBuffer[tl_domainLocationInUrlEnds - tl_domainLocationInUrlBegins + 1] = '\0';

    strncpy(pathBuffer, str.ascii() + tl_domainLocationInUrlEnds + 1, tl_pathLocationInUrlEnds - tl_domainLocationInUrlEnds);
    pathBuffer[tl_pathLocationInUrlEnds - tl_domainLocationInUrlEnds] = '\0';

    char* cookStr = WebKitApollo::g_HostFunctions->getJavaScriptCookies(domainBuffer,pathBuffer);
    if(!cookStr)
    {
        return String();
    }

    String cookieString(cookStr);
    ::free(cookStr);
    return cookieString;
}

bool cookiesEnabled()
{
    return true;
}

}
