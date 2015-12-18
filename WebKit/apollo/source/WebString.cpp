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
 
 #include <wtf/Platform.h>
#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif
 
#include "PlatformString.h"
#include <wtf/text/CString.h>

#include <WebKitApollo/helpers/WebStringHelper.h>

namespace WebKitApollo {
class WebStringImpl : private WebKitApollo::WebStringHelper<WebStringImpl> 
{
    friend class WebKitApollo::WebStringHelper<WebStringImpl>;
public:
    static ::WebString* create(const WebCore::String& s) throw();
private:
    WebStringImpl(const WebCore::String& s) throw();
    
    void free() throw();
    const uint16_t* getUTF16(unsigned long* const numUTF16CodeUnits) const throw();
    unsigned long getMaxUTF8Bytes() const throw();
    void getUTF8(char* const dest)const throw();
    
    WebCore::String m_str;
    
    WebStringImpl(const WebStringImpl&);
    WebStringImpl& operator=(const WebStringImpl&);

};

inline WebString* WebStringImpl::create(const WebCore::String& s) throw()
{
    WebStringImpl* impl = new WebStringImpl(s);
    return impl->getWebString();
}

WebStringImpl::WebStringImpl(const WebCore::String& s ) throw()
    : m_str(s)
{
}

void WebStringImpl::free() throw()
{
    delete this;
}

const uint16_t* WebStringImpl::getUTF16(unsigned long* const numUTF16CodeUnits) const throw()
{
    ASSERT(numUTF16CodeUnits);
    *numUTF16CodeUnits = m_str.length();
    return reinterpret_cast<const uint16_t*>(m_str.characters());
}

unsigned long WebStringImpl::getMaxUTF8Bytes() const throw()
{
    return m_str.length() * 6;
}

void WebStringImpl::getUTF8(char* const dest) const throw()
{
    ASSERT(dest);
    WTF::CString const utf8Str(m_str.utf8());
    ASSERT(getMaxUTF8Bytes() >= utf8Str.length());
    memcpy(dest, utf8Str.data(), utf8Str.length());
}
}

namespace WebCore
{
    String adoptWebString(WebString* const webString)
    {
        ASSERT(webString);
        ASSERT(webString->m_vTable);
        ASSERT(webString->m_vTable->getUTF16);
        unsigned long numUTF16CodeUnits = 0;
        const uint16_t* const utf16CodeUnits = webString->m_vTable->getUTF16(webString, &numUTF16CodeUnits);
        String result(reinterpret_cast<const UChar*>(utf16CodeUnits), numUTF16CodeUnits);
        webString->m_vTable->free(webString);
        return result;
    }

    WebString* String::webString() const
    {
        return WebKitApollo::WebStringImpl::create(*this);
    }
}
