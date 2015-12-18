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
#include "FrameLoader.h"

#if PLATFORM(APOLLO)

#include "FloatRect.h"
#include "Event.h"
#include "FormState.h"
#include "HTMLFormElement.h"
#include "Frame.h"
#include "FrameLoadRequest.h"
#include "FrameTree.h"
#include "Page.h"
#include "WindowFeatures.h"
#include "FrameLoaderClient.h"
#include "Chrome.h"

namespace WebCore {

/*
 * The Apollo interface wants the Window features all at one time since the customization interface
 * creates the Frame and the window all at the same time.
 */

Frame* FrameLoader::createWindow(FrameLoader* frameLoaderForFrameLookup, const FrameLoadRequest& request, const WindowFeatures& features, bool& created)
{ 
    ASSERT(!features.dialog || request.frameName().isEmpty());
    created = false;

    if (!request.frameName().isEmpty() && request.frameName() != "_blank") {
        Frame* frame = frameLoaderForFrameLookup->frame()->tree()->find(request.frameName());
        if (frame && shouldAllowNavigation(frame, kCheckUserGestureIfTargetIsNull)) {
            if (!request.resourceRequest().url().isEmpty())
                frame->loader()->loadFrameRequest(request, false, false, 0, 0, NoReferrer);
            if (Page* page = frame->page())
                page->chrome()->focus();
            created = false;
            return frame;
        }
    }

    // FIXME: Setting the referrer should be the caller's responsibility.
    FrameLoadRequest requestWithReferrer = request;
    requestWithReferrer.resourceRequest().setHTTPReferrer(m_outgoingReferrer);
    
    Page* oldPage = m_frame->page();
    if (!oldPage)
        return 0;

    Page* page = oldPage->chrome()->createWindow(m_frame, requestWithReferrer, features);
    if (!page)
        return 0;

    created = true;

    Frame* frame = page->mainFrame();
    if (request.frameName() != "_blank")
        frame->tree()->setName(request.frameName());

	return frame;
}

void FrameLoader::dispatchWindowObjectDead()
{
    m_client->windowObjectDead();
}

// In AIR 2.0 we do not take x-frame-options into account, we ignore this http header.
// This option will be taken into account for next AIR versions.
// Please see Watson 2574614: [Injection] If root content of an app is html with iframe sourced to gmail/yahoo mail, 
//  then user cannot log into his/her mail account.
// Both GMail and Yahoo are sending this http header.
// x-frame-options is a HTTP response header.
// It can be used to prevent framing of the pages that are delivered to browsers in the browser: 
// the browser simply refuses to render the page in a frame if the header is present dependign on the set value.
// Values are: 
//  * DENY: Stops all framing 
//  * SAMEORIGIN: Stops framing except for the same website that delivered the page itself. 
//      (Allowing http://www.example.com/ to frame pages served from http://www.example.com/ with x-frame-options set to this value)
bool FrameLoader::shouldInterruptLoadForXFrameOptions(const String&, const KURL&)
{
    return false;
}

void FrameLoader::dispatchDOMInitializeToElement(Element* ownerElement)
{
    if (ownerElement && !m_creatingInitialEmptyDocument && !m_dispatchedDOMInitializeEvent) {
        RefPtr<WebCore::Event> ownerEvent = WebCore::Event::create(eventNames().dominitializeEvent, false, false);
        WebCore::ExceptionCode ec = 0;
        ownerEvent->setTarget(ownerElement);
        ownerElement->dispatchEvent(ownerEvent.release(), ec);
        m_dispatchedDOMInitializeEvent = true;
    }
}

}

#endif //PLATFORM(APOLLO)
