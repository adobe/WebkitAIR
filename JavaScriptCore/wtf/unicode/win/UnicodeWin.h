/*
 * Copyright (C) 2009 Adobe Systems, Inc.  All rights reserved.
 */
#ifndef UNICODE_WIN_H
#define UNICODE_WIN_H

#if PLATFORM(APOLLO)

#include <WebKitApollo/WebKit.h>
#include "UnicodeWinDefs.h"
#include "UnicodeWinEnums.h"

typedef wchar_t UChar;		// From ICU: Define UChar to be wchar_t if that is 16 bits wide; always assumed to be unsigned.
typedef int32_t UChar32;	// From ICU: UChar32 is a signed 32-bit integer (same as int32_t).

#define UNI_WIN_LOCALE LOCALE_SYSTEM_DEFAULT_WTF	// LOCALE_USER_DEFAULT?

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

#define U16_NEXT(s,i,length,c) __U16_NEXT((s),(i),(length),(c));
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
  namespace Unicode {
	
	//
	// Unicode functions
	//

	#define MAP_CHAR(mapping, c)				( WebKitApollo::g_HostFunctions->mapChar(mapping, c) )
	#define IS_GENERAL_CATEGORY(category, c)	( category == (CTS_AGL_Gc_META & MAP_CHAR(CharMapping_GeneralCatagory, c)) )
	#define IS_CATEGORY(category, c)			( category == (MAP_CHAR(CharMapping_GeneralCatagory, c)) )
	CharCategory CTSToWebkitCharCategory(CTS_AGL_Gc category);

	// Character conversion
	inline UChar32 toLower(UChar32 c)			{ return MAP_CHAR(CharMapping_LowerCase, c);	}
	inline UChar32 toUpper(UChar32 c)			{ return MAP_CHAR(CharMapping_UpperCase, c);	}
	inline UChar32 toTitleCase(UChar32 c)		{ return MAP_CHAR(CharMapping_TitleCase, c);	}
	inline UChar32 foldCase(UChar32 c)			{ return MAP_CHAR(CharMapping_FoldCase, c);		}
	inline UChar32 mirroredChar(UChar32 c)		{ return MAP_CHAR(CharMapping_MirrorChar, c);	}
	inline int digitValue(UChar32 c)			{ return MAP_CHAR(CharMapping_DigitValue, c);	}

	// Character properties
	inline bool isDigit(UChar32 c)				{ return IS_CATEGORY(CTS_AGL_Gc_Nd, c);			}		
	inline bool isPunct(UChar32 c)				{ return IS_GENERAL_CATEGORY(CTS_AGL_Gc_P, c);	}
	inline bool isLower(UChar32 c)				{ return IS_CATEGORY(CTS_AGL_Gc_Ll, c);			}
	inline bool isUpper(UChar32 c)				{ return IS_CATEGORY(CTS_AGL_Gc_Lu, c);			}
	inline bool isFormatChar(UChar32 c)			{ return IS_CATEGORY(CTS_AGL_Gc_Cf, c);			}
	inline bool isSeparatorSpace(UChar32 c)		{ return IS_CATEGORY(CTS_AGL_Gc_Zs, c);			}
	inline bool isPrintableChar(UChar32 c)		{ return !IS_GENERAL_CATEGORY(CTS_AGL_Gc_C, c); }
	inline bool isArabicChar(UChar32 c)			{ return c >= 0x0600 && c <= 0x06FF;			}	// Arabic Unicode Block (not script class)

	// Character classification
	inline Direction direction(UChar32 c)		{ return (Direction) MAP_CHAR(CharMapping_BidirectionalClass, c);						}
	inline CharCategory category(UChar32 c)		{ return CTSToWebkitCharCategory((CTS_AGL_Gc)MAP_CHAR(CharMapping_GeneralCatagory, c)); }
    inline uint8_t combiningClass(UChar32 c)	{ return (c == 0x3099 || c == 0x309A) ? hiraganaKatakanaVoicingMarksCombiningClass : unknownCombiningClass; }
	inline DecompositionType decompositionType(UChar32 c)	{ return MAP_CHAR(CharMapping_FontOrCompatDecompositionType, c) ? DecompositionCompat : DecompositionUnknown;}

	// Implementation notes
	// --------------------
	// decompositionType: Webkit only needs to know if a char is Font or Compat (see Document.cpp::isValidNameStart), so to save space in CTS we don't support
	// other types and we actually do not distinguish between Font or Compat. (We return Compat when we have a Font type).
	// combiningClass: Webkit only cares about hiraganaKatakanaVoicingMarks, so to save space we return unknown for everything else.

 
    inline int mapString(StringMapping mapping, UChar* dest, int destCapacity, const UChar* src, int srcLength, bool* error)
	{
		int32_t nError = 0;
		int nResult = 0;

		ASSERT( sizeof(UChar) == sizeof(uint16_t) );

		// Make sure to run ApolloUnit *html.webkit::JavaScriptCoreTestCase after modifying this function
		if ( src == NULL || srcLength == 0 || destCapacity == 0 || dest == NULL )
		{
			nError = 1;
		}
		else
		{
			ASSERT( srcLength != -1 );			
			// ToDo: We should calculate srcLength ourselves in this case, although this feature doesn't seem to be actually used by Webkit.

			nResult = WebKitApollo::g_HostFunctions->mapString( mapping, (uint16_t*) dest, destCapacity, (uint16_t*) src,  srcLength, &nError);
		}
		if (error != NULL)
		{
			*error = nError != 0;
		}
		return nResult;
	}


    inline int foldCase(UChar* dest, int destCapacity, const UChar* src, int srcLength, bool* error)
	{
		return mapString(StringMapping_FoldCase, dest, destCapacity, src, srcLength, error);
	}

    inline int toLower(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error)
    {
		return mapString(StringMapping_LowerCase, result, resultLength, src, srcLength, error);
    }

    inline int toUpper(UChar* dest, int destCapacity, const UChar* src, int srcLength, bool* error)
    {
		return mapString(StringMapping_UpperCase, dest, destCapacity, src, srcLength, error);
    }

	inline int umemcasecmp(const UChar* a, const UChar* b, int len)
    {	// Docs say it's ok to substract 2 to make compatible with strcmp (and umemcasecmp) return values.
		return CompareStringW( UNI_WIN_LOCALE, NORM_IGNORECASE, (LPCWSTR) a, len, (LPCWSTR) b, len) - 2;
    }

	inline bool hasLineBreakingPropertyComplexContext(UChar32 c)
    {
		return CTS_AGL_Lb_SA == MAP_CHAR(CharMapping_LineBreakClass, c);		// ICU's u_getIntPropertyValue(c, UCHAR_LINE_BREAK) == U_LB_COMPLEX_CONTEXT;
    }

    inline bool hasLineBreakingPropertyComplexContextOrIdeographic(UChar32 c )
    {
		uint32_t prop = MAP_CHAR(CharMapping_LineBreakClass, c);
        return prop == CTS_AGL_Lb_SA || prop == CTS_AGL_Lb_ID;					// ICU's prop == U_LB_COMPLEX_CONTEXT || prop == U_LB_IDEOGRAPHIC 
    }

  }
}

#endif //PLATFORM(APOLLO)

#endif
