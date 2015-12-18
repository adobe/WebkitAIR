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
#include "WebPopupWindowClientImpl.h"
#include <config.h>
#include "PopupMenuClient.h"


namespace WebKitApollo
{
WebPopupWindowClientImpl::WebPopupWindowClientImpl(WebHost* pHost, WebCore::PopupMenuClient* popupMenuClient)
    : m_inDestructor(false)
    , m_pHost(pHost)
	, m_pPopupMenuClient( popupMenuClient )
{
}

WebPopupWindowClientImpl* WebPopupWindowClientImpl::construct(WebHost* pHost, WebCore::PopupMenuClient* pPopupMenuClient)
{
    return new WebPopupWindowClientImpl(pHost, pPopupMenuClient);
}

WebPopupWindowClientImpl::~WebPopupWindowClientImpl()
{
    m_inDestructor = true;
}

void WebPopupWindowClientImpl::onItemClicked( int const itemIndex )
{
	m_pPopupMenuClient->setTextFromItem( itemIndex );
	m_pPopupMenuClient->valueChanged( itemIndex );
}

void WebPopupWindowClientImpl::onPopupMenuDestroyed()
{
    //Latest updates on Webkit (bug 29138) renamed the old PopupMenuClient::hidePopup
    //into PopupMenuClient::popupDidHide which is now only setting some flag. All other logic
    //like clearing the popup window pointer, moved to the new RenderMenuList::hidePopup.
    //Not clearing the popup window pointer resulted in a crash so we had to restore the 
    //call here. hidePopup became virtual such that can be executed via base class pointer.

    //The whole AIR Popup Menu logic should be subjected to refactorization.

    m_pPopupMenuClient->hidePopup();

	m_pPopupMenuClient->popupDidHide();
    delete this;
}

}

