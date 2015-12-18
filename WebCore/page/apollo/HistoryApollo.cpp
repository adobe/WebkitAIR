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
#include "History.h"

#if PLATFORM(APOLLO)

#include "JSHistory.h"
#include "Frame.h"
#include "FrameLoaderClient.h"
#include "ExceptionCode.h"

namespace WebCore {

// In Apollo, history methods are allowed in the same cases that focus and close methods are allowed.

JSC::JSValue JSHistory::back(JSC::ExecState* exec, const JSC::ArgList&)
{
    History *history = impl();
    Frame* frame = history->frame();
    if (frame && frame->loader()->client()->canFrameBeClosedOrFocusedFrom(exec))
        history->back();
    return JSC::jsUndefined();
}

JSC::JSValue JSHistory::forward(JSC::ExecState* exec, const JSC::ArgList&)
{
    History *history = impl();
    Frame* frame = history->frame();
    if (frame && frame->loader()->client()->canFrameBeClosedOrFocusedFrom(exec))
        history->forward();
    return JSC::jsUndefined();
}

JSC::JSValue JSHistory::go(JSC::ExecState* exec, const JSC::ArgList& args)
{
    History *history = impl();
    Frame* frame = history->frame();
    if (frame && frame->loader()->client()->canFrameBeClosedOrFocusedFrom(exec)) {
        bool distanceOk;
        int distance = args.at(0).toInt32(exec, distanceOk);
        if (!distanceOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return JSC::jsUndefined();
        }
        history->go(distance);
    }
    return JSC::jsUndefined();
}

} //namespace WebCore

#endif //PLATFORM(APOLLO)

