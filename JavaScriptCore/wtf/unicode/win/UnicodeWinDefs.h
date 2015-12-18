//Forward declare unicode related windows functions, so we don't have to include windows.h here.
extern "C"
{
	typedef unsigned short  WORD;
    typedef unsigned long   DWORD;
    typedef const wchar_t	*LPCWSTR;
    typedef wchar_t			*LPWSTR;
	typedef DWORD			LCID;  
	typedef WORD			*LPWORD;

    __declspec(dllimport) int __stdcall FoldStringW(DWORD dwMapFlags, LPCWSTR lpSrcStr, int cchSrc, LPWSTR lpDestStr, int cchDest);
	__declspec(dllimport) int __stdcall CompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);
	__declspec(dllimport) int __stdcall GetStringTypeW(DWORD dwInfoType, LPCWSTR lpSrcStr, int cchSrc, LPWORD lpCharType);
	__declspec(dllimport) int __stdcall LCMapStringW(LCID Locale, DWORD dwMapFlags, LPCWSTR lpSrcStr, int cchSrc, LPWSTR lpDestStr, int cchDest);
}

//
//  Locale Dependent Mapping Flags.
//
#define LCMAP_LOWERCASE           0x00000100  // lower case letters
#define LCMAP_UPPERCASE           0x00000200  // upper case letters

//
//  Locale Independent Mapping Flags.
//
#define MAP_FOLDCZONE             0x00000010  // fold compatibility zone chars
#define MAP_PRECOMPOSED           0x00000020  // convert to precomposed chars
#define MAP_COMPOSITE             0x00000040  // convert to composite chars
#define MAP_FOLDDIGITS            0x00000080  // all digits to ASCII 0-9

// CompareStringW constants
#define NORM_IGNORECASE           0x00000001  // ignore case
#define CSTR_LESS_THAN            1           // string 1 less than string 2
#define CSTR_EQUAL                2           // string 1 equal to string 2
#define CSTR_GREATER_THAN         3           // string 1 greater than string 2


// LOCALE_SYSTEM_DEFAULT

#define LANG_NEUTRAL                     0x00
#define SORT_DEFAULT_WTF                 0x00    // sorting default
#define SUBLANG_NEUTRAL                  0x00    // language neutral
#define SUBLANG_DEFAULT                  0x01    // user default
#define SUBLANG_SYS_DEFAULT              0x02    // system default

#define MAKELCID(lgid, srtid)  ((DWORD)((((DWORD)((WORD  )(srtid))) << 16) |  \
                                         ((DWORD)((WORD  )(lgid)))))

#define MAKELANGID_WTF(p, s)       ((((WORD  )(s)) << 10) | (WORD  )(p))

#define LANG_SYSTEM_DEFAULT_WTF    (MAKELANGID_WTF(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT))
#define LANG_USER_DEFAULT_WTF      (MAKELANGID_WTF(LANG_NEUTRAL, SUBLANG_DEFAULT))

#define LOCALE_USER_DEFAULT_WTF    (MAKELCID(LANG_USER_DEFAULT_WTF, SORT_DEFAULT_WTF))
#define LOCALE_SYSTEM_DEFAULT_WTF  (MAKELCID(LANG_SYSTEM_DEFAULT_WTF, SORT_DEFAULT_WTF))


//
//  Character Type Flags.
//
#define CT_CTYPE1                 0x00000001  // ctype 1 information
#define CT_CTYPE2                 0x00000002  // ctype 2 information
#define CT_CTYPE3                 0x00000004  // ctype 3 information

//
//  CType 1 Flag Bits.
//
#define C1_UPPER                  0x0001      // upper case
#define C1_LOWER                  0x0002      // lower case
#define C1_DIGIT                  0x0004      // decimal digits
#define C1_SPACE                  0x0008      // spacing characters
#define C1_PUNCT                  0x0010      // punctuation characters
#define C1_CNTRL                  0x0020      // control characters
#define C1_BLANK                  0x0040      // blank characters
#define C1_XDIGIT                 0x0080      // other digits
#define C1_ALPHA                  0x0100      // any linguistic character
#define C1_DEFINED                0x0200      // defined character

//
//  CType 2 Flag Bits.
//
#define C2_LEFTTORIGHT            0x0001      // left to right
#define C2_RIGHTTOLEFT            0x0002      // right to left

#define C2_EUROPENUMBER           0x0003      // European number, digit
#define C2_EUROPESEPARATOR        0x0004      // European numeric separator
#define C2_EUROPETERMINATOR       0x0005      // European numeric terminator
#define C2_ARABICNUMBER           0x0006      // Arabic number
#define C2_COMMONSEPARATOR        0x0007      // common numeric separator

#define C2_BLOCKSEPARATOR         0x0008      // block separator
#define C2_SEGMENTSEPARATOR       0x0009      // segment separator
#define C2_WHITESPACE             0x000A      // white space
#define C2_OTHERNEUTRAL           0x000B      // other neutrals

#define C2_NOTAPPLICABLE          0x0000      // no implicit directionality

//
//  CType 3 Flag Bits.
//
#define C3_NONSPACING             0x0001      // nonspacing character
#define C3_DIACRITIC              0x0002      // diacritic mark
#define C3_VOWELMARK              0x0004      // vowel mark
#define C3_SYMBOL                 0x0008      // symbols

#define C3_KATAKANA               0x0010      // katakana character
#define C3_HIRAGANA               0x0020      // hiragana character
#define C3_HALFWIDTH              0x0040      // half width character
#define C3_FULLWIDTH              0x0080      // full width character
#define C3_IDEOGRAPH              0x0100      // ideographic character
#define C3_KASHIDA                0x0200      // Arabic kashida character
#define C3_LEXICAL                0x0400      // lexical character
#define C3_HIGHSURROGATE          0x0800      // high surrogate code unit
#define C3_LOWSURROGATE           0x1000      // low surrogate code unit

#define C3_ALPHA                  0x8000      // any linguistic char (C1_ALPHA)

#define C3_NOTAPPLICABLE          0x0000      // ctype 3 is not applicable


// Additional GetStringTypeW constants
#define C2X_RIGHTTOLEFTARABIC	0x000C
#define C2X_NONSPACINGMARK		0x000D
#define C2X_BOUNDARYNEUTRAL		0x000E

#define C2X_LEFTTORIGHTEMBEDDING	LEFT_TO_RIGHT_EMBEDDING_CHAR
#define C2X_RIGHTTOLEFTEMBEDDING	RIGHT_TO_LEFT_EMBEDDING_CHAR
#define C2X_POPDIRECTIONALFORMAT	POP_DIRECTIONAL_FORMAT_CHAR
#define C2X_LEFTTORIGHTOVERRIDE		LEFT_TO_RIGHT_OVERRIDE_CHAR
#define C2X_RIGHTTOLEFTOVERRIDE		RIGHT_TO_LEFT_OVERRIDE_CHAR

#define LEFT_TO_RIGHT_EMBEDDING_CHAR	0x202A
#define RIGHT_TO_LEFT_EMBEDDING_CHAR	0x202B
#define POP_DIRECTIONAL_FORMAT_CHAR		0x202C
#define LEFT_TO_RIGHT_OVERRIDE_CHAR		0x202D
#define RIGHT_TO_LEFT_OVERRIDE_CHAR		0x202E