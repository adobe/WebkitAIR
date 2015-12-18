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
#ifndef PluginCarbonWindow_h
#define PluginCarbonWindow_h

#include "npapi.h"
#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)

#include <wtf/Noncopyable.h>
#include "IntRect.h"
#include "IntPoint.h"

struct OpaqueWindowPtr;
typedef OpaqueWindowPtr* WindowRef;
struct _NPWindow;
typedef _NPWindow NPWindow;

namespace WebCore {

class PluginView;

class PluginCarbonWindow : public Noncopyable {

public:
    virtual ~PluginCarbonWindow();
    static std::auto_ptr<PluginCarbonWindow> construct(PluginView*);

    inline WindowRef windowRef() const { return m_windowRef; }
    
    void translatePoint(Point* const translatedPoint, int const eventX, int const eventY);
    bool fillInNPWindowPositionInfoForPainting(NPWindow* npWindow);
    bool fillInNPWindowPositionInfoForEvents(NPWindow* npWindow);
     
    
private:
    static WindowRef createTransparentWindow();
    PluginCarbonWindow(PluginView*);
    IntRect syncWindowAndGetGeometry();
    
    
    WindowRef m_windowRef;
    PluginView* m_ownerPluginView;
};

}


#endif // PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
#endif

