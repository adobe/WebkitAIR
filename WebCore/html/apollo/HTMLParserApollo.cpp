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
#include "HTMLParser.h"

#if PLATFORM(APOLLO)

#include "Document.h"
#include <WebKitApollo/WebKit.h>
#include "FrameLoaderClientApollo.h"
#include "WebKitApollo/WebHost.h"

namespace WebKitApollo
{
    extern WebKitAPIHostFunctions* g_HostFunctions;
}

namespace WebCore {

bool shouldCreateImplicitHead(Document* document)
{
#ifdef NDEBUG
    UNUSED_PARAM(document);
#else
    ASSERT(document);
#endif

	Frame* frame = document->frame();
	
	// investigate if this assert fires for versions greater than WEBKIT_APOLLO_AIR_2_0
	ASSERT(frame);
    
	if (frame)
	{
		WebHost* webHost = FrameLoaderClientApollo::clientApollo(frame)->webHost();
		if(webHost->m_pVTable->isPlatformAPIVersion(webHost,WEBKIT_APOLLO_AIR_2_0))
		    return true;
    }

    return false;
}

}

#endif

