/*
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

#ifndef WTF_GTK_UNICODE_H
#define WTF_GTK_UNICODE_H

#include <WebKitApollo/WebKit.h>
#include <glib.h>
#include <string.h>
#include "Assertions.h"


typedef guint16 UChar;
typedef guint32 UChar32;

#define U_MASK(x) ((UChar32)1<<(x))

template <typename T>
inline UChar U16_LEAD(T supplementary)
{
    return static_cast<UChar>((supplementary>>10) + 0xd7c0);
}

template <typename T>
inline UChar U16_TRAIL(T supplementary)
{
    return static_cast<UChar>((supplementary&0x3ff)|0xdc00);
}

template <typename T>
inline bool U16_IS_LEAD(T c)
{
    return (c&0xfffffc00)==0xd800;
} 

template <typename T>
inline bool U16_IS_TRAIL(T c)
{
    return (c&0xfffffc00)==0xdc00;
}

static const UChar32 U16_SURROGATE_OFFSET = (0xd800<<10UL)+0xdc00-0x10000;

template <typename T, typename U>
inline UChar32 U16_GET_SUPPLEMENTARY(T lead, U trail)
{
    return ((UChar32)lead<<10UL)+(UChar32)trail-U16_SURROGATE_OFFSET;
}

template <typename T>
inline void __U16_NEXT(const UChar* s, T& i, size_t length, UChar32& c)
{
    c=s[i++];
    if(U16_IS_LEAD(c)) {
        uint16_t __c2;
        if(i<length && U16_IS_TRAIL(__c2=s[i])) {
            ++i;
            c=U16_GET_SUPPLEMENTARY(c, __c2);
        }
    }
}

#define U16_NEXT(s,i,length,c) __U16_NEXT((s),(i),(length),(c));

template <typename T>
inline void __U16_PREV(const UChar* s, size_t start, T& i, UChar32& c)
{
    (c)=(s)[--(i)];
    if(U16_IS_TRAIL(c)) {
        uint16_t __c2;
        if((i)>(start) && U16_IS_LEAD(__c2=(s)[(i)-1])) {
            --(i);
            (c)=U16_GET_SUPPLEMENTARY(__c2, (c));
        }
    }
}

#define U16_PREV(s, start, i, c) __U16_PREV((s),(start),(i),(c));


template <typename T>
inline bool U_IS_SURROGATE(T c)
{
    return (c&0xfffff800)==0xd800;
}

template <typename T>
inline bool U16_IS_SINGLE(T c)
{
    return !U_IS_SURROGATE(c);
}
template <typename T>
inline bool U16_IS_SURROGATE(T c)
{
    return U_IS_SURROGATE(c);
}

template <typename T>
inline bool U16_IS_SURROGATE_LEAD(T c)
{
    return (c&0x400)==0;
}


namespace WebKitApollo {
	extern WebKitAPIHostFunctions* g_HostFunctions;
}

namespace WTF {
namespace Unicode
{

    // Describes a sequence of consecutive charaters
    struct Range {
            UChar32 code;	// Starting char
            long count;		// Length of the sequence
    };

    // Describes a mapping of a sequence of consecutive characters to another sequence

    // Mappings (see UnicodeGtk.cpp)
    extern const Range NonPrintableRange[];
    extern const Range DecompositionFontRange[];
    extern const Range DecompositionCompatRange[];
    extern const Range FormatRange[];
    extern const Range SeparatorRange[];
    extern const Range MarkNonSpacingRange[];
    extern const Range MarkSpacingCombiningRange[];
    extern const Range MarkEnclosingRange[];
    extern const Range LetterModifierRange[];
    extern const Range NumberLetterRange[];
    extern const Range PunctuationConnectorRange[];
    extern const UChar32 TitleCaseMap[][2]; 
    extern const UChar32 DigitsArray[]; 

    // Length of each mapping
    const int NON_PRINT_CHAR_RLEN = 12;
    const int FONT_DEC_RLEN = 13;
    const int COMPAT_DEC_RLEN = 71;
    const int FORMAT_CHAR_RLEN = 11;
    const int SEPARATOR_CHAR_RLEN = 8;
    const int MIRROR_CHAR_RLEN = 90;
    const int TITLE_CASE_MAP_LEN = 8;
    const int DIGITS_ARR_LEN = 23;
    const int MARK_NON_SPACING_RLEN = 121;
    const int MARK_SPACING_COMBINING_RLEN = 66;
    const int MARK_ENCLOSING_RLEN = 4;
    const int LETTER_MODIFIER_RLEN = 29;
    const int NUMBER_LETTER_RLEN = 5 ;
    const int PUNCTUATION_CONNECTOR_RLEN = 6;


    // Original ICU Unicode general category types
    enum ICUCharCategory
    {
        CAT_UNASSIGNED              = 0,
        CAT_GENERAL_OTHER_TYPES     = 0,
        CAT_UPPERCASE_LETTER        = 1,
        CAT_LOWERCASE_LETTER        = 2,
        CAT_TITLECASE_LETTER        = 3,
        CAT_MODIFIER_LETTER         = 4,
        CAT_OTHER_LETTER            = 5,
        CAT_NON_SPACING_MARK        = 6,
        CAT_ENCLOSING_MARK          = 7,
        CAT_COMBINING_SPACING_MARK  = 8,
        CAT_DECIMAL_DIGIT_NUMBER    = 9,
        CAT_LETTER_NUMBER           = 10,
        CAT_OTHER_NUMBER            = 11,
        CAT_SPACE_SEPARATOR         = 12,
        CAT_LINE_SEPARATOR          = 13,
        CAT_PARAGRAPH_SEPARATOR     = 14,
        CAT_CONTROL_CHAR            = 15,
        CAT_FORMAT_CHAR             = 16,
        CAT_PRIVATE_USE_CHAR        = 17,
        CAT_SURROGATE               = 18,
        CAT_DASH_PUNCTUATION        = 19,
        CAT_START_PUNCTUATION       = 20,
        CAT_END_PUNCTUATION         = 21,
        CAT_CONNECTOR_PUNCTUATION   = 22,
        CAT_OTHER_PUNCTUATION       = 23,
        CAT_MATH_SYMBOL             = 24,
        CAT_CURRENCY_SYMBOL         = 25,
        CAT_MODIFIER_SYMBOL         = 26,
        CAT_OTHER_SYMBOL            = 27,
        CAT_INITIAL_PUNCTUATION     = 28,
        CAT_FINAL_PUNCTUATION       = 29,
        CAT_CHAR_CATEGORY_COUNT
   };

   enum CharCategory {
        Unknown_CharCategory =  0,

        Other_Control = U_MASK(U_MASK(G_UNICODE_CONTROL)),
        Other_Format = U_MASK(G_UNICODE_FORMAT),
        Other_NotAssigned = U_MASK(G_UNICODE_UNASSIGNED),
        Other_PrivateUse = U_MASK(G_UNICODE_PRIVATE_USE),
        Other_Surrogate = U_MASK(G_UNICODE_SURROGATE),

        Letter_Lowercase = U_MASK(G_UNICODE_LOWERCASE_LETTER),
        Letter_Modifier = U_MASK(G_UNICODE_MODIFIER_LETTER),
        Letter_Other = U_MASK(G_UNICODE_OTHER_LETTER),
        Letter_Titlecase = U_MASK(G_UNICODE_TITLECASE_LETTER),
        Letter_Uppercase = U_MASK(G_UNICODE_UPPERCASE_LETTER),
        Mark_SpacingCombining = U_MASK(G_UNICODE_COMBINING_MARK),
        Mark_Enclosing = U_MASK(G_UNICODE_ENCLOSING_MARK),
        Mark_NonSpacing = U_MASK(G_UNICODE_NON_SPACING_MARK),
        Number_DecimalDigit = U_MASK(G_UNICODE_DECIMAL_NUMBER),
        Number_Letter = U_MASK(G_UNICODE_LETTER_NUMBER),
        Number_Other = U_MASK(G_UNICODE_OTHER_NUMBER),
        Punctuation_Connector = U_MASK(G_UNICODE_CONNECT_PUNCTUATION),
        Punctuation_Dash = U_MASK(G_UNICODE_DASH_PUNCTUATION),
        Punctuation_Close = U_MASK(G_UNICODE_CLOSE_PUNCTUATION),
        Punctuation_FinalQuote = U_MASK(G_UNICODE_FINAL_PUNCTUATION),
        Punctuation_InitialQuote = U_MASK(G_UNICODE_INITIAL_PUNCTUATION),
        Punctuation_Other = U_MASK(G_UNICODE_OTHER_PUNCTUATION),
        Punctuation_Open = U_MASK(G_UNICODE_OPEN_PUNCTUATION),
        Symbol_Currency = U_MASK(G_UNICODE_CURRENCY_SYMBOL),
        Symbol_Modifier = U_MASK(G_UNICODE_MODIFIER_SYMBOL),
        Symbol_Math = U_MASK(G_UNICODE_MATH_SYMBOL),
        Symbol_Other = U_MASK(G_UNICODE_OTHER_SYMBOL),
        Separator_Line = U_MASK(G_UNICODE_LINE_SEPARATOR),
        Separator_Paragraph = U_MASK(G_UNICODE_PARAGRAPH_SEPARATOR),
        Separator_Space = U_MASK(G_UNICODE_SPACE_SEPARATOR)

    };

    enum Direction {
        RightToLeftArabic        = 0,
        ArabicNumber             = 1,
        BlockSeparator           = 2,
        BoundaryNeutral          = 3,
        CommonNumberSeparator    = 4,
        EuropeanNumber           = 5,
        EuropeanNumberSeparator  = 6,
        EuropeanNumberTerminator = 7,
        LeftToRight              = 8,
        LeftToRightEmbedding     = 9,
        LeftToRightOverride     = 10,
        NonSpacingMark          = 11,
        OtherNeutral            = 12,
        PopDirectionalFormat    = 13,
        RightToLeft             = 14,
        RightToLeftEmbedding    = 15,
        RightToLeftOverride     = 16,
        SegmentSeparator        = 17,
        WhiteSpaceNeutral       = 18
    };

    enum DecompositionType {
        DecompositionUnknown,
        DecompositionCompat,
        DecompositionFont
    };

        // Combining classes
    const uint8_t unknownCombiningClass = 0;
    const uint8_t hiraganaKatakanaVoicingMarksCombiningClass = 8;

    // True if the character falls within any of the char sequences in range[]
    inline bool isInRange(const Range range[], unsigned int len, UChar32 c)
    {
        for (unsigned int i = 0; i < len; i++)
            if (c >= range[i].code && c < range[i].code + range[i].count)
                return true;
        return false;
    }



    inline bool isSeparatorSpace(UChar32 c)
    {
        return isInRange(SeparatorRange, SEPARATOR_CHAR_RLEN, c);
    }

    inline bool isLower(UChar32 c) 
    {
        return g_unichar_islower(c);
    }


    inline bool isUpper(UChar32 c) 
    {
        return g_unichar_isupper(c);
    }

    inline bool isDigit(UChar32 c)
    {
        return g_unichar_isdigit(c);
    }

    inline bool isTitle(UChar32 c)
    {
        return g_unichar_istitle(c);
    }

    inline bool isHexaDigit(UChar32 c)
    {
        return g_unichar_isxdigit(c);
    }

    inline bool isDefined(UChar32 c)
    {
        return g_unichar_isdefined(c);
    }

    inline bool isPrintableChar(UChar32 c)
    {
        return g_unichar_isprint(c);
    }

    inline CharCategory category(UChar32 c)
    {
        return static_cast<CharCategory>(U_MASK(g_unichar_type(c)));
    }

	inline Direction direction(UChar32 c)
    {
        return static_cast<Direction>(WebKitApollo::g_HostFunctions->mapChar(CharMapping_BidirectionalClass,c));
    }
    
    inline UChar32 toLower(UChar32 c)
    {
        return g_unichar_tolower(c);
    }

    inline UChar32 toUpper(UChar32 c)
    {
        return g_unichar_toupper(c);
    }


    inline int toLower(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error)
    {
        *error = !src;
        if (*error)	return 0; 
        if (srcLength == 0) return 0;
        if (srcLength == -1) 
            // ToDo: Calculate srcLength
            ASSERT(false);

        if (srcLength == 1) 
        {
            if (result)
                result[0] = (UChar) toLower((UChar32) src[0]);
            return 1;
        }

        GError* gerror = NULL;
        gchar* InputStringUTF8 = g_utf16_to_utf8(src, srcLength, NULL, NULL, &gerror);
        if(gerror) 
        {
            ASSERT(0);
            g_error_free(gerror);
        }
        if(!InputStringUTF8)
        {
            *error = false;
            ASSERT(0);
            return 0;
        }


        gchar* OutputStringUTF8 = g_utf8_strdown(InputStringUTF8, -1);
        g_free(InputStringUTF8);
        if(!OutputStringUTF8)
        {
            ASSERT(0);
            *error = false;
            return 0;	
        }

        guint32 lengthOfOutput = g_utf8_strlen(OutputStringUTF8, -1);
        UChar* OutputStringUTF16 = g_utf8_to_utf16(OutputStringUTF8, -1, NULL, NULL, &gerror);
        g_free(OutputStringUTF8);
        if(gerror) 
        {
            ASSERT(0);
            g_error_free(gerror);
        }

        if(!OutputStringUTF16)
        {
            ASSERT(0);
            *error = true;
            return 0;	
        }

        guint32 lengthToCopy = (lengthOfOutput < resultLength) ? lengthOfOutput + 1 : resultLength;
        memcpy(result, OutputStringUTF16, sizeof(UChar) * lengthToCopy);
        g_free(OutputStringUTF16);
        return lengthToCopy;

    }

    inline int toUpper(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error)
    {
        *error = !src;
        if (*error)	return 0; 
        if (srcLength == 0) return 0;
        if (srcLength == -1) 
            // ToDo: Calculate srcLength
            ASSERT(false);

        if (srcLength == 1) 
        {
            if (result)
                result[0] = (UChar) toUpper((UChar32) src[0]);
            return 1;
        }

        GError* gerror = NULL;
        gchar* InputStringUTF8 = g_utf16_to_utf8(src, srcLength, NULL, NULL, &gerror);
        if(gerror) 
        { 
            ASSERT(0);
            g_error_free(gerror);
        }
        if(!InputStringUTF8)
        {
            ASSERT(0);
            *error = false;
            return 0;
        }


        gchar* OutputStringUTF8 = g_utf8_strup(InputStringUTF8, -1);
        g_free(InputStringUTF8);
        if(!OutputStringUTF8)
        {
            ASSERT(0);
            *error = false;
            return 0;	
        }


        guint32 lengthOfOutput = g_utf8_strlen(OutputStringUTF8, -1);
        UChar* OutputStringUTF16 = g_utf8_to_utf16(OutputStringUTF8, -1, NULL, NULL, &gerror);
        g_free(OutputStringUTF8);
        if(gerror) 
        { 
            ASSERT(0);
            g_error_free(gerror);
        }
        if(!OutputStringUTF16)
        {
            *error = false;
            return 0;	
        }

        guint32 lengthToCopy = (lengthOfOutput < resultLength) ? lengthOfOutput + 1 : resultLength;
        memcpy(result, OutputStringUTF16, sizeof(UChar) * lengthToCopy);
        g_free(OutputStringUTF16);
        return lengthToCopy;
    }

    inline UChar32 foldCase(UChar32 c)
    {
        return toLower(c); 
    }

    inline int foldCase(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error)
    {
        return toLower(result, resultLength, src, srcLength, error);
    }

    inline int umemcasecmp(const UChar* a, const UChar* b, int len)
    {
        GError* error = NULL;
        gchar* aUTF8 = g_utf16_to_utf8(a, len, NULL, NULL, &error);

        if(error)
        {
            ASSERT(0);	
            g_error_free(error);
        }
        gchar *aCaseFoldedUTF8;
        if(aUTF8)
        {
            aCaseFoldedUTF8 = g_utf8_casefold(aUTF8, -1);
            g_free(aUTF8);
        }
        else
            ASSERT(0);

        gchar* bUTF8 = g_utf16_to_utf8(b, len, NULL, NULL, &error);
        if(error)
        {
            ASSERT(0);	
            g_error_free(error);
        }
        gchar *bCaseFoldedUTF8;
        if(bUTF8)
        {
            bCaseFoldedUTF8 = g_utf8_casefold(bUTF8, -1);
            g_free(bUTF8);
        }
        else
            ASSERT(0);

        if(!aCaseFoldedUTF8)
        {
            ASSERT(0);
            if(!bCaseFoldedUTF8)
                return 0;
            else
            {
                g_free(bCaseFoldedUTF8);
                return -1;
            }
        }
        else 
        {
            ////aCaseFoldedUTF8 but no bCaseFoldedUTF8
            if(!bCaseFoldedUTF8)
            {
                g_free(aCaseFoldedUTF8);
                ASSERT(0);
                return 1;
            }
        }


        //both aCaseFoldedUTF8 and bCaseFoldedUTF8 exist here
        ASSERT(aCaseFoldedUTF8 && bCaseFoldedUTF8);
        guint32 aLengthUTF8 = strnlen(aCaseFoldedUTF8, -1);
        guint32 bLengthUTF8 = strnlen(bCaseFoldedUTF8, -1);

        guint32 length = (aLengthUTF8 < bLengthUTF8) ? aLengthUTF8 : bLengthUTF8;

        int result = strncmp(aCaseFoldedUTF8, bCaseFoldedUTF8, length);
        g_free(aCaseFoldedUTF8);
        g_free(bCaseFoldedUTF8);
        if ((result == 0) && (aLengthUTF8 < bLengthUTF8))
            result = -1;
        else if(aLengthUTF8 > bLengthUTF8)
            result = 1;

        return result;
    }

    inline DecompositionType decompositionType(UChar32 c)
    {
        // Only supports DecompositionFontRange and DecompositionCompatRange
        if (isInRange(DecompositionFontRange, FONT_DEC_RLEN, c))
            return DecompositionFont;

        if (isInRange(DecompositionCompatRange, COMPAT_DEC_RLEN, c))
            return DecompositionCompat;

        return DecompositionUnknown;
    }
    
    inline bool hasLineBreakingPropertyComplexContext(UChar32 /* c */)
    {
        // rhu - implement ?
        return false;
    }

    inline bool hasLineBreakingPropertyComplexContextOrIdeographic(UChar32 /* c */)
    {
        // rhu implement ?
        return false;
    }    

    inline UChar32 mirroredChar(UChar32 c)
    {
        UChar32 mc;
        if(g_unichar_get_mirror_char(c, &mc))
            return mc;
        return c;
    }

    inline uint8_t combiningClass(UChar32 c)
    {
        // Only supports hiraganaKatakanaVoicingMarksCombiningClass class
        if (c == 0x3099 || c == 0x309A)
            return hiraganaKatakanaVoicingMarksCombiningClass;	
        return unknownCombiningClass;
    }

    inline UChar32 toTitleCase(UChar32 c)
    {
        return g_unichar_totitle(c);
    }

    inline bool isPunct(UChar32 c)
    {
        return g_unichar_ispunct(c);
    }


} //WTF::Unicode

} //WTF
#endif //WTF_GTK_UNICODE_H






