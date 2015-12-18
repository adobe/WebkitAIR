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
#include <config.h>

#include <stdio.h>
#include <SharedBuffer.h>
#include <DocumentFragment.h>
#include <FontCache.h>
#include <FontData.h>
#include "FontDescription.h"
#include <wtf/MathExtras.h>
#include <cairo.h>
#include <ContextMenuItem.h>
#include <Image.h>
#include <Editor.h>
#include <Icon.h>
#include <PluginInfoStore.h>
#include <FloatRect.h>
#include <KURL.h>

#if DEBUG
#define notImplemented() do { \
    char buf[256] = {0}; \
    snprintf(buf, sizeof(buf), "TODO_AIRLinux: UNIMPLEMENTED: %s:%d\n", __FILE__, __LINE__); \
    fprintf(stderr, buf); \
} while (0)
#else
#define notImplemented()
#endif

namespace WebCore {

float userIdleTime()
{
    notImplemented();
    return 0;
}

//Icon::Icon()
//{
//    notImplemented();
//}

String KURL::fileSystemPath() const 
{ 
    ASSERT(0);
    notImplemented(); 
    return String(); 
}

}
