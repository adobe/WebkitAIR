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

#include <dlfcn.h>
#include "UtilsMac.h"

// These values are not defined in Mac OS 10.4 SDK
#ifndef NSAppKitVersionNumber10_4
#define NSAppKitVersionNumber10_4 824
#define NSAppKitVersionNumber10_4_1 824.1
#define NSAppKitVersionNumber10_4_3 824.23
#define NSAppKitVersionNumber10_4_4 824.33
#define NSAppKitVersionNumber10_4_7 824.41
#endif

#ifndef NSAppKitVersionNumber10_5
#define NSAppKitVersionNumber10_5 949
#define NSAppKitVersionNumber10_5_2 949.27
#define NSAppKitVersionNumber10_5_3 949.33
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	extern const double NSAppKitVersionNumber;
	
#ifdef __cplusplus
}
#endif

namespace WebCore {
	
	_CGFontGetNumberOfGlyphsFunc getCGFontGetNumberOfGlyphsFunc()
	{
		static _CGFontGetNumberOfGlyphsFunc CGFontGetNumberOfGlyphsFunc = 0;
		static bool funcSearched = false;
		
		// We shouldn't call dlsym more than once
		if (!funcSearched) {
			CGFontGetNumberOfGlyphsFunc = reinterpret_cast<_CGFontGetNumberOfGlyphsFunc> (dlsym(RTLD_DEFAULT, "CGFontGetNumberOfGlyphs"));
			funcSearched = true;
		}
		
		return CGFontGetNumberOfGlyphsFunc;
	}
	
	bool isOsLeopardOrGreater()
	{
		return NSAppKitVersionNumber >= NSAppKitVersionNumber10_5;
	}
}
