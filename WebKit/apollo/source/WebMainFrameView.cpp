/*
 * Copyright (C) 2005, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Adobe Systems Incorporated.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config.h>
#include "WebMainFrameView.h"
#include <WebKitApollo/WebWindow.h>
#include <WebKitApollo/WebHost.h>
#include <Frame.h>

WebMainFrameView::WebMainFrameView(WebCore::Frame* main, WebWindow* pWebWindow)
    : WebCore::FrameView(main),
      m_pWebWindow(pWebWindow),
      m_isInScrollEvent(false),
      m_hasHostScrollbars(true)
{
    // this is to compensate since the ctor for FrameView does not call show again
    show();
}
WebMainFrameView::~WebMainFrameView()
{
}

WebCore::IntSize WebMainFrameView::apolloContentSize() const
{
    int const visibleWidth = m_pWebWindow->m_pVTable->visibleWidth( m_pWebWindow );
    int const visibleHeight = m_pWebWindow->m_pVTable->visibleHeight( m_pWebWindow );
    
    return WebCore::IntSize(visibleWidth, visibleHeight);
}

void WebMainFrameView::updateApolloScrollbars(const IntSize& scrollOffset)
{
    if(m_isInScrollEvent)
        return;
    
    m_pWebWindow->m_pVTable->setContentsPos(m_pWebWindow, scrollOffset.width(), scrollOffset.height());
}

