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
#include "CSSFontSelector.h"
#include "Document.h"

#if PLATFORM(APOLLO)

#include "FrameLoaderClientApollo.h"
#include "WebKitApollo/WebHost.h"
#include "RenderObject.h"

namespace WebCore {

#if OS(DARWIN)
// On Mac, fontCacheInvalidated can be called directly from OS, via ATSFontNotification,
// See FontCacheMac.mm
void fontCacheCallback(void *arg);
void fontCacheCallback(void *arg)
{
    Document* document = static_cast<Document *>(arg);
    document->recalcStyle(Document::Force);
    document->renderer()->setNeedsLayoutAndPrefWidthsRecalc();    
}

void CSSFontSelector::fontCacheInvalidated()
{
    if (!m_document || m_document->inPageCache() || !m_document->renderer())
        return;

    if (m_document->frame()) {
        WebHost* webHost = FrameLoaderClientApollo::clientApollo(m_document->frame())->webHost();
        ASSERT(webHost);
        webHost->m_pVTable->enterPlayer(webHost, fontCacheCallback, (void *)m_document);
        return;
    }
}

#endif //OS(DARWIN)

} //namespace WebCore

#endif //PLATFORM(APOLLO)

