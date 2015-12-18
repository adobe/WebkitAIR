/*
 * Copyright (C) 2009 Adobe Systems, Inc.  All rights reserved.
 */
#ifndef UNICODE_WIN_ENUMS_H
#define UNICODE_WIN_ENUMS_H

// CTS enumeration of the bidi classes.
#include "CTS_AGL_Properties_Air.h"


#define _BIT(x) ((uint32_t)1<<(x))

namespace WTF {
  namespace Unicode {

	// Webkit/ICU enumeration of bidi classes (maps to CTS constants)
	enum Direction {
		LeftToRight = CTS_AGL_Bc_L,
		RightToLeft = CTS_AGL_Bc_R,
		EuropeanNumber = CTS_AGL_Bc_EN,
		EuropeanNumberSeparator = CTS_AGL_Bc_ES,
		EuropeanNumberTerminator = CTS_AGL_Bc_ET,
		ArabicNumber = CTS_AGL_Bc_AN,
		CommonNumberSeparator = CTS_AGL_Bc_CS,
		BlockSeparator = CTS_AGL_Bc_B,
		SegmentSeparator = CTS_AGL_Bc_S,
		WhiteSpaceNeutral = CTS_AGL_Bc_WS,
		OtherNeutral = CTS_AGL_Bc_ON,
		LeftToRightEmbedding = CTS_AGL_Bc_LRE,
		LeftToRightOverride = CTS_AGL_Bc_LRO,
		RightToLeftArabic = CTS_AGL_Bc_AL,
		RightToLeftEmbedding = CTS_AGL_Bc_RLE,
		RightToLeftOverride = CTS_AGL_Bc_RLO,
		PopDirectionalFormat = CTS_AGL_Bc_PDF,
		NonSpacingMark = CTS_AGL_Bc_NSM,
		BoundaryNeutral = CTS_AGL_Bc_BN
	};


	// Webkit/ICU enumeration of general character categories.
	// Only one bit is expected to be set in each constant.
	// Use CTSToWebkitCharCategory() to map from CTS constants
	// Based on enum in ICU's uchar.h
	enum CharCategory {
		NoCategory =  0,
		Other_NotAssigned = _BIT(0),
		Letter_Uppercase = _BIT(1),
		Letter_Lowercase = _BIT(2),
		Letter_Titlecase = _BIT(3),
		Letter_Modifier = _BIT(4),
		Letter_Other = _BIT(5),

		Mark_NonSpacing = _BIT(6),
		Mark_Enclosing = _BIT(7),
		Mark_SpacingCombining = _BIT(8),

		Number_DecimalDigit = _BIT(9),
		Number_Letter = _BIT(10),
		Number_Other = _BIT(11),

		Separator_Space = _BIT(12),
		Separator_Line = _BIT(13),
		Separator_Paragraph = _BIT(14),

		Other_Control = _BIT(15),
		Other_Format = _BIT(16),
		Other_PrivateUse = _BIT(17),
		Other_Surrogate = _BIT(18),

		Punctuation_Dash = _BIT(19),
		Punctuation_Open = _BIT(20),
		Punctuation_Close = _BIT(21),
		Punctuation_Connector = _BIT(22),
		Punctuation_Other = _BIT(23),

		Symbol_Math = _BIT(24),
		Symbol_Currency = _BIT(25),
		Symbol_Modifier = _BIT(26),
		Symbol_Other = _BIT(27),

		Punctuation_InitialQuote = _BIT(28),
		Punctuation_FinalQuote = _BIT(29)
	};


	// Webkit/ICU DecompositionTypes
	enum DecompositionType {
		DecompositionNone,
		DecompositionCanonical,
		DecompositionCompat,
		DecompositionCircle,
		DecompositionFinal,
		DecompositionFont,
		DecompositionFraction,
		DecompositionInitial,
		DecompositionIsolated,
		DecompositionMedial,
		DecompositionNarrow,
		DecompositionNoBreak,
		DecompositionSmall,
		DecompositionSquare,
		DecompositionSub,
		DecompositionSuper,
		DecompositionVertical,
		DecompositionWide,

		DecompositionUnknown
	};

	// Webkit/ICU Combining Class
	enum CombinigClass {
		unknownCombiningClass = 0,
		hiraganaKatakanaVoicingMarksCombiningClass = 8
	};

}
}

#endif