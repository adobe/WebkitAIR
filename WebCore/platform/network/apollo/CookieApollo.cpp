/*************************************************************************
 *
 * ADOBE CONFIDENTIAL
 * ___________________
 *
 *  Copyright 2007 Adobe Systems Incorporated
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Adobe Systems Incorporated and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to Adobe Systems Incorporated and its
 * suppliers and may be covered by U.S. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Adobe Systems Incorporated.
 **************************************************************************/

#include "config.h"
#include "CookieJar.h"
#include <WebKitApollo/WebKit.h>

#if !PLATFORM(APOLLO_UNIX)
#include "String.h"
#endif

#include "KURL.h"
#include "PlatformString.h"
#include "StringHash.h"
#include <wtf/HashMap.h>

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions;}

namespace WebCore {

void setCookies(Document*, const KURL& url, const String& value)
{
    WebString* const urlWebStr = url.string().webString();
    WebString* const valueWebString = value.webString();

    WebKitApollo::g_HostFunctions->setJavaScriptCookies(urlWebStr, 
                                                        NULL, 
                                                        valueWebString);
    urlWebStr->m_vTable->free(urlWebStr);
    valueWebString->m_vTable->free(valueWebString);
}

String cookies(const Document*, const KURL& url)
{
    WebString* urlWebStr = url.string().webString();
    
    
    WebString* const cookieWebStr = WebKitApollo::g_HostFunctions->getJavaScriptCookies(urlWebStr);
    
    if(!cookieWebStr)
    {
        return String();
    }
    
    return adoptWebString(cookieWebStr);
}

bool cookiesEnabled(const Document*)
{
    //As of now set to true always (similar implementation on Windows)
    return true;
}

}
