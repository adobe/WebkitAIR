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
    class WebSamplerHelper : private WebSampler {

    public:

        inline WebSampler* getWebSampler() { return static_cast<WebSampler*>(this); }

    protected:
        WebSamplerHelper();
        virtual ~WebSamplerHelper();

    private:
        static WebSamplerVTable const s_VTable;
        static ImplClass* getThis( WebSampler* const pWebSampler ) { return static_cast<ImplClass*>( pWebSampler ); }
	
	public:

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sStartSampling( struct WebSampler* pWebSampler );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sStopSampling( struct WebSampler* pWebSampler );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sPauseSampling( struct WebSampler* pWebSampler );

        static WEBKIT_APOLLO_PROTO1 WebString* WEBKIT_APOLLO_PROTO2 sGetObjectType( struct WebSampler* pWebSampler, uint64_t identifier );
        static WEBKIT_APOLLO_PROTO1 size_t WEBKIT_APOLLO_PROTO2 sGetObjectSize( struct WebSampler* pWebSampler, uint64_t identifier );
        static WEBKIT_APOLLO_PROTO1 bool WEBKIT_APOLLO_PROTO2 sGetObjectMembers( struct WebSampler* pWebSampler, uint64_t identifier, WebSamplerMembersList* pWebSamplerMembersList );
        static WEBKIT_APOLLO_PROTO1 uint64_t WEBKIT_APOLLO_PROTO2 sGetObjectCallCount( struct WebSampler* pWebSampler, uint64_t identifier );
        
		static WEBKIT_APOLLO_PROTO1 WebString* WEBKIT_APOLLO_PROTO2 sGetObjectFilename( struct WebSampler* pWebSampler, uint64_t identifier );
		static WEBKIT_APOLLO_PROTO1 WebString* WEBKIT_APOLLO_PROTO2 sGetObjectName( struct WebSampler* pWebSampler, uint64_t identifier );
		
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sClearDeadObjects( struct WebSampler* pWebSampler );

    };

    template <class ImplClass>
    WebSamplerVTable const WebSamplerHelper<ImplClass>::s_VTable =
    {
        sizeof( WebSamplerVTable ),
		WebSamplerHelper<ImplClass>::sStartSampling,
        WebSamplerHelper<ImplClass>::sStopSampling,
        WebSamplerHelper<ImplClass>::sPauseSampling,
        WebSamplerHelper<ImplClass>::sGetObjectType,
        WebSamplerHelper<ImplClass>::sGetObjectSize,
        WebSamplerHelper<ImplClass>::sGetObjectMembers,
		WebSamplerHelper<ImplClass>::sGetObjectFilename,
        WebSamplerHelper<ImplClass>::sGetObjectName,
		WebSamplerHelper<ImplClass>::sGetObjectCallCount,
        WebSamplerHelper<ImplClass>::sClearDeadObjects
    };
    
    template <class ImplClass>
    WebSamplerHelper<ImplClass>::WebSamplerHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebSamplerHelper<ImplClass>::~WebSamplerHelper()
    {
        m_pVTable = 0;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sStartSampling( struct WebSampler* pWebSampler )
    {
		getThis(pWebSampler)->startSampling();
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sStopSampling( struct WebSampler* pWebSampler )
    {
		getThis(pWebSampler)->stopSampling();
	}

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
        void WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sPauseSampling( struct WebSampler* pWebSampler )
    {
        getThis(pWebSampler)->pauseSampling();
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    WebString* WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sGetObjectType( struct WebSampler* pWebSampler, uint64_t identifier )
    {
		return getThis(pWebSampler)->getObjectType(identifier);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    size_t WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sGetObjectSize( struct WebSampler* pWebSampler, uint64_t identifier )
    {
		return getThis(pWebSampler)->getObjectSize(identifier);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    bool WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sGetObjectMembers( struct WebSampler* pWebSampler, uint64_t identifier, WebSamplerMembersList* pWebSamplerMembersList )
    {
		return getThis(pWebSampler)->getObjectMembers(identifier, pWebSamplerMembersList);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    uint64_t WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sGetObjectCallCount( struct WebSampler* pWebSampler, uint64_t identifier )
    {
		return getThis(pWebSampler)->getObjectCallCount(identifier);
	}
	
	template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    WebString* WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sGetObjectFilename( struct WebSampler* pWebSampler, uint64_t identifier )
    {
		return getThis(pWebSampler)->getObjectFilename(identifier);
	}
	
	template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    WebString* WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sGetObjectName( struct WebSampler* pWebSampler, uint64_t identifier )
    {
		return getThis(pWebSampler)->getObjectName(identifier);
	}
        
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebSamplerHelper<ImplClass>::sClearDeadObjects( struct WebSampler* pWebSampler )
    {
		getThis(pWebSampler)->clearDeadObjects();
	}

}

#endif //WebSamplerHelper_h
