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
 
#ifndef QDDeprecated_h
#define QDDeprecated_h
#include "config.h"
#include "npapi.h"
#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)

typedef struct OpaqueRgnHandle*         RgnHandle;
struct Rect;
typedef struct OpaqueGrafPtr * GrafPtr;
typedef short QDErr;
typedef unsigned long GWorldFlags;

namespace WebCore { namespace QDDeprecated {

extern ::Rect* GetRegionBounds(RgnHandle region, ::Rect* bounds);
extern void SetRect(::Rect* r, short left, short top, short right, short bottom);
extern void SetEmptyRgn(RgnHandle region);
extern void GetPort(GrafPtr* port);
extern void SetPort(GrafPtr const port);
extern void SetOrigin(short const h, short const v);
extern RgnHandle NewRgn(void);
extern void DisposeRgn(RgnHandle rgn);
extern void MacSetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom);
extern void SetClip(RgnHandle rgn);
    
// this is really a call to NewGWorldFromPtr with some of the args set to 
// specific values.
// PixelFormat = k32ARGBPixelFormat
// CTabHandle = NULL
// GDHandle = NULL
// GWorldFlags = 0
extern ::QDErr NewPortFromBytes(
  GrafPtr* offscreenGWorld,
  const ::Rect* boundsRect,
  void* bytes,
  long rowBytes);
extern void DisposePortFromBytes( ::GrafPtr port );

extern void LocalToGlobal(::Point* const p);

} }

#endif  // PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
#endif
