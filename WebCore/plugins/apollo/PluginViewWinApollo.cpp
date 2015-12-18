/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
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
#include "config.h"

#include "PluginView.h"
#include "PlatformKeyboardEvent.h"
#include "KeyboardEvent.h"
#include "FrameView.h"
#include "HostWindow.h"

#if PLATFORM(APOLLO) && ENABLE(NETSCAPE_PLUGIN_API)

namespace WebCore {

IntPoint PluginView::clientToWindow(const IntPoint &p) const
{
	// translate from ClientRect to WindowRect
	HWND hwnd = getApolloWebWindow()->m_pVTable->getPlatformWindow(getApolloWebWindow());

	RECT windowScreenRect;

	// screen coords of window rect
	GetWindowRect(hwnd, &windowScreenRect);

	POINT pt = { 0, 0 };

	ClientToScreen(hwnd, &pt);

	return IntPoint(p.x() + (pt.x - windowScreenRect.left),
		p.y() + (pt.y - windowScreenRect.top) );
}

IntPoint PluginView::viewportToWindow(const IntPoint &pIn) const
{
	IntPoint p = pIn;

	p = convertToApolloWindowPoint( p );

	return m_isWindowed ? p : clientToWindow(p);
}

void PluginView::userGestureTimerFired(Timer<PluginView>*)
{
	if(isCallingPlugin()) {
		m_userGestureTimer.startOneShot(0.25);
		return;
	}

	m_doingUserGesture = false;
}


void PluginView::setFocus(bool focus)
{
	if (platformPluginWidget() && focus)
		SetFocus(platformPluginWidget());

	Widget::setFocus(focus);
}

void PluginView::show()
{
	setSelfVisible(true);

	updatePluginWindow();

	if (isParentVisible() && platformPluginWidget())
		ShowWindow(platformPluginWidget(), SW_SHOWNA);

	Widget::show();
}

void PluginView::hide()
{
	setSelfVisible(false);

	if (isParentVisible() && platformPluginWidget())
		ShowWindow(platformPluginWidget(), SW_HIDE);

	Widget::hide();
}

bool PluginView::handleOnCopy()
{
	ASSERT(m_parentFrame);
	ASSERT(m_parentFrame->document());
	ASSERT(m_parentFrame->document()->defaultView());

	WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
		, 'c' /* winCharCode */
		, 0x43 /* Web_VK_C */ /* keyCode*/
		, false /* isShiftKey */
		, true /* isCtrlKey */
		, false /* isAltKey */ 
		, false  /* isMetaKey */
		, false  /* isAutoRepeat */);
	RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent,  m_parentFrame->document()->defaultView());

	// set Ctrl as pressed
	BYTE keyboardState[256] = {0};
	::GetKeyboardState(keyboardState);
	BYTE savedCtrlState = keyboardState[VK_CONTROL];
	keyboardState[VK_CONTROL] |= 0x80;
	::SetKeyboardState(keyboardState);

	handleEvent(keyDown.get());

	// restore Ctrl state
	keyboardState[VK_CONTROL] = savedCtrlState;
	::SetKeyboardState(keyboardState);

	return true;
}

bool PluginView::handleOnCut()
{
	ASSERT(m_parentFrame);
	ASSERT(m_parentFrame->document());
	ASSERT(m_parentFrame->document()->defaultView());

	WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
		, 'x' /* winCharCode */
		, 0x58 /* Web_VK_X */ /* keyCode*/
		, false /* isShiftKey */
		, true /* isCtrlKey */
		, false /* isAltKey */ 
		, false  /* isMetaKey */
		, false  /* isAutoRepeat */);
	RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent,  m_parentFrame->document()->defaultView());

	// set Ctrl as pressed
	BYTE keyboardState[256] = {0};
	::GetKeyboardState(keyboardState);
	BYTE savedCtrlState = keyboardState[VK_CONTROL];
	keyboardState[VK_CONTROL] |= 0x80;
	::SetKeyboardState(keyboardState);

	handleEvent(keyDown.get());

	// restore Ctrl state
	keyboardState[VK_CONTROL] = savedCtrlState;
	::SetKeyboardState(keyboardState);

	return true;
}

