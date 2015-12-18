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
#ifndef WebPopupWindowClientImpl_h
#define WebPopupWindowClientImpl_h

#include <WebKitApollo/WebKit.h>
#include <WebKitApollo/helpers/WebPopupWindowClientHelper.h>
namespace WebCore {
	class PopupMenuClient;
}

namespace WebKitApollo
{
    class WebPopupWindowClientImpl : public WebPopupWindowClientHelper< WebPopupWindowClientImpl >
    {
        friend class WebPopupWindowClientHelper< WebPopupWindowClientImpl >;
    private:
		WebPopupWindowClientImpl(WebHost* pHost, WebCore::PopupMenuClient* pPopupMenuClient);
        virtual ~WebPopupWindowClientImpl();
    public:
        static WebPopupWindowClientImpl* construct(WebHost* pHost, WebCore::PopupMenuClient* pPopupMenuClient);
		void onItemClicked( int const itemIndex );
        void onPopupMenuDestroyed();

		WebHost *webHost() { return m_pHost; }

    private:

        WebPopupWindowClientImpl(const WebPopupWindowClientImpl& other);
        WebPopupWindowClientImpl& operator=(const WebPopupWindowClientImpl& other);
    
        bool m_inDestructor;
        WebHost* const m_pHost;
		WebCore::PopupMenuClient* m_pPopupMenuClient;
    };
}


#endif
