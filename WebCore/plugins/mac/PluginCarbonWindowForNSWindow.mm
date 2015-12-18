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
#include "config.h"
#include "npapi.h"
#include <wtf/Assertions.h>
#include "PluginCarbonWindowForNSWindow.h"
#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)

namespace WebCore {

WindowRef pluginCarbonWindowForNSWindow(NSWindow* nsWindow)
{
    ASSERT(nsWindow);
    WindowRef const carbonWindow = reinterpret_cast<WindowRef>([nsWindow windowRef]);
    bool const isKeyWindow = [nsWindow isKeyWindow] == YES;
    if (isKeyWindow)
        SelectWindow(carbonWindow);
    return carbonWindow;
}

IntRect pluginCarbonWindowRectForNSWindowRect(NSWindow* win, const IntRect& rect)
{
    NSRect const nsContentRect = [[win contentView] frame];
    NSRect const nsFrameRect = [win frameRectForContentRect: nsContentRect];
	return IntRect(rect.x(), rect.y() + nsFrameRect.size.height - nsContentRect.size.height, rect.width(), rect.height());
}
	
void pluginCarbonWindowCheckIsKey(NSWindow* nsWindow, WindowRef carbonWindow)
{
    ASSERT(nsWindow);
    ASSERT(carbonWindow);
    bool const isKeyWindow = [nsWindow isKeyWindow] == YES;
    if (isKeyWindow)
        SelectWindow(carbonWindow);
}


}
#endif // PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
