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
#include "JSDOMWindowBase.h"
#include "JSDOMWindowCustom.h"

#include "Page.h"


#if PLATFORM(APOLLO)
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClientApollo.h"
#include "FrameTree.h"
#include "ScriptController.h"
#include "sampler/SamplerApollo.h"

using namespace JSC;

namespace WebCore {

#if ENABLE(APOLLO_PROFILER)
bool JSDOMWindowBase::supportsSampling() const
{
    Frame* frame = impl()->frame();
    if (!frame)
        return false;

    Page* page = frame->page();
    if (!page)
        return false;

    return (*SamplerApollo::enabledSamplerReference());
}
#endif

EvalLevel JSDOMWindowBase::currentEvalLevel() const
{
    Frame* frame = impl()->frame();
    return frame->loader()->client()->currentEvalLevel();
}

bool JSDOMWindowBase::canConstructFunctions() const
{
    // Whatever the default value, we need to allow the function constructor
    // when defining an event handler
    if (definingEventHandler())
        return true;

    Frame* frame = impl()->frame();
    return frame->loader()->client()->canConstructFunctions();
}

void JSDOMWindowBase::setDefiningEventHandler(bool definingEventHandler)
{
    ASSERT(d());
    d()->m_definingEventHandler = definingEventHandler;
}

bool JSDOMWindowBase::definingEventHandler() const {
    ASSERT(d());
    return d()->m_definingEventHandler;
}

void *JSDOMWindowBase::platformInterpreterContext()
{
    Frame* frame = impl()->frame();
    FrameLoaderClientApollo* frameLoaderClient = FrameLoaderClientApollo::clientApollo(frame);
    ASSERT(frameLoaderClient);
    void *interpreterContext = frameLoaderClient->getPlatformInterpreterContext();

    // Non-root frames don't have their own context.  Null is treated as being in a neutral
    // "about:blank" domain.  But if this frame can script to the top frame, then we will allow
    // let it use the top frame's interpreter context.
	Frame* mainFrame =  frame->page() ? frame->page()->mainFrame() : 0;
    if (mainFrame && (frame != mainFrame)) {
        ASSERT(interpreterContext == 0);

        JSDOMWindow* window = toJSDOMWindow(mainFrame, mainThreadNormalWorld());
        if (window && window->allowsAccessFromPrivate(this)) {
            frameLoaderClient = FrameLoaderClientApollo::clientApollo(mainFrame);
            ASSERT(frameLoaderClient);
            interpreterContext = frameLoaderClient->getPlatformInterpreterContext();
        }
    }
    return interpreterContext;
}

JSC::UString JSDOMWindowBase::typeName() const
{
	if (!impl()->frame())
		return UString(classInfo()->className);
    KURL targetURL = impl()->frame()->document()->url();
    if (targetURL.isNull())
        return UString(classInfo()->className);

    String name("<WindowObject>");
    name.append(targetURL.string());
    return stringToUString(name);
}

}

#endif


