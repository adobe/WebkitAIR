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

#include <WebKitApollo/WebKit.h>
#include <wtf/Assertions.h>
#include "SystemTime.h"
#include <wtf/CurrentTime.h>

namespace WebKitApollo
{

extern WebKitAPIHostFunctions* g_HostFunctions;

}

namespace WebCore {

namespace {

typedef void (*SharedTimerFiredFunction)();

static SharedTimerFiredFunction g_sharedTimerFiredFunction = 0;

static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sharedTimerFiredFunction()
{
    ASSERT(g_sharedTimerFiredFunction);
    g_sharedTimerFiredFunction();
}

}

void stopSharedTimer();
void stopSharedTimer()
{
    ASSERT(WebKitApollo::g_HostFunctions);
    WebKitApollo::g_HostFunctions->stopSharedTimer();
}

void setSharedTimerFireTime(double nextFireTime);
void setSharedTimerFireTime(double nextFireTime)
{
    ASSERT(WebKitApollo::g_HostFunctions);
    ASSERT(g_sharedTimerFiredFunction);
    double delay = nextFireTime - currentTime();
    int delayMs = int(delay*1000);
    WebKitApollo::g_HostFunctions->setSharedTimerFireTime(delayMs);
}

void setSharedTimerFiredFunction(SharedTimerFiredFunction firedFunction);
void setSharedTimerFiredFunction(SharedTimerFiredFunction firedFunction)
{
    ASSERT(WebKitApollo::g_HostFunctions);
    g_sharedTimerFiredFunction = firedFunction;
    WebKitApollo::g_HostFunctions->setSharedTimerFiredFunction(sharedTimerFiredFunction);
    ASSERT(g_sharedTimerFiredFunction);
}

}

