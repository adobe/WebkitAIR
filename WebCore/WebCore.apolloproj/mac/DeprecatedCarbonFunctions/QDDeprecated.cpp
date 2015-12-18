/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "QDDeprecated.h"
#include <ApplicationServices/ApplicationServices.h>
#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)

namespace WebCore { namespace QDDeprecated {

::Rect* GetRegionBounds(RgnHandle region, ::Rect* bounds)
{
    return ::GetRegionBounds(region, bounds);
}

void SetRect(::Rect* r, short left, short top, short right, short bottom)
{
    return ::SetRect(r, left, top, right, bottom);
}

void SetEmptyRgn(RgnHandle region)
{
    ::SetEmptyRgn(region);
}

void GetPort(GrafPtr* port)
{
    ::GetPort(port);
}

void SetPort(GrafPtr const port)
{
    ::SetPort(port);
}

void SetOrigin(short const h, short const v)
{
    ::SetOrigin(h, v);
}

RgnHandle NewRgn(void)
{
    return ::NewRgn();
}

void DisposeRgn(RgnHandle rgn)
{
    ::DisposeRgn(rgn);
}

void MacSetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom)
{
    ::MacSetRectRgn(rgn, left, top, right, bottom);
}

void SetClip(RgnHandle rgn)
{
    ::SetClip(rgn);
}

::QDErr NewPortFromBytes(
  ::GrafPtr* newPort,
  const ::Rect* boundsRect,
  void* bytes,
  long rowBytes)
{
    return ::NewGWorldFromPtr( newPort, k32BGRAPixelFormat, boundsRect, NULL, NULL, kNativeEndianPixMap, reinterpret_cast<char *>(bytes), rowBytes );
}

void DisposePortFromBytes( ::GrafPtr port )
{
    DisposeGWorld(port);
}

void LocalToGlobal(::Point* const p)
{
    ::LocalToGlobal(p);
}

} }

#endif // PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
