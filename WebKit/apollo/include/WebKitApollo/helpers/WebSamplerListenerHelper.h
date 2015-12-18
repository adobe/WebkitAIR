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

#ifndef WebSamplerListenerHelper_h
#define WebSamplerListenerHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo
{
    template <class ImplClass>
    class WebSamplerListenerHelper : private WebSamplerListener {

    public:

        inline WebSamplerListener* getWebSamplerListener() { return this; }

    protected:
        WebSamplerListenerHelper();
        virtual ~WebSamplerListenerHelper();

    private:
        static WebSamplerListenerVTable const s_VTable;
        static ImplClass* getThis( WebSamplerListener* const pWebSamplerListener ) { PLAYERASSERT(pWebSamplerListener); return static_cast<ImplClass*>( pWebSamplerListener ); }
	
	public:

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDidEnterFunction( struct WebSamplerListener* pWebSamplerListener, 
                        uint64_t identifier, int32_t lineno);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDidExitFunction( struct WebSamplerListener* pWebSamplerListener, uint64_t identifier );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAtStatement( struct WebSamplerListener* pWebSamplerListener, int32_t lineno );

        static WEBKIT_APOLLO_PROTO1 uint64_t WEBKIT_APOLLO_PROTO2 sDidAllocateObject( struct WebSamplerListener* pWebSamplerListener, const void* ptr, size_t size );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDidDeallocateObject( struct WebSamplerListener* pWebSamplerListener, const void* ptr, size_t size );
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetWebSamplerRef( struct WebSamplerListener* pWebSamplerListener, WebSampler **pWebSamplerRef );

    };

    template <class ImplClass>
    WebSamplerListenerVTable const WebSamplerListenerHelper<ImplClass>::s_VTable =
    {
        sizeof( WebSamplerListenerVTable ),
        WebSamplerListenerHelper<ImplClass>::sDidEnterFunction,
        WebSamplerListenerHelper<ImplClass>::sDidExitFunction,
        WebSamplerListenerHelper<ImplClass>::sAtStatement,
        WebSamplerListenerHelper<ImplClass>::sDidAllocateObject,
        WebSamplerListenerHelper<ImplClass>::sDidDeallocateObject,
        WebSamplerListenerHelper<ImplClass>::sSetWebSamplerRef
    };
    
    template <class ImplClass>
    WebSamplerListenerHelper<ImplClass>::WebSamplerListenerHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebSamplerListenerHelper<ImplClass>::~WebSamplerListenerHelper()
    {
        m_pVTable = 0;
        // remove the virtual pointer
        (* reinterpret_cast<int*>( this )) = NULL; 
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebSamplerListenerHelper<ImplClass>::sDidEnterFunction( struct WebSamplerListener* pWebSamplerListener, uint64_t identifier, int32_t lineno )
    {

		getThis(pWebSamplerListener)->didEnterFunction(identifier, lineno);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebSamplerListenerHelper<ImplClass>::sDidExitFunction( struct WebSamplerListener* pWebSamplerListener, uint64_t identifier )
    {
		getThis(pWebSamplerListener)->didExitFunction(identifier);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebSamplerListenerHelper<ImplClass>::sAtStatement( struct WebSamplerListener* pWebSamplerListener, int32_t lineno )
    {
		getThis(pWebSamplerListener)->atStatement(lineno);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    uint64_t WEBKIT_APOLLO_PROTO2 WebSamplerListenerHelper<ImplClass>::sDidAllocateObject( struct WebSamplerListener* pWebSamplerListener, const void* ptr, size_t size  )
    {
		return getThis(pWebSamplerListener)->didAllocateObject(ptr, size);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebSamplerListenerHelper<ImplClass>::sDidDeallocateObject( struct WebSamplerListener* pWebSamplerListener, const void* ptr, size_t size  )
    {
		getThis(pWebSamplerListener)->didDeallocateObject(ptr, size);
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebSamplerListenerHelper<ImplClass>::sSetWebSamplerRef( struct WebSamplerListener* pWebSamplerListener, WebSampler **pWebSamplerRef )
    {
		getThis(pWebSamplerListener)->setWebSamplerRef(pWebSamplerRef);
	}
}

#endif //WebSamplerListenerHelper_h
