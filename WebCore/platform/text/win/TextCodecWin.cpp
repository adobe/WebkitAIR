/*
 * Copyright (C) 2004, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Alexey Proskuryakov <ap@nypop.com>
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

#include "config.h"
#include "TextCodecWin.h"

#include "CharacterNames.h"
#include "PlatformString.h"
#include <wtf/Assertions.h>
#include "win/COMPtr.h"
#include "win/PlatformShared.h"

using std::auto_ptr;

/*
* We assume the encoding of a unicode char does not take more than MAX_BYTES_FOR_CHARACTER bytes.
* For now 6 bytes for a character seems a reasonable assumption, but we use 10 to be on the safe side.
*/
#define MAX_BYTES_FOR_CHARACTER 10

namespace Shared {
    // we will never release this as we're need a structure which would call our own
    // version of release
    static IMultiLanguage2* s_multiLang = 0;

    MultiLangCOMPtr getIMultiLanguage()
    {
		if(!s_multiLang) {
            HRESULT res = S_OK;
			if (res == S_OK) {
				res = ::CoCreateInstance(CLSID_CMultiLanguage, 0, CLSCTX_INPROC, IID_IMultiLanguage2, (void**)&s_multiLang);
				ASSERT(res == S_OK);
			}
			else
				ASSERT(false);
		}
        return MultiLangCOMPtr(s_multiLang);
    }
}

