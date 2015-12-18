#include "UnicodeWin.h"

namespace WTF {
  namespace Unicode {

	CharCategory CTSToWebkitCharCategory(CTS_AGL_Gc category)
	{
		switch (CTS_AGL_Gc_META & category)
		{
			case CTS_ALG_Gc_L:	// Letter
				switch (category)
				{
					case CTS_AGL_Gc_Ll: return Letter_Lowercase;
					case CTS_AGL_Gc_Lm: return Letter_Modifier;
					case CTS_AGL_Gc_Lo: return Letter_Other;
					case CTS_AGL_Gc_Lt: return Letter_Titlecase;
					case CTS_AGL_Gc_Lu: return Letter_Uppercase;
				}
			break;

			case CTS_AGL_Gc_N:	// Number
				switch (category)
				{
					case CTS_AGL_Gc_Nd: return Number_DecimalDigit;
					case CTS_AGL_Gc_Nl: return Number_Letter;
					case CTS_AGL_Gc_No: return Number_Other;
				}
			break;

			case CTS_AGL_Gc_Z:	// Separator
				switch (category)
				{
 					case CTS_AGL_Gc_Zl: return Separator_Line;
 					case CTS_AGL_Gc_Zp: return Separator_Paragraph;
 					case CTS_AGL_Gc_Zs: return Separator_Space;	
				}
			break;

			case CTS_AGL_Gc_P:	// Punctuation
				switch (category)
				{
 					case CTS_AGL_Gc_Pc: return Punctuation_Connector;
 					case CTS_AGL_Gc_Pd: return Punctuation_Dash;
 					case CTS_AGL_Gc_Pe: return Punctuation_Close;
 					case CTS_AGL_Gc_Pf: return Punctuation_FinalQuote;
 					case CTS_AGL_Gc_Pi: return Punctuation_InitialQuote;
 					case CTS_AGL_Gc_Po: return Punctuation_Other;
 					case CTS_AGL_Gc_Ps: return Punctuation_Open;
				}
			break;

			case CTS_AGL_Gc_S:	// Symbol
				switch (category)
				{
					case CTS_AGL_Gc_Sc: return Symbol_Currency;
 					case CTS_AGL_Gc_Sk: return Symbol_Modifier;
 					case CTS_AGL_Gc_Sm: return Symbol_Math;
 					case CTS_AGL_Gc_So: return Symbol_Other;	
				}
			break;

			case CTS_AGL_Gc_M:	// Mark
				switch (category)
				{
					case CTS_AGL_Gc_Mc: return Mark_SpacingCombining;
					case CTS_AGL_Gc_Me: return Mark_Enclosing;
					case CTS_AGL_Gc_Mn: return Mark_NonSpacing;
				}
			break;

			case CTS_AGL_Gc_C:	// Other
				switch (category)
				{
					case CTS_AGL_Gc_Cc:  return Other_Control;
					case CTS_AGL_Gc_Cf:	 return Other_Format; 
					case CTS_AGL_Gc_Cn:	 return Other_Control;
					case CTS_AGL_Gc_Co:	 return Other_PrivateUse;
					case CTS_AGL_Gc_Cs:	 return Other_Surrogate;
				}
			break;
		}
		return NoCategory;
	}

  }
}
    
    
	

    

    


	