bool PluginView::handleOnPaste()
{
	ASSERT(m_parentFrame);
	ASSERT(m_parentFrame->document());
	ASSERT(m_parentFrame->document()->defaultView());

	WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
		, 'v' /* winCharCode */
		, 0x56 /* Web_VK_V */ /* keyCode*/
		, false /* isShiftKey */
		, true /* isCtrlKey */
		, false /* isAltKey */ 
		, false  /* isMetaKey */
		, false  /* isAutoRepeat */);
	RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent,  m_parentFrame->document()->defaultView());

	// set Ctrl as pressed
	BYTE keyboardState[256] = {0};
	::GetKeyboardState(keyboardState);
	BYTE savedCtrlState = keyboardState[VK_CONTROL];
	keyboardState[VK_CONTROL] |= 0x80;
	::SetKeyboardState(keyboardState);

	handleEvent(keyDown.get());

	// restore Ctrl state
	keyboardState[VK_CONTROL] = savedCtrlState;
	::SetKeyboardState(keyboardState);

	return true;
}

bool PluginView::handleSelectAll()
{
	ASSERT(m_parentFrame);
	ASSERT(m_parentFrame->document());
	ASSERT(m_parentFrame->document()->defaultView());

	WebCore::PlatformKeyboardEvent keyEvent( WebCore::PlatformKeyboardEvent::RawKeyDown
		, 'a' /* winCharCode */
		, 0x41 /* Web_VK_A */ /* keyCode*/
		, false /* isShiftKey */
		, true /* isCtrlKey */
		, false /* isAltKey */ 
		, false  /* isMetaKey */
		, false  /* isAutoRepeat */);
	RefPtr<WebCore::KeyboardEvent> keyDown = WebCore::KeyboardEvent::create(keyEvent,  m_parentFrame->document()->defaultView());

	// set Ctrl as pressed
	BYTE keyboardState[256] = {0};
	::GetKeyboardState(keyboardState);
	BYTE savedCtrlState = keyboardState[VK_CONTROL];
	keyboardState[VK_CONTROL] |= 0x80;
	::SetKeyboardState(keyboardState);

	handleEvent(keyDown.get());

	// restore CTRL state
	keyboardState[VK_CONTROL] = savedCtrlState;
	::SetKeyboardState(keyboardState);

	return true;
}

void PluginView::paintIntoTransformedContext(HDC hdc, IntPoint& location)
{
	if (m_isWindowed) {
		return;
	}

	m_npWindow.type = NPWindowTypeDrawable;
	m_npWindow.window = hdc;

	WINDOWPOS windowpos = { 0 };

#if  PLATFORM(APOLLO)
    IntPoint p = viewportToWindow(location);

    windowpos.x = p.x();
    windowpos.y = p.y();
    windowpos.cx = frameRect().width();
    windowpos.cy = frameRect().height();
#else
	windowpos.x = location.x();
	windowpos.y = location.y();
	windowpos.cx = frameRect().width();
	windowpos.cy = frameRect().height();
#endif

	NPEvent npEvent;
	npEvent.event = WM_WINDOWPOSCHANGED;
	npEvent.lParam = reinterpret_cast<uintptr_t>(&windowpos);
	npEvent.wParam = 0;

	dispatchNPEvent(npEvent);

	setNPWindowRect(frameRect());

	npEvent.event = WM_PAINT;
	npEvent.wParam = reinterpret_cast<uintptr_t>(hdc);

	// This is supposed to be a pointer to the dirty rect, but it seems that the Flash plugin
	// ignores it so we just pass null.
	npEvent.lParam = 0;

	dispatchNPEvent(npEvent);
}
}

#endif