namespace WebCore {

static const char* s_alias37[] = { "IBM037", "cp037", "csIBM037", "ebcdic-cp-ca", "ebcdic-cp-nl", "ebcdic-cp-us", "ebcdic-cp-wt", NULL };
static const char* s_alias437[] = { "437", "IBM437", "cp437", "csPC8CodePage437", NULL };
static const char* s_alias500[] = { "CP500", "IBM500", "csIBM500", "ebcdic-cp-be", "ebcdic-cp-ch", NULL };
static const char* s_alias708[] = { "ASMO-708", NULL };
static const char* s_alias720[] = { "DOS-720", NULL };
static const char* s_alias737[] = { "ibm737", NULL };
static const char* s_alias775[] = { "ibm775", NULL };
static const char* s_alias850[] = { "IBM850", "cp850", "ibm850", NULL };
static const char* s_alias852[] = { "IBM852", "cp852", "ibm852", NULL };
static const char* s_alias855[] = { "IBM855", "cp855", NULL };
static const char* s_alias857[] = { "IBM857", "cp857", "ibm857", NULL };
static const char* s_alias858[] = { "CCSID00858", "CP00858", "IBM00858", "PC-Multilingual-850+euro", "cp858", NULL };
static const char* s_alias860[] = { "IBM860", "cp860", NULL };
static const char* s_alias861[] = { "IBM861", "cp861", "ibm861", NULL };
static const char* s_alias862[] = { "DOS-862", "IBM862", "cp862", NULL };
static const char* s_alias863[] = { "IBM863", "cp863", NULL };
static const char* s_alias864[] = { "IBM864", "cp864", NULL };
static const char* s_alias865[] = { "IBM865", "cp865", NULL };
static const char* s_alias866[] = { "IBM866", "cp866", NULL };
static const char* s_alias869[] = { "IBM869", "cp869", "ibm869", NULL };
static const char* s_alias870[] = { "CP870", "IBM870", "csIBM870", "ebcdic-cp-roece", "ebcdic-cp-yu", NULL };
static const char* s_alias874[] = { "DOS-874", "TIS-620", "iso-8859-11", "windows-874", NULL };
static const char* s_alias875[] = { "cp875", NULL };
static const char* s_alias932[] = { "csShiftJIS", "csWindows31J", "ms_Kanji", "shift-jis", "shift_jis", "sjis", "x-ms-cp932", "x-sjis", NULL };
static const char* s_alias936[] = { "CN-GB", "GB2312", "GB231280", "GBK", "GB_2312-80", "chinese", "csGB2312", "csGB231280", "csISO58GB231280", "gb2312", "iso-ir-58", NULL };
static const char* s_alias949[] = { "KSC5601", "KSC_5601", "csKSC56011987", "iso-ir-149", "korean", "ks-c-5601", "ks-c5601", "ks_c_5601", "ks_c_5601-1987", "ks_c_5601-1989", "ks_c_5601_1987", NULL };
static const char* s_alias950[] = { "Big5", "Big5-HKSCS", "big5", "cn-big5", "csbig5", "x-x-big5", NULL };
static const char* s_alias1026[] = { "CP1026", "IBM1026", "csIBM1026", NULL };
static const char* s_alias1047[] = { "IBM01047", NULL };
static const char* s_alias1140[] = { "CCSID01140", "CP01140", "IBM01140", "ebcdic-us-37+euro", NULL };
static const char* s_alias1141[] = { "CCSID01141", "CP01141", "IBM01141", "ebcdic-de-273+euro", NULL };
static const char* s_alias1142[] = { "CCSID01142", "CP01142", "IBM01142", "ebcdic-dk-277+euro", "ebcdic-no-277+euro", NULL };
static const char* s_alias1143[] = { "CCSID01143", "CP01143", "IBM01143", "ebcdic-fi-278+euro", "ebcdic-se-278+euro", NULL };
static const char* s_alias1144[] = { "CCSID01144", "CP01144", "IBM01144", "ebcdic-it-280+euro", NULL };
static const char* s_alias1145[] = { "CCSID01145", "CP01145", "IBM01145", "ebcdic-es-284+euro", NULL };
static const char* s_alias1146[] = { "CCSID01146", "CP01146", "IBM01146", "ebcdic-gb-285+euro", NULL };
static const char* s_alias1147[] = { "CCSID01147", "CP01147", "IBM01147", "ebcdic-fr-297+euro", NULL };
static const char* s_alias1148[] = { "CCSID01148", "CP01148", "IBM01148", "ebcdic-international-500+euro", NULL };
static const char* s_alias1149[] = { "CCSID01149", "CP01149", "IBM01149", "ebcdic-is-871+euro", NULL };
static const char* s_alias1200[] = { "UTF-16LE", "unicode", "utf-16", NULL };
static const char* s_alias1201[] = { "UTF-16BE", "unicodeFFFE", NULL };
static const char* s_alias1250[] = { "windows-1250", "x-cp1250", NULL };
static const char* s_alias1251[] = { "windows-1251", "x-cp1251", NULL };
static const char* s_alias1252[] = { "Windows-1252", "windows-1252", "x-ansi", NULL };
static const char* s_alias1253[] = { "windows-1253", NULL };
static const char* s_alias1254[] = { "Windows-1254", "windows-1254", NULL };
static const char* s_alias1255[] = { "windows-1255", NULL };
static const char* s_alias1256[] = { "cp1256", "windows-1256", NULL };
static const char* s_alias1257[] = { "windows-1257", NULL };
static const char* s_alias1258[] = { "windows-1258", NULL };
static const char* s_alias1361[] = { "Johab", NULL };
static const char* s_alias10000[] = { "macintosh", NULL };
static const char* s_alias10001[] = { "x-mac-japanese", NULL };
static const char* s_alias10002[] = { "x-mac-chinesetrad", NULL };
static const char* s_alias10003[] = { "x-mac-korean", NULL };
static const char* s_alias10004[] = { "x-mac-arabic", NULL };
static const char* s_alias10005[] = { "x-mac-hebrew", NULL };
static const char* s_alias10006[] = { "x-mac-greek", NULL };
static const char* s_alias10007[] = { "x-mac-cyrillic", NULL };
static const char* s_alias10008[] = { "x-mac-chinesesimp", NULL };
static const char* s_alias10010[] = { "x-mac-romanian", NULL };
static const char* s_alias10017[] = { "x-mac-ukrainian", NULL };
static const char* s_alias10021[] = { "x-mac-thai", NULL };
static const char* s_alias10029[] = { "x-mac-ce", NULL };
static const char* s_alias10079[] = { "x-mac-icelandic", NULL };
static const char* s_alias10081[] = { "x-mac-turkish", NULL };
static const char* s_alias10082[] = { "x-mac-croatian", NULL };
static const char* s_alias20000[] = { "x-Chinese-CNS", NULL };
static const char* s_alias20001[] = { "x-cp20001", NULL };
static const char* s_alias20002[] = { "x-Chinese-Eten", NULL };
static const char* s_alias20003[] = { "x-cp20003", NULL };
static const char* s_alias20004[] = { "x-cp20004", NULL };
static const char* s_alias20005[] = { "x-cp20005", NULL };
static const char* s_alias20105[] = { "irv", "x-IA5", NULL };
static const char* s_alias20106[] = { "DIN_66003", "German", "x-IA5-German", NULL };
static const char* s_alias20107[] = { "SEN_850200_B", "Swedish", "x-IA5-Swedish", NULL };
static const char* s_alias20108[] = { "NS_4551-1", "Norwegian", "x-IA5-Norwegian", NULL };
static const char* s_alias20127[] = { "ANSI_X3.4-1968", "ANSI_X3.4-1986", "IBM367", "ISO646-US", "ISO_646.irv:1991", "ascii", "cp367", "csASCII", "iso-ir-6", "us", "us-ascii", NULL };
static const char* s_alias20261[] = { "x-cp20261", NULL };
static const char* s_alias20269[] = { "x-cp20269", NULL };
static const char* s_alias20273[] = { "CP273", "IBM273", "csIBM273", NULL };
static const char* s_alias20277[] = { "EBCDIC-CP-DK", "EBCDIC-CP-NO", "IBM277", "csIBM277", NULL };
static const char* s_alias20278[] = { "CP278", "IBM278", "csIBM278", "ebcdic-cp-fi", "ebcdic-cp-se", NULL };
static const char* s_alias20280[] = { "CP280", "IBM280", "csIBM280", "ebcdic-cp-it", NULL };
static const char* s_alias20284[] = { "CP284", "IBM284", "csIBM284", "ebcdic-cp-es", NULL };
static const char* s_alias20285[] = { "CP285", "IBM285", "csIBM285", "ebcdic-cp-gb", NULL };
static const char* s_alias20290[] = { "EBCDIC-JP-kana", "IBM290", "cp290", "csIBM290", NULL };
static const char* s_alias20297[] = { "IBM297", "cp297", "csIBM297", "ebcdic-cp-fr", NULL };
static const char* s_alias20420[] = { "IBM420", "cp420", "csIBM420", "ebcdic-cp-ar1", NULL };
static const char* s_alias20423[] = { "IBM423", "cp423", "csIBM423", "ebcdic-cp-gr", NULL };
static const char* s_alias20424[] = { "IBM424", "cp424", "csIBM424", "ebcdic-cp-he", NULL };
static const char* s_alias20833[] = { "X-EBCDIC-KoreanExtended", "x-EBCDIC-KoreanExtended", NULL };
static const char* s_alias20838[] = { "IBM-Thai", "csIBMThai", NULL };
static const char* s_alias20866[] = { "csKOI8R", "koi", "koi8", "koi8-r", "koi8r", NULL };
static const char* s_alias20871[] = { "CP871", "IBM871", "csIBM871", "ebcdic-cp-is", NULL };
static const char* s_alias20880[] = { "EBCDIC-Cyrillic", "IBM880", "cp880", "csIBM880", NULL };
static const char* s_alias20905[] = { "CP905", "IBM905", "csIBM905", "ebcdic-cp-tr", NULL };
static const char* s_alias20924[] = { "CCSID00924", "CP00924", "IBM00924", "ebcdic-Latin9--euro", NULL };
static const char* s_alias20936[] = { "x-cp20936", NULL };
static const char* s_alias20949[] = { "x-cp20949", NULL };
static const char* s_alias21025[] = { "cp1025", NULL };
static const char* s_alias21027[] = { "x-cp21027", NULL };
static const char* s_alias21866[] = { "koi8-ru", "koi8-u", NULL };
static const char* s_alias28591[] = { "cp819", "csISOLatin1", "ibm819", "iso-8859-1", "iso-ir-100", "iso8859-1", "iso_8859-1", "iso_8859-1:1987", "l1", "latin1", NULL };
static const char* s_alias28592[] = { "csISOLatin2", "iso-8859-2", "iso-ir-101", "iso8859-2", "iso_8859-2", "iso_8859-2:1987", "l2", "latin2", NULL };
static const char* s_alias28593[] = { "ISO_8859-3", "ISO_8859-3:1988", "csISOLatin3", "iso-8859-3", "iso-ir-109", "l3", "latin3", NULL };
static const char* s_alias28594[] = { "ISO_8859-4", "ISO_8859-4:1988", "csISOLatin4", "iso-8859-4", "iso-ir-110", "l4", "latin4", NULL };
static const char* s_alias28595[] = { "ISO_8859-5", "ISO_8859-5:1988", "csISOLatinCyrillic", "cyrillic", "iso-8859-5", "iso-ir-144", NULL };
static const char* s_alias28596[] = { "ECMA-114", "ISO_8859-6", "ISO_8859-6:1987", "arabic", "csISOLatinArabic", "iso-8859-6", "iso-ir-127", NULL };
static const char* s_alias28597[] = { "ECMA-118", "ELOT_928", "ISO_8859-7", "ISO_8859-7:1987", "csISOLatinGreek", "greek", "greek8", "iso-8859-7", "iso-ir-126", NULL };
static const char* s_alias28598[] = { "ISO-8859-8 Visual", "ISO_8859-8", "ISO_8859-8:1988", "csISOLatinHebrew", "hebrew", "iso-8859-8", "iso-ir-138", "logical", "visual", NULL };
static const char* s_alias28599[] = { "ISO_8859-9", "ISO_8859-9:1989", "csISOLatin5", "iso-8859-9", "iso-ir-148", "l5", "latin5", NULL };
static const char* s_alias28603[] = { "iso-8859-13", NULL };
static const char* s_alias28605[] = { "ISO_8859-15", "csISOLatin9", "iso-8859-15", "l9", "latin9", NULL };
static const char* s_alias29001[] = { "x-Europa", NULL };
static const char* s_alias38598[] = { "iso-8859-8-i", NULL };
static const char* s_alias50000[] = { "x-user-defined", NULL };
static const char* s_alias50001[] = { "_autodetect_all", NULL };
static const char* s_alias50220[] = { "iso-2022-jp", NULL };
static const char* s_alias50221[] = { "csISO2022JP", NULL };
static const char* s_alias50225[] = { "csISO2022KR", "iso-2022-kr", "iso-2022-kr-7", "iso-2022-kr-7bit", NULL };
static const char* s_alias50227[] = { "x-cp50227", NULL };
static const char* s_alias50229[] = { "x-cp50229", NULL };
static const char* s_alias50930[] = { "cp930", NULL };
static const char* s_alias50931[] = { "X-EBCDIC-JapaneseAndUSCanada", "x-EBCDIC-JapaneseAndUSCanada", NULL };
static const char* s_alias50932[] = { "_autodetect", NULL };
static const char* s_alias50933[] = { "cp933", NULL };
static const char* s_alias50935[] = { "cp935", NULL };
static const char* s_alias50937[] = { "cp937", NULL };
static const char* s_alias50939[] = { "cp939", NULL };
static const char* s_alias50949[] = { "_autodetect_kr", NULL };
static const char* s_alias51932[] = { "EUC-JP", "Extended_UNIX_Code_Packed_Format_for_Japanese", "csEUCPkdFmtJapanese", "euc-jp", "iso-2022-jpeuc", "x-euc", "x-euc-jp", NULL };
static const char* s_alias51936[] = { "EUC-CN", "euc-cn", "x-euc-cn", NULL };
static const char* s_alias51949[] = { "csEUCKR", "euc-kr", "iso-2022-kr-8", "iso-2022-kr-8bit", NULL };
static const char* s_alias52936[] = { "hz-gb-2312", NULL };
static const char* s_alias54936[] = { "GB18030", NULL };
static const char* s_alias57002[] = { "x-iscii-de", NULL };
static const char* s_alias57003[] = { "x-iscii-be", NULL };
static const char* s_alias57004[] = { "x-iscii-ta", NULL };
static const char* s_alias57005[] = { "x-iscii-te", NULL };
static const char* s_alias57006[] = { "x-iscii-as", NULL };
static const char* s_alias57007[] = { "x-iscii-or", NULL };
static const char* s_alias57008[] = { "x-iscii-ka", NULL };
static const char* s_alias57009[] = { "x-iscii-ma", NULL };
static const char* s_alias57010[] = { "x-iscii-gu", NULL };
static const char* s_alias57011[] = { "x-iscii-pa", NULL };
static const char* s_alias65000[] = { "csUnicode11UTF7", "unicode-1-1-utf-7", "unicode-2-0-utf-7", "utf-7", "x-unicode-1-1-utf-7", "x-unicode-2-0-utf-7", NULL };
static const char* s_alias65001[] = { "unicode-1-1-utf-8", "unicode-2-0-utf-8", "utf-8", "x-unicode-1-1-utf-8", "x-unicode-2-0-utf-8", NULL };

struct CodePageAlias
{
	unsigned int codePage;
	const char **names;
};

static CodePageAlias s_aliases[] = {
	{ 37, s_alias37 },
	{ 437, s_alias437 },
	{ 500, s_alias500 },
	{ 708, s_alias708 },
	{ 720, s_alias720 },
	{ 737, s_alias737 },
	{ 775, s_alias775 },
	{ 850, s_alias850 },
	{ 852, s_alias852 },
	{ 855, s_alias855 },
	{ 857, s_alias857 },
	{ 858, s_alias858 },
	{ 860, s_alias860 },
	{ 861, s_alias861 },
	{ 862, s_alias862 },
	{ 863, s_alias863 },
	{ 864, s_alias864 },
	{ 865, s_alias865 },
	{ 866, s_alias866 },
	{ 869, s_alias869 },
	{ 870, s_alias870 },
	{ 874, s_alias874 },
	{ 875, s_alias875 },
	{ 932, s_alias932 },
	{ 936, s_alias936 },
	{ 949, s_alias949 },
	{ 950, s_alias950 },
	{ 1026, s_alias1026 },
	{ 1047, s_alias1047 },
	{ 1140, s_alias1140 },
	{ 1141, s_alias1141 },
	{ 1142, s_alias1142 },
	{ 1143, s_alias1143 },
	{ 1144, s_alias1144 },
	{ 1145, s_alias1145 },
	{ 1146, s_alias1146 },
	{ 1147, s_alias1147 },
	{ 1148, s_alias1148 },
	{ 1149, s_alias1149 },
	{ 1200, s_alias1200 },
	{ 1201, s_alias1201 },
	{ 1250, s_alias1250 },
	{ 1251, s_alias1251 },
	{ 1252, s_alias1252 },
	{ 1253, s_alias1253 },
	{ 1254, s_alias1254 },
	{ 1255, s_alias1255 },
	{ 1256, s_alias1256 },
	{ 1257, s_alias1257 },
	{ 1258, s_alias1258 },
	{ 1361, s_alias1361 },
	{ 10000, s_alias10000 },
	{ 10001, s_alias10001 },
	{ 10002, s_alias10002 },
	{ 10003, s_alias10003 },
	{ 10004, s_alias10004 },
	{ 10005, s_alias10005 },
	{ 10006, s_alias10006 },
	{ 10007, s_alias10007 },
	{ 10008, s_alias10008 },
	{ 10010, s_alias10010 },
	{ 10017, s_alias10017 },
	{ 10021, s_alias10021 },
	{ 10029, s_alias10029 },
	{ 10079, s_alias10079 },
	{ 10081, s_alias10081 },
	{ 10082, s_alias10082 },
	{ 20000, s_alias20000 },
	{ 20001, s_alias20001 },
	{ 20002, s_alias20002 },
	{ 20003, s_alias20003 },
	{ 20004, s_alias20004 },
	{ 20005, s_alias20005 },
	{ 20105, s_alias20105 },
	{ 20106, s_alias20106 },
	{ 20107, s_alias20107 },
	{ 20108, s_alias20108 },
	{ 20127, s_alias20127 },
	{ 20261, s_alias20261 },
	{ 20269, s_alias20269 },
	{ 20273, s_alias20273 },
	{ 20277, s_alias20277 },
	{ 20278, s_alias20278 },
	{ 20280, s_alias20280 },
	{ 20284, s_alias20284 },
	{ 20285, s_alias20285 },
	{ 20290, s_alias20290 },
	{ 20297, s_alias20297 },
	{ 20420, s_alias20420 },
	{ 20423, s_alias20423 },
	{ 20424, s_alias20424 },
	{ 20833, s_alias20833 },
	{ 20838, s_alias20838 },
	{ 20866, s_alias20866 },
	{ 20871, s_alias20871 },
	{ 20880, s_alias20880 },
	{ 20905, s_alias20905 },
	{ 20924, s_alias20924 },
	{ 20936, s_alias20936 },
	{ 20949, s_alias20949 },
	{ 21025, s_alias21025 },
	{ 21027, s_alias21027 },
	{ 21866, s_alias21866 },
	{ 28591, s_alias28591 },
	{ 28592, s_alias28592 },
	{ 28593, s_alias28593 },
	{ 28594, s_alias28594 },
	{ 28595, s_alias28595 },
	{ 28596, s_alias28596 },
	{ 28597, s_alias28597 },
	{ 28598, s_alias28598 },
	{ 28599, s_alias28599 },
	{ 28603, s_alias28603 },
	{ 28605, s_alias28605 },
	{ 29001, s_alias29001 },
	{ 38598, s_alias38598 },
	{ 50000, s_alias50000 },
	{ 50001, s_alias50001 },
	{ 50220, s_alias50220 },
	{ 50221, s_alias50221 },
	{ 50225, s_alias50225 },
	{ 50227, s_alias50227 },
	{ 50229, s_alias50229 },
	{ 50930, s_alias50930 },
	{ 50931, s_alias50931 },
	{ 50932, s_alias50932 },
	{ 50933, s_alias50933 },
	{ 50935, s_alias50935 },
	{ 50937, s_alias50937 },
	{ 50939, s_alias50939 },
	{ 50949, s_alias50949 },
	{ 51932, s_alias51932 },
	{ 51936, s_alias51936 },
	{ 51949, s_alias51949 },
	{ 52936, s_alias52936 },
	{ 54936, s_alias54936 },
	{ 57002, s_alias57002 },
	{ 57003, s_alias57003 },
	{ 57004, s_alias57004 },
	{ 57005, s_alias57005 },
	{ 57006, s_alias57006 },
	{ 57007, s_alias57007 },
	{ 57008, s_alias57008 },
	{ 57009, s_alias57009 },
	{ 57010, s_alias57010 },
	{ 57011, s_alias57011 },
	{ 65000, s_alias65000 },
	{ 65001, s_alias65001 },
};

#define ENCLIST_MAXENTRIES 200
static char *s_encBuf[ENCLIST_MAXENTRIES];
static unsigned int nEncBuf = 0;

static const char* getCachedEncoding(const wchar_t* name)
{
    CString utf8Name = String(name).utf8();

    unsigned int i;
    for(i = 0; i < nEncBuf && i < ENCLIST_MAXENTRIES; ++i)
    {
        if(!strcmp(s_encBuf[i], utf8Name.data()))
            return s_encBuf[i];
    }

    if(i == ENCLIST_MAXENTRIES)
    {
        ASSERT(0);
        return 0;
    }

    s_encBuf[i] = strdup(utf8Name.data());
    nEncBuf++;
    return s_encBuf[i];
}

static PassOwnPtr<TextCodec> newTextCodecWin(const TextEncoding& encoding, const void*)
{
    return new TextCodecWin(encoding);
}

void TextCodecWin::registerBaseEncodingNames(EncodingNameRegistrar registrar)
{
    registrar("UTF-8", "UTF-8");
}

void TextCodecWin::registerBaseCodecs(TextCodecRegistrar registrar)
{
    registrar("UTF-8", newTextCodecWin, 0);
}

void TextCodecWin::registerExtendedEncodingNames(EncodingNameRegistrar registrar)
{
    IEnumCodePage *enumCodePage = 0;
    Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();
    multiLang->EnumCodePages(0, 0, &enumCodePage);

    MIMECPINFO codePageInfo;
    ULONG nCodePageInfo;
    while(enumCodePage->Next(1, &codePageInfo, &nCodePageInfo) == S_OK)
    {
        const char *cachedName = getCachedEncoding(codePageInfo.wszWebCharset);
        registrar(cachedName, cachedName);

        // should probably optimize this
        for(unsigned int i = 0; i < sizeof(s_aliases) / sizeof(CodePageAlias); ++i)
        {
            if(s_aliases[i].codePage == codePageInfo.uiCodePage)
            {
                for(unsigned int j = 0; s_aliases[i].names[j]; ++j)
                {
                    if(strcmp(s_aliases[i].names[j], cachedName))
                        registrar(s_aliases[i].names[j], cachedName);
                }
            }
        }
    }

    enumCodePage->Release();

    // Additional aliases.
    // Perhaps we can get these added to ICU.
    registrar("macroman", "macintosh");
    registrar("xmacroman", "macintosh");

    // Additional aliases that historically were present in the encoding
    // table in WebKit on Macintosh that don't seem to be present in ICU.
    // Perhaps we can prove these are not used on the web and remove them.
    // Or perhaps we can get them added to ICU.
    registrar("cnbig5", /*"Big5"*/ "big5");
    registrar("cngb", "EUC-CN");
    registrar("csISO88598I", /*"ISO_8859-8-I"*/ "windows-1255");
    //registrar("csgb231280", "EUC-CN"); from windows aliases
    registrar("dos720", /*"cp864"*/ "DOS-720");
    registrar("dos874", /*"cp874"*/ "windows-874");
    registrar("jis7", /*"ISO-2022-JP"*/ "iso-2022-jp");
    //registrar("koi", "KOI8-R"); from windows aliases
    //registrar("logical", "ISO-8859-8-I"); from windows aliases
    registrar("unicode11utf8", /*"UTF-8"*/ "utf-8");
    registrar("unicode20utf8", /*"UTF-8"*/ "utf-8");
    //registrar("visual", "ISO-8859-8");
    registrar("winarabic", "windows-1256");
    registrar("winbaltic", "windows-1257");
    registrar("wincyrillic", "windows-1251");
    registrar("windows874", /*"windows874-2000"*/ "windows-874");
    registrar("iso885911", /*"windows874-2000"*/ "windows-874");
    registrar("wingreek", "windows-1253");
    registrar("winhebrew", "windows-1255");
    registrar("winlatin2", "windows-1250");
    registrar("winturkish", "windows-1254");
    registrar("winvietnamese", "windows-1258");
    registrar("xcp1250", "windows-1250");
    registrar("xcp1251", "windows-1251");
    registrar("xeuc", /*"EUC-JP"*/ "euc-jp");
    registrar("xeuccn", "EUC-CN");
    registrar("xgbk", "EUC-CN");
    registrar("xunicode20utf8", /*"UTF-8"*/ "utf-8");
    registrar("xxbig5", /*"Big5"*/ "big5");
}

void TextCodecWin::registerExtendedCodecs(TextCodecRegistrar registrar)
{
    // See comment above in registerEncodingNames.
    registrar("ISO-8859-8-I", newTextCodecWin, 0);

    IEnumCodePage *enumCodePage = 0;
    Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();
    multiLang->EnumCodePages(0, 0, &enumCodePage);

    MIMECPINFO codePageInfo;
    ULONG nCodePageInfo;
    while(enumCodePage->Next(1, &codePageInfo, &nCodePageInfo) == S_OK)
    {
        const char* cacheName = getCachedEncoding(codePageInfo.wszWebCharset);
        registrar(cacheName, newTextCodecWin, 0);
    }

    enumCodePage->Release();
}

TextCodecWin::TextCodecWin(const TextEncoding& encoding)
    :   m_encoding(encoding),
        m_dwMode(0),
		m_buffer(0)
{
    Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();

    MIMECSETINFO mimeInfo;
    String charSet(m_encoding.name());

    multiLang->GetCharsetInfo((BSTR)charSet.charactersWithNullTermination(), &mimeInfo);
    m_codePage = mimeInfo.uiInternetEncoding;
}

/*
* For several code pages MultiByteToWideChar accespts only 0 for the second parameter.
*/
DWORD TextCodecWin::flagsForCodePage(UINT codePage)
{
	 return (codePage == 50220 || codePage == 50221 || codePage == 50222 || codePage == 50225 ||
            codePage == 50227 || codePage == 50229 || (codePage >= 57002 && codePage <= 57011) || 
			codePage == 65000 || codePage == 42) ? 0 : MB_ERR_INVALID_CHARS; 

}

String TextCodecWin::decode(const char* bytes, size_t length, bool flush, bool stopOnError, bool& sawError)
{    
	const char* actualBuffer;
	size_t actualLength;

	/* If m_buffer is empty we are working on the original bytes.
	* Otherwise, we are appending the bytes to m_buffer and we are working on the buffer.
	* This is an optimization based on the fact that we expect considerably fewer cases in which m_buffer is not empty.
	*/
	if(m_buffer.size()==0)
	{
		actualBuffer = bytes;
		actualLength = length;
	}
	else
	{
		m_buffer.appendRange(bytes,bytes+length);
		actualBuffer = m_buffer.data();
		actualLength = m_buffer.size();
	}
	
	/*
	* Find the longest prefix of valid bytes by decreasing the size of the byte array.
	* This is quick for potentially valid inputs, because only a few bytes of the last bytes can be invalid.
	* Invalid bytes at the begging of the array are more expensive to detect, but they represent invalid inputs which are less common. 
	* The penalty for detecting an invalid UTF-8 byte at the beggining of an array of 64KBytes seems unnoticeable.
	*/
	UINT wideCharsLength = 0;
	size_t parsedLength;

	// Some codepages do not work well with windows API but we can use equivalent ones.
	UINT actualCodePage = m_codePage;
	switch(actualCodePage)
	{
		case 51932:
			/*
			* Fix for bug 2507406.
			* MultiByteToWideChar does not work correctly with code page 51932, however it works with the equivalent codepage 20932.
			* http://blogs.msdn.com/shawnste/archive/2006/07/18/encoding-getencodings-has-a-couple-duplicate-names.aspxhttp://blogs.msdn.com/shawnste/archive/2006/07/18/encoding-getencodings-has-a-couple-duplicate-names.aspx
			*/
			actualCodePage = 20932;
			break;
		// Add more cases here for equivalent codepages.
	}

	/*
	* In case of unexpected errors we will try to parse the string using IMultiLanguage API.
	* That API was used in previous versions and falling back to it should ensure backwards compatibility.
	* This variable will be set to true when we need to enter the fallback mode.
	* However, we do no expect to fallback and cases that make this happen should be investigated.
	*/
	bool fallBack = false;

	for(parsedLength = actualLength; parsedLength>0;parsedLength--)
	{
		DWORD flags = stopOnError ? flagsForCodePage(actualCodePage) : 0;
		wideCharsLength = MultiByteToWideChar(actualCodePage,flags,actualBuffer, parsedLength,0,0);

		if(wideCharsLength > 0)
		{
            if (!stopOnError)
            {
                // We're doing this check only when stopOnError is false, because 
                // we want to leave the extra bytes (associated with a split multi-byte Unicode character) in the buffer
                // so that the next chunk will have them and the decoding function will decode properly the Unicode character.
                //
                // When stopOnError is true, we have to find the exact position of the invalid character and this can be done
                // only by using MultiByteToWideChar and removing bytes from the end.
                Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();
                UINT checkWideCharsLength = 0;
                UINT checkParsedLength = (UINT)parsedLength;
                if (multiLang->ConvertStringToUnicode(&m_dwMode, m_codePage, (CHAR*)actualBuffer, &checkParsedLength, NULL, &checkWideCharsLength) == S_OK)
                {
                    // this might happen if a multi-byte Unicode character is split across chunks
                    if (checkParsedLength < parsedLength)
                    {
                        ASSERT(checkWideCharsLength != wideCharsLength);
                        wideCharsLength = checkWideCharsLength;
                        parsedLength = checkParsedLength;
                    }
                }
            }
			break;
		}

		DWORD err = ::GetLastError();
		// If there are invalid characters continue parsing with a shorter string.
		if(err == ERROR_NO_UNICODE_TRANSLATION)
			continue;
		
		
		// We do no expect to fallback and cases that make this happen should be investigated.
		ASSERT(0);
		fallBack = true;
		Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();
		// in 64 bit mode, we get a compiler error when trying to pass a size_t* as a UINT* in the call below.
		UINT tempLength = (UINT)parsedLength; 
		if(multiLang->ConvertStringToUnicode(&m_dwMode, m_codePage, (CHAR*)actualBuffer, &tempLength, NULL, &wideCharsLength)!=S_OK)
		{
			// If fall back option does not work either, just give up gracefully.
			parsedLength = wideCharsLength = 0;
		}
		else
		{
			parsedLength = tempLength;
		}
		break;	
	}
			
	Vector<UChar> dst(wideCharsLength);
	
	if(parsedLength>0)
	{
		if(!fallBack)
		{
			int res = MultiByteToWideChar(actualCodePage,0,actualBuffer,parsedLength,dst.data(),wideCharsLength);
			ASSERT(res == wideCharsLength);
		}
		else
		{
			Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();
			// in 64 bit mode, we get a compiler error when trying to pass a size_t* as a UINT* in the call below.
			UINT tempLength = (UINT)parsedLength; 
			if(multiLang->ConvertStringToUnicode(&m_dwMode, m_codePage, (CHAR*)actualBuffer, &tempLength, dst.data(), &wideCharsLength)!=S_OK)
			{
				// If fall back option does not work either, just give up gracefully.
				parsedLength = wideCharsLength = 0;
				dst.clear();
			}
			else
			{
				parsedLength = tempLength;
			}
		}
	}

	/*
	* If not all bytes were parsed then copy the remaining ones to m_buffer.
	* Otherwise, clear m_buffer.
	*/
	if(parsedLength<actualLength)
	{
		/*
		* If m_buffer is empty then we are working on the original bytes, so just copy the remaining bytes.
		* Otherwise, we are working on m_buffer, so remove the parsed bytes.
		*/
		if(m_buffer.size()==0)
		{
			m_buffer.appendRange(actualBuffer+parsedLength,actualBuffer+actualLength);
		}
		else
		{
			m_buffer.remove(0,parsedLength);
		}
	}
	else
	{
		m_buffer.clear();
	}

	/*
	* If there are any remaining bytes when flushing then we have an error.
	* We assume the encoding of a char does not take more than MAX_BYTES_FOR_CHARACTER bytes.
	* If there are more than MAX_BYTES_FOR_CHARACTER bytes left in m_buffer then we have an error.
	*/
	if( (flush && m_buffer.size()>0) || (m_buffer.size()>MAX_BYTES_FOR_CHARACTER) )
	{
		sawError = true;
		m_buffer.clear();
	}
	
	sawError = sawError && stopOnError;
	
    return String::adopt(dst);
}

CString TextCodecWin::encode(const UChar* characters, size_t length, UnencodableHandling handling)
{
    if (!length)
        return "";

    // FIXME: We should see if there is "force ASCII range" mode in ICU;
    // until then, we change the backslash into a yen sign.
    // Encoding will change the yen sign back into a backslash.
    String copy(characters, length);
    copy = m_encoding.displayString(copy.impl());

    const UChar* source = copy.characters();
    UINT sourceLen = copy.length();
    DWORD flag = 0;

    // Unfortunately, if we pass a buffer that is too small, this method will not
    // tell us how many characters more we need
    // Thus, try first with the buffer, if that fails, try the 2 pass size+convert
    // TODO: research if ConvertStringInIStream would be any faster

    static const unsigned int s_bufferSize = 2048;
    char buffer[s_bufferSize];

    WCHAR fallBack[] = { 0, 0 };

    // I'm pretty sure EntitiesForUnencodables + URLEncodedEntitiesForUnencodables
    // are not right
    switch (handling) {
        case QuestionMarksForUnencodables:
            flag = MLCONVCHARF_USEDEFCHAR;
            fallBack[0] = '?';
            break;
        case EntitiesForUnencodables:
            flag = MLCONVCHARF_NAME_ENTITIZE;
            break;
        case URLEncodedEntitiesForUnencodables:
            flag = MLCONVCHARF_NCR_ENTITIZE; // use numerics (which should be URL Encodeable)
            break;
    }

    UINT dstLen = s_bufferSize;

    Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();
    if(multiLang->ConvertStringFromUnicodeEx(&m_dwMode, m_codePage, 
        (WCHAR*) source, &sourceLen, 
        buffer, &dstLen,
        flag, fallBack) == S_OK)
    {
        return CString(buffer, dstLen);
    }

    sourceLen = copy.length(); // ConvertStringFromUnicodeEx will set this to zero <sigh>
    dstLen = 0;

    // --------------------- Otherwise.... --------------------------------
    //
    // Get dst length, then do conversion
    if(multiLang->ConvertStringFromUnicodeEx(&m_dwMode, m_codePage, 
        (WCHAR*) source, &sourceLen, 
        0, &dstLen,
        flag, fallBack) != S_OK)
    {
        ASSERT(0);
        return CString();
    }

    Vector<char> vBuffer(dstLen);

    if(multiLang->ConvertStringFromUnicodeEx(&m_dwMode, m_codePage, 
        (WCHAR*) source, &sourceLen, 
        vBuffer.data(), &dstLen,
        flag, fallBack) != S_OK)
    {
        ASSERT(0);
        return CString();
    }

    return CString(vBuffer.data(), dstLen);
}


} // namespace WebCore
