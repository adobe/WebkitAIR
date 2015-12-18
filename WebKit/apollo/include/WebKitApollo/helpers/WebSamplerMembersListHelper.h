/*
 * Copyright (C) 2009 Adobe Systems Incorporated.  All rights reserved.
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

#ifndef WebSamplerHelper_h
#define WebSamplerHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo
{
    template <class ImplClass>
    class WebSamplerMembersListHelper : private WebSamplerMembersList {

    public:
        inline WebSamplerMembersList* getWebSamplerMembersList() { return static_cast<WebSamplerMembersList*>(this); }

    protected:
        WebSamplerMembersListHelper();
        virtual ~WebSamplerMembersListHelper();

    private:
        static WebSamplerMembersListVTable const s_VTable;
        static ImplClass* getThis( WebSamplerMembersList* const pWebSamplerMembersList ) { return static_cast<ImplClass*>( pWebSamplerMembersList ); }
	
	public:

		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sInit( struct WebSamplerMembersList* pWebSamplerMembersList, int32_t size );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAppend( struct WebSamplerMembersList* pWebSamplerMembersList, WebString* pName, uint64_t identifier );

    };

    template <class ImplClass>
    WebSamplerMembersListVTable const WebSamplerMembersListHelper<ImplClass>::s_VTable =
    {
        sizeof( WebSamplerMembersListVTable ),
		WebSamplerMembersListHelper<ImplClass>::sInit,
		WebSamplerMembersListHelper<ImplClass>::sAppend
    };
    
    template <class ImplClass>
    WebSamplerMembersListHelper<ImplClass>::WebSamplerMembersListHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebSamplerMembersListHelper<ImplClass>::~WebSamplerMembersListHelper()
    {
        m_pVTable = 0;
        // remove the virtual pointer
        (* reinterpret_cast<int*>( this )) = NULL; 
    }
	
	template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebSamplerMembersListHelper<ImplClass>::sInit( struct WebSamplerMembersList* pWebSamplerMembersList, int32_t size )
    {
		getThis(pWebSamplerMembersList)->init(size);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebSamplerMembersListHelper<ImplClass>::sAppend( struct WebSamplerMembersList* pWebSamplerMembersList, WebString* pName, uint64_t identifier )
    {
		getThis(pWebSamplerMembersList)->append(pName, identifier);
	}

}

#endif //WebSamplerHelper_h
