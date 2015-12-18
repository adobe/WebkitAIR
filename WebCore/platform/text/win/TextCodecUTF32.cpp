/*
* Copyright (C) 2004, 2006, 2008 Apple Inc. All rights reserved.
* Copyright (C) 2009 Adobe Systems Incorporated.  All rights reserved.
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
/* Based on ConvertUTF.c and ConvertUTF.h from Unicode */
/*
* Copyright 2001-2004 Unicode, Inc.
* 
* Disclaimer
* 
* This source code is provided as is by Unicode, Inc. No claims are
* made as to fitness for any particular purpose. No warranties of any
* kind are expressed or implied. The recipient agrees to determine
* applicability of information provided. If this file has been
* purchased on magnetic or optical media from Unicode, Inc., the
* sole remedy for any claim will be exchange of defective media
* within 90 days of receipt.
* 
* Limitations on Rights to Redistribute This Code
* 
* Unicode, Inc. hereby grants the right to freely use the information
* supplied in this file in the creation of products supporting the
* Unicode Standard, and to make copies of this file in any form
* for internal or external distribution as long as this notice
* remains attached.
*/

#include "config.h"
#include "TextCodecUTF32.h"

#include "PlatformString.h"
#include "StringBuffer.h"

static const int halfShift  = 10; /* used for shifting by 10 bits */

static const UChar32 halfBase = 0x0010000UL;
static const UChar32 halfMask = 0x3FFUL;

#define UNI_MAX_BMP (UChar32)0x0000FFFF
#define UNI_MAX_LEGAL_UTF32 (UChar32)0x0010FFFF

#define UNI_SUR_HIGH_START  (UChar32)0xD800
#define UNI_SUR_HIGH_END    (UChar32)0xDBFF
#define UNI_SUR_LOW_START   (UChar32)0xDC00
#define UNI_SUR_LOW_END     (UChar32)0xDFFF

using std::auto_ptr;

namespace WebCore {

void TextCodecUTF32::registerEncodingNames(EncodingNameRegistrar registrar)
{
    registrar("UTF-32LE", "UTF-32LE");
    registrar("UTF-32BE", "UTF-32BE");

    //registrar("ISO-10646-UCS-4", "UTF-32LE");
    //registrar("UCS-4", "UTF-32LE");
    registrar("UTF-32", "UTF-32LE");
}

static PassOwnPtr<TextCodec> newStreamingTextDecoderUTF32LE(const TextEncoding&, const void*)
{
    return new TextCodecUTF32(true);
}

static PassOwnPtr<TextCodec> newStreamingTextDecoderUTF32BE(const TextEncoding&, const void*)
{
    return new TextCodecUTF32(false);
}

void TextCodecUTF32::registerCodecs(TextCodecRegistrar registrar)
{
    registrar("UTF-32LE", newStreamingTextDecoderUTF32LE, 0);
    registrar("UTF-32BE", newStreamingTextDecoderUTF32BE, 0);
}

static size_t CharConvertUTF32ToUTF16(UChar32 src, UChar *dst, size_t dst_length)
{
    if (src < UNI_MAX_BMP) {  /* Target is a character <= 0xFFFF */
        /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
        if (src >= UNI_SUR_HIGH_START && src <= UNI_SUR_LOW_END) {
            return 0;
        }
        *dst++ = static_cast<UChar>(src); /* normal case */
        return 1;
    } else if (src < UNI_MAX_LEGAL_UTF32 && dst_length > 1) {
        src -= halfBase;
        *dst++ = static_cast<UChar>((src >> halfShift) + UNI_SUR_HIGH_START);
        *dst++ = static_cast<UChar>((src & halfMask) + UNI_SUR_LOW_START);
        return 2;
    }
    return 0;
}

static size_t CharConvertUTF16ToUTF32(const UChar *src, size_t src_length, UChar32 *dst)
{
    UChar32 ch = *src;
    /* If we have a surrogate pair, convert to UTF32 first. */
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
        /* If the 16 bits following the high surrogate are in the source buffer... */
        if (src_length > 1) {
            UChar32 ch2 = *(src+1);
            /* If it's a low surrogate, convert to UTF32. */
            if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
                    + (ch2 - UNI_SUR_LOW_START) + halfBase;
                return 2;
            }
        }
        return 0;
    } else if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) { /* UTF-16 surrogate values are illegal in UTF-32 */
        return 0;
    }
    *dst = ch;
    return 1;
}


String TextCodecUTF32::decode(const char* bytes, size_t length, bool, bool, bool&)
{
    if (length == 0)
        return String();

	ASSERT((length & 3) == 0);
    const unsigned char* p = reinterpret_cast<const unsigned char*>(bytes);
    size_t numChars = length / 4;

    StringBuffer buffer(numChars);
    UChar* q = buffer.characters();

    for (size_t i = 0; i < numChars; ++i) {
        UChar32 c = 0;
        if (m_littleEndian)
            c = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
        else
            c = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
        p += 4;
        size_t len = CharConvertUTF32ToUTF16(c, q, numChars - i);
		if (!len)
			break;
        q += len;
    }

    buffer.shrink(q - buffer.characters());

    return String::adopt(buffer);
}

CString TextCodecUTF32::encode(const UChar* characters, size_t length, UnencodableHandling)
{
    char* bytes;
    CString string = CString::newUninitialized(length * 4, bytes);

    // FIXME: CString is not a reasonable data structure for encoded UTF-16, which will have
    // null characters inside it. Perhaps the result of encode should not be a CString?
    size_t i = 0;
    size_t indexEncoded = 0;
    while (i < length)
    {
        UChar32 ch;
        size_t usedChars = CharConvertUTF16ToUTF32(characters + i, length - i, &ch);
        if (!usedChars)
            break;
        i += usedChars;
        if (m_littleEndian) {
            bytes[indexEncoded] = ch;
            bytes[++indexEncoded] = ch >> 8;
            bytes[++indexEncoded] = ch >> 16;
            bytes[++indexEncoded] = ch >> 24;
        } else {
            bytes[indexEncoded] = ch >> 24;
            bytes[++indexEncoded] = ch >> 16;
            bytes[++indexEncoded] = ch >> 8;
            bytes[++indexEncoded] = ch;
        }
    }

    return string;
}

} // namespace WebCore
