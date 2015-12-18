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
#ifndef WebPopupWindowClientHelper_h
#define WebPopupWindowClientHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo {
    template <class ImplClass>
    class WebPopupWindowClientHelper : private WebPopupWindowClient {

    public:
    
        inline WebPopupWindowClient* getWebPopupWindowClient() { return this; }
 
        static inline ImplClass* getImpl( WebPopupWindowClient* const pWebPopupWindowClient )
        {
            //ASSERT( pWebPopupWindowClient );
            //ASSERT( pWebPopupWindowClient->m_pVTable == &s_VTable );
            return getThis( pWebPopupWindowClient );
        }

    protected:
        WebPopupWindowClientHelper();
        virtual ~WebPopupWindowClientHelper();
        
    private:
        static WebPopupWindowClientVTable const s_VTable;
        static ImplClass* getThis(WebPopupWindowClient* const pWebPopupWindowClient) { return static_cast<ImplClass*>(pWebPopupWindowClient); }
        static const ImplClass* getThis(const WebPopupWindowClient* const pWebPopupWindowClient) { return static_cast<const ImplClass*>(pWebPopupWindowClient); }

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnPopupMenuDestroyed(struct WebPopupWindowClient* pPopupWindow);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sOnItemClicked(struct WebPopupWindowClient* pPopupWindow, int const itemIndex);        
    };

    template <class ImplClass>
    WebPopupWindowClientVTable const WebPopupWindowClientHelper<ImplClass>::s_VTable = {
        sizeof(WebPopupWindowClientVTable),
        WebPopupWindowClientHelper<ImplClass>::sOnPopupMenuDestroyed,
        WebPopupWindowClientHelper<ImplClass>::sOnItemClicked
    };

    template<class ImplClass>
    WebPopupWindowClientHelper<ImplClass>::WebPopupWindowClientHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
	WebPopupWindowClientHelper<ImplClass>::~WebPopupWindowClientHelper()
	{
        m_pVTable = 0;
	}
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowClientHelper<ImplClass>::sOnPopupMenuDestroyed(struct WebPopupWindowClient* pPopupWindow)
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        pThis->onPopupMenuDestroyed();
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowClientHelper<ImplClass>::sOnItemClicked(struct WebPopupWindowClient* pPopupWindow, int const itemIndex)
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        pThis->onItemClicked(itemIndex);
    }
}

#endif
