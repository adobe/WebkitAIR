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
#include "ImageSource.h"

namespace WebCore {

//ImageSource::IMAGE_TYPE ImageSource::getImageType(const Vector<char>& data)
//{
//    // We need at least 4 bytes to figure out what kind of image we're dealing with.
//    int length = data.size();
//    if (length < 4)
//        return IMAGE_TYPE_UNKNOWN;
//
//    const unsigned char* uContents = (const unsigned char*)data.data();
//    const char* contents = data.data();
//
//    // GIFs begin with GIF8(7 or 9).
//    if (strncmp(contents, "GIF8", 4) == 0)
//        return IMAGE_TYPE_GIF;
//
//    // Test for PNG.
//    if (uContents[0]==0x89 &&
//        uContents[1]==0x50 &&
//        uContents[2]==0x4E &&
//        uContents[3]==0x47)
//        return IMAGE_TYPE_PNG;
//
//    // JPEG
//    if (uContents[0]==0xFF &&
//        uContents[1]==0xD8 &&
//        uContents[2]==0xFF)
//        return IMAGE_TYPE_JPEG;
//
//    // BMP
//    if (strncmp(contents, "BM", 2) == 0)
//        return IMAGE_TYPE_BMP;
//
//    // ICOs always begin with a 2-byte 0 followed by a 2-byte 1.
//    // CURs begin with 2-byte 0 followed by 2-byte 2.
//    if (!memcmp(contents, "\000\000\001\000", 4) ||
//        !memcmp(contents, "\000\000\002\000", 4))
//        return IMAGE_TYPE_ICO;
//
//    // XBMs require 8 bytes of info.
//    if (length >= 8 && strncmp(contents, "#define ", 8) == 0)
//        return IMAGE_TYPE_XBM;
//
//    // Give up. We don't know what the heck this is.
//    return IMAGE_TYPE_UNKNOWN;    
//}

}
