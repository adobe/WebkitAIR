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
#include "SmartReplace.h"
#include <wtf/Assertions.h>

#if PLATFORM(APOLLO_UNIX)
//needed for the 'lower_bound' function
#include <algorithm>
#endif

namespace WebCore {

namespace {

struct CharacterRange
{
    unsigned long const firstCharacter;
    size_t const numCharacters;
};

static const CharacterRange preCharacterSet[] =
{
    { 0x22, 3 },  // '\"', '#', '$'
    { 0x27, 2 },  // '\'', '('
    { 0x2d, 1 },  // '-'
    { 0x2f, 1 },  // '/'
    { 0x5b, 1 },  // '['
    { 0x60, 1 },  // '`'
    { 0x7B, 1 },  // '{'
    { 0x1100, 256 }, // Hangul Jamo (0x1100 - 0x11FF)
    { 0x2E80, 352 }, // CJK & Kangxi Radicals (0x2E80 - 0x2FDF)
    { 0x2FF0, 464 }, // Ideograph Descriptions, CJK Symbols, Hiragana, Katakana, Bopomofo, Hangul Compatibility Jamo, Kanbun, & Bopomofo Ext (0x2FF0 - 0x31BF)
    { 0x3200, 29392 }, // Enclosed CJK, CJK Ideographs (Uni Han & Ext A), & Yi (0x3200 - 0xA4CF)
    { 0xAC00, 11183 }, // Hangul Syllables (0xAC00 - 0xD7AF)
    { 0xF900, 352 }, // CJK Compatibility Ideographs (0xF900 - 0xFA5F)
    { 0xFE30, 32 }, // CJK Compatibility From (0xFE30 - 0xFE4F)
    { 0xFF00, 240 }, // Half/Full Width Form (0xFF00 - 0xFFEF)
    { 0x20000, 0xA6D7 }, // CJK Ideograph Exntension B
    { 0x2F800, 0x021E } // CJK Compatibility Ideographs (0x2F800 - 0x2FA1D)
};

static const size_t numPreCharacterSetEntries = sizeof(preCharacterSet) / sizeof(CharacterRange);

static const CharacterRange postCharacterSet[] =
{
    { 0x00022, 1      }, // '"'
    { 0x00025, 1      }, // '%'
    { 0x00027, 1      }, // '\''
    { 0x00029, 2      }, // ')', '*'
    { 0x0002c, 4      }, // ',', '-', '.', '/'
    { 0x0003a, 2      }, // ':', ';'
    { 0x0003f, 1      }, // '?'
    { 0x0005d, 1      }, // ']'
    { 0x0007d, 1      }, // '}'
    { 0x01100, 256    }, // Hangul Jamo (0x1100 - 0x11FF)
    { 0x02E80, 352    }, // CJK & Kangxi Radicals (0x2E80 - 0x2FDF)
    { 0x02FF0, 464    }, // Ideograph Descriptions, CJK Symbols, Hiragana, Katakana, Bopomofo, Hangul Compatibility Jamo, Kanbun, & Bopomofo Ext (0x2FF0 - 0x31BF)
    { 0x03200, 29392  }, // Enclosed CJK, CJK Ideographs (Uni Han & Ext A), & Yi (0x3200 - 0xA4CF)
    { 0x0AC00, 11183  }, // Hangul Syllables (0xAC00 - 0xD7AF)
    { 0x0F900, 352    }, // CJK Compatibility Ideographs (0xF900 - 0xFA5F)
    { 0x0FE30, 32     }, // CJK Compatibility From (0xFE30 - 0xFE4F)
    { 0x0FF00, 240    }, // Half/Full Width Form (0xFF00 - 0xFFEF)
    { 0x20000, 0xA6D7 }, // CJK Ideograph Exntension B
    { 0x2F800, 0x021E }  // CJK Compatibility Ideographs (0x2F800 - 0x2FA1D)

    // ** TODO need to add character ranges for kCFCharacterSetPunctuation.
};

static const size_t numPostCharacterSetEntries = sizeof(postCharacterSet) / sizeof(CharacterRange);

struct CharacterRangePredicate
{
    inline CharacterRangePredicate() {}
    inline ~CharacterRangePredicate() {}
    bool operator()(const CharacterRange& r, UChar32 const c)
    {
        return (static_cast<unsigned long>(c) < ( r.firstCharacter + r.numCharacters ));
    }

    bool operator()(UChar32 const c, const CharacterRange& r)
    {
        return (static_cast<unsigned long>(c) >= ( r.firstCharacter + r.numCharacters ));
    }

    bool operator()(const CharacterRange& r1, const CharacterRange& r2)
    {
        
        ASSERT(((r1.firstCharacter < r2.firstCharacter) &&
                ((r1.firstCharacter + r1.numCharacters) < r2.firstCharacter)) ||
               ((r2.firstCharacter < r1.firstCharacter) &&
                ((r2.firstCharacter + r2.numCharacters) < r1.firstCharacter)));
        return r1.firstCharacter < r2.firstCharacter;
    }

    inline CharacterRangePredicate(const CharacterRangePredicate&) {}
    inline CharacterRangePredicate& operator=(const CharacterRangePredicate&) { return *this; }
};

}


bool isCharacterSmartReplaceExempt(UChar32 c, bool isPreviousCharacter)
{
    const CharacterRange* ranges = isPreviousCharacter ? preCharacterSet : postCharacterSet;
    const size_t numRanges = isPreviousCharacter ? numPreCharacterSetEntries : numPostCharacterSetEntries;

    const CharacterRange* findResult = std::lower_bound<const CharacterRange*, UChar32, CharacterRangePredicate>(postCharacterSet, ranges + numRanges, c, CharacterRangePredicate());
    return  (findResult < (ranges + numRanges))
         && (static_cast<unsigned long>(c) >= findResult->firstCharacter)
         && (static_cast<unsigned long>(c) < (findResult->firstCharacter + findResult->numCharacters));
}

}
