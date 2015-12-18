/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "Widget.h"

#include "Cursor.h"
#include "Document.h"
#include "Element.h"
#include "GraphicsContext.h"
#include "IntRect.h"
#include "FrameView.h"

#include "WebKitApollo/WebHost.h"
#include "FrameLoaderClientApollo.h"

#include "HostWindow.h"
#include <WebKitApollo/WebKit.h>
#include <WebKitApollo/WebWindow.h>

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions; }

namespace WebCore {

Widget::Widget(PlatformWidget widget)
{
    init(widget);
}

Widget::~Widget() 
{
    ASSERT(!parent());
}

void Widget::show()
{
}

void Widget::hide()
{
}

bool ignoreNextSetCursor = false;

void Widget::setCursor(const Cursor& cursor)
{
    // This is set by PluginViewWin so it can ignore set setCursor call made by
    // EventHandler.cpp.
    if (ignoreNextSetCursor) {
        ignoreNextSetCursor = false;
        return;
    }

    WebCore::FrameLoaderClientApollo* clientApollo = WebCore::FrameLoaderClientApollo::clientApollo(static_cast<FrameView*>(root()));
    WebHost* webHost = clientApollo->webHost();
    webHost->m_pVTable->setMouseCursor(webHost, cursor.impl());
}

void Widget::paint(GraphicsContext*, const IntRect&)
{
}

void Widget::setFocus(bool)
{
}

void Widget::setIsSelected(bool)
{
}

IntRect Widget::frameRect() const
{
    if (isApolloMainFrameView())
        return IntRect(IntPoint(0, 0), apolloContentSize());

    return m_frame;
}

void Widget::setFrameRect(const IntRect& rect)
{
    m_frame = rect;
}


IntPoint Widget::convertToApolloWindowPoint(const IntPoint& point) const
{
    if(!parent())
        return point;
    
    WebWindow* pWebWindow = parent()->hostWindow()->getApolloWindow();
    ASSERT(pWebWindow);
    WebIntPoint webPoint = { point.x(), point.y() };
    WebIntPoint webResultPoint;
    pWebWindow->m_pVTable->viewportToWindow(pWebWindow, &webPoint, &webResultPoint);
    return IntPoint(webResultPoint.m_x, webResultPoint.m_y);
}

IntRect Widget::convertToApolloWindowRect(const IntRect& rect) const
{
    return IntRect(convertToApolloWindowPoint(rect.location()), rect.size());
}

WebWindow* Widget::getApolloWebWindow() const
{
    if(!parent())
        return 0;
    return parent()->hostWindow()->getApolloWindow();    
}

} // namespace WebCore
