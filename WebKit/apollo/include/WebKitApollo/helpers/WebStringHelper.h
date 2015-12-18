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
#ifndef WebStringHelper_h
#define WebStringHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo
{

	template <class ImplClass>
    class WebStringHelper : private ::WebString {
    public:
		inline WebString* getWebString() { return this; }
    protected:
		WebStringHelper();
		virtual ~WebStringHelper();
	private:
		static WebStringVTable const s_VTable;
		static ImplClass* getThis( WebString* webString ) { return static_cast<ImplClass*>(webString); }
        static const ImplClass* getThis( const WebString* webString ) { return static_cast<const ImplClass*>(webString); }

		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sFree(struct WebString*);
        static WEBKIT_APOLLO_PROTO1 const uint16_t* WEBKIT_APOLLO_PROTO2 sGetUTF16(const struct WebString*, unsigned long* numUTF16CodeUnits);
        static WEBKIT_APOLLO_PROTO1 unsigned long WEBKIT_APOLLO_PROTO2 sGetMaxUTF8Bytes(const struct WebString*);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetUTF8(const struct WebString*, char* dest);
    };

    template <class ImplClass>
	WebStringVTable const WebStringHelper<ImplClass>::s_VTable = {
		sizeof( WebStringVTable ),
		WebStringHelper<ImplClass>::sFree,
		WebStringHelper<ImplClass>::sGetUTF16,
		WebStringHelper<ImplClass>::sGetMaxUTF8Bytes,
		WebStringHelper<ImplClass>::sGetUTF8
	};

    template<class ImplClass>
    WebStringHelper<ImplClass>::WebStringHelper()
    {
        m_vTable = &s_VTable;
    }

    template <class ImplClass>
	WebStringHelper<ImplClass>::~WebStringHelper()
	{
        m_vTable = 0;
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebStringHelper<ImplClass>::sFree(struct WebString* webString)
	{
		ImplClass* const pThis = getThis(webString);
		pThis->free();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	const uint16_t* WEBKIT_APOLLO_PROTO2 WebStringHelper<ImplClass>::sGetUTF16(const struct WebString* webString, unsigned long* numUTF16CodeUnits)
	{
		const ImplClass* const pThis = getThis(webString);
		return pThis->getUTF16(numUTF16CodeUnits);
	}

    template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	unsigned long WEBKIT_APOLLO_PROTO2 WebStringHelper<ImplClass>::sGetMaxUTF8Bytes(const struct WebString* webString)
	{
		const ImplClass* const pThis = getThis(webString);
		return pThis->getMaxUTF8Bytes();
	}

    template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebStringHelper<ImplClass>::sGetUTF8(const struct WebString*webString, char* dest)
	{
		const ImplClass* const pThis = getThis(webString);
		pThis->getUTF8(dest);
	}
}

#endif
