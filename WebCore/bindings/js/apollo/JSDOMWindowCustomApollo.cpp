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
#include "JSDOMWindowCustom.h"

#if PLATFORM(APOLLO)
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"

using namespace JSC;

namespace WebCore {

JSValue JSDOMWindow::parentSandboxBridge(ExecState* exec) const
{
    Frame* frame = impl()->frame();
    if (!frame)
        return jsUndefined();

    return frame->loader()->client()->getParentDoor(exec);
}

void JSDOMWindow::setParentSandboxBridge(ExecState* exec, JSValue value)
{
    Frame* frame = impl()->frame();
    if (!frame)
        return;

    frame->loader()->client()->setParentDoor(exec, value);
}

JSValue JSDOMWindow::childSandboxBridge(ExecState* exec) const
{

    Frame* frame = impl()->frame();
    if (!frame)
        return jsUndefined();

    return frame->loader()->client()->getChildDoor(exec);
}    

void JSDOMWindow::setChildSandboxBridge(ExecState* exec, JSValue value)
{
    Frame* frame = impl()->frame();
    if (!frame)
        return;

    frame->loader()->client()->setChildDoor(exec, value);
}

JSValue JSDOMWindow::htmlLoader(ExecState* exec) const
{
    Frame* frame = impl()->frame();
    if (!frame)
        return jsUndefined();

    return frame->loader()->client()->getHtmlLoader(exec);
}

JSValue JSDOMWindow::nativeWindow(ExecState* exec) const
{
    Frame* frame = impl()->frame();
    if (!frame)
        return jsUndefined();

    JSValue val = frame->loader()->client()->getHtmlLoader(exec);
    if (val.isUndefinedOrNull())
        return jsUndefined();
    val = val.get(exec, Identifier(exec, "stage"));
    if (val.isUndefinedOrNull())
        return jsUndefined();
    val = val.get(exec, Identifier(exec, "nativeWindow"));
    return val;
}        

JSValue JSDOMWindow::close(JSC::ExecState* exec, const JSC::ArgList& args)
{
    (void)args;

    Frame* frame = impl()->frame();
    if (frame && frame->loader()->client()->canFrameBeClosedOrFocusedFrom(exec))
        static_cast<DOMWindow*>(impl())->close();

    return jsUndefined();
}

JSValue JSDOMWindow::focus(JSC::ExecState* exec, const JSC::ArgList&)
{
    Frame* frame = impl()->frame();
    if (frame && frame->loader()->client()->canFrameBeClosedOrFocusedFrom(exec))
        static_cast<DOMWindow*>(impl())->focus();

    return jsUndefined();
}

JSValue JSDOMWindow::blur(JSC::ExecState* exec, const JSC::ArgList& args)
{
    (void)args;

    Frame* frame = impl()->frame();
    if (frame && frame->loader()->client()->canFrameBeClosedOrFocusedFrom(exec))
        static_cast<DOMWindow*>(impl())->blur();

    return jsUndefined();
}

}

#endif



