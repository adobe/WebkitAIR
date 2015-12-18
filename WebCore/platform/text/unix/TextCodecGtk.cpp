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


#include "config.h"
#include "TextCodecGtk.h"

#include "ThreadGlobalData.h"
#include <wtf/text/CString.h>
#include <glib.h>

#if DEBUG
extern FILE *printencodinglog_file;
#include <sys/time.h>
#define PrintEncodingLog(X...)      \
	        {                                                               \
	                if(!printencodinglog_file) printencodinglog_file = fopen("/tmp/enc_logs", "w"); \
	                if(printencodinglog_file)                                \
	                {                                                       \
	                        timeval time;                                   \
	                        gettimeofday(&time, NULL);                      \
		                int milli_seconds = ((time.tv_sec % 1000) * 1000) + (time.tv_usec / 1000); \
	                        fprintf(printencodinglog_file, "%d - ", milli_seconds); \
		                fprintf(printencodinglog_file, X);               \
	                        fflush(printencodinglog_file);                   \
	                }                                                       \
	        }
#else
#define PrintEncodingLog(X...)
#endif

namespace WebCore {

const size_t ConversionBufferSize = 16384;
static const char* supportedEncodings[] = {
					"ASCII",
					"ISO-8859-1",
					"ISO-8859-2",
					"ISO-8859-3",
					"ISO-8859-4",
					"ISO-8859-5",
					"ISO-8859-7",
					"ISO-8859-9",
					"ISO-8859-10",
					"ISO-8859-13",
					"ISO-8859-14",
					"ISO-8859-15",
					"ISO-8859-16",
					"KOI8-R",
					"KOI8-RU",
					"CP1250",
					"CP1251",
					"CP1252",
					"CP1253",
					"CP1254",
					"CP1257",
					"CP850",
					"CP866",
					"CP1131",
					"MacRoman",
					"MacCentralEurope",
					"MacIceland",
					"MacCroatian",
					"MacRomania",
					"MacCyrillic",
					"MacUkraine",
					"MacGreek",
					"MacTurkish",
					"ISO-8859-6",
					"ISO-8859-8",
					"CP1255",
					"CP1256",
					"CP862",
					"MacHebrew",
					"MacArabic",
					"EUC-JP",
					"SHIFT_JIS",
					"CP932",
					"ISO-2022-JP",
					"ISO-2022-JP-2",
					"ISO-2022-JP-1",
					"EUC-CN",
					"HZ",
					"GBK",
					"CP936",
					"GB18030",
					"EUC-TW",
					"BIG5",
					"CP950",
					"BIG5-HKSCS:2001",
					"BIG5-HKSCS:1999",
					"ISO-2022-CN",
					"ISO-2022-CN-EXT",
					"EUC-KR",
					"CP949",
					"ISO-2022-KR",
					"JOHAB",
					"ARMSCII-8",
					"Georgian-Academy",
					"Georgian-PS",
					"KOI8-T",
					"PT154",
					"RK1048",
					"ISO-8859-11",
					"TIS-620",
					"CP874",
					"MacThai"
					"MuleLao-1",
					"CP1133",
					"VISCII",
					"TCVN",
					"CP1258",
					"HP-ROMAN8",
					"NEXTSTEP",
					"UTF-8",
					"UCS-2",
					"UCS-2BE",
					"UCS-2LE",
					"UCS-4",
					"UCS-4BE",
					"UCS-4LE",
					"UTF-16",
					"UTF-16BE",
					"UTF-32",
					"UTF-32BE",
					"UTF-32LE",
					"UTF-7",
					"C99",
					"JAVA",
					"UCS-2-INTERNAL",
					"UCS-4-INTERNAL"
				};

static const char* s_alias28591[] = { "cp819", "csISOLatin1", "ibm819", "iso-8859-1", "iso-ir-100", "iso8859-1", "iso_8859-1", "iso_8859-1:1987", "l1", "latin1", "us-ascii", "ansi_x3.4-1968", "iso-ir-6", "ansi_x3.4-1986", "iso646-us", "us", "iso_8859-1:1978", "ibm-819", "819", "ISO-8859-1-Windows-3.0-Latin-1", "ISO-8859-1-Windows-3.1-Latin-1", "8859_1", NULL };
static const char* s_alias28592[] = { "csISOLatin2", "iso-8859-2", "iso-ir-101", "iso8859-2", "iso_8859-2", "iso_8859-2:1987", "l2", "latin2", NULL };
static const char* s_alias28593[] = { "ISO_8859-3", "ISO_8859-3:1988", "csISOLatin3", "iso-8859-3", "iso-ir-109", "l3", "latin3", NULL };
static const char* s_alias28594[] = { "ISO_8859-4", "ISO_8859-4:1988", "csISOLatin4", "iso-8859-4", "iso-ir-110", "l4", "latin4", NULL };
static const char* s_alias28595[] = { "ISO_8859-5", "ISO_8859-5:1988", "csISOLatinCyrillic", "cyrillic", "iso-8859-5", "iso-ir-144", "ibm915", "cp915", "915", NULL };
static const char* s_alias28596[] = { "ECMA-114", "ISO_8859-6", "ISO_8859-6:1987", "arabic", "csISOLatinArabic", "iso-8859-6", "iso-ir-127", "asmp-708", "cp1089", "ibm1089", NULL };
static const char* s_alias28597[] = { "ECMA-118", "ELOT_928", "ISO_8859-7", "ISO_8859-7:1987", "csISOLatinGreek", "greek", "greek8", "iso-8859-7", "iso-ir-126", "ibm813", NULL };
static const char* s_alias28598[] = { "ISO-8859-8 Visual", "ISO_8859-8", "ISO_8859-8:1988", "csISOLatinHebrew", "hebrew", "iso-8859-8", "iso-ir-138", "logical", "visual", NULL };
static const char* s_alias28599[] = { "ISO_8859-9", "ISO_8859-9:1989", "csISOLatin5", "iso-8859-9", "iso-ir-148", "l5", "latin5", NULL };
static const char* s_alias28603[] = { "iso-8859-13", NULL };
static const char* s_alias28605[] = { "ISO_8859-15", "csISOLatin9", "iso-8859-15", "l9", "latin9", NULL };
static const char* s_alias21866[] = { "koi8-ru", "koi8-u", NULL };
static const char* s_alias1250[] = { "windows-1250", "x-cp1250", NULL };
static const char* s_alias1251[] = { "windows-1251", "x-cp1251", NULL };
static const char* s_alias1252[] = { "Windows-1252", "windows-1252", "x-ansi", NULL };
static const char* s_alias1253[] = { "windows-1253", NULL };
static const char* s_alias1254[] = { "Windows-1254", "windows-1254", NULL };
static const char* s_alias1255[] = { "windows-1255", NULL };
static const char* s_alias1256[] = { "cp1256", "windows-1256", NULL };
static const char* s_alias1257[] = { "windows-1257", NULL };

static const char* s_alias850[] = { "IBM850", "cp850", "ibm850", NULL };
static const char* s_alias866[] = { "IBM866", "cp866", NULL };

static const char* s_alias862[] = { "DOS-862", "IBM862", "cp862", NULL };
static const char* s_alias51932[] = { "EUC-JP", "Extended_UNIX_Code_Packed_Format_for_Japanese", "csEUCPkdFmtJapanese", "euc-jp", "iso-2022-jpeuc", "x-euc", "x-euc-jp", "eucjis", "csEUCkdFmtJapanese", NULL };
static const char* s_alias932[] = { "csShiftJIS", "csWindows31J", "ms_Kanji", "shift-jis", "shift_jis", "sjis", "x-ms-cp932", "x-sjis", "jis", "jis_encoding", "Windows-31J", "cswindows31j", "x-ms-cp932", NULL };
static const char* s_alias2022jp[] = { "iso-2022-jp", "csISO2022JP", "ISO2022JP", NULL };

static const char* s_alias51936[] = { "EUC-CN", "euc-cn", "x-euc-cn", NULL };

static const char* s_alias936[] = { "CN-GB", "GB2312", "GB231280", "GBK", "GB_2312-80", "chinese", "csGB2312", "csGB231280", "csISO58GB231280", "gb2312", "iso-ir-58", NULL };

static const char* s_alias54936[] = { "gb18030", NULL };
static const char* s_alias11643[] = { "euc-tw", "euc_TW", "euctw", "EUC-TW", "cns11643", NULL };
static const char* s_alias950[] = { "Big5", "Big5-HKSCS", "big5", "cn-big5", "csbig5", "x-x-big5", NULL };
static const char* s_alias51949[] = { "csEUCKR", "euc-kr", "x-windows-949", "iso-2022-kr-8", "iso-2022-kr-8bit", "ks_c_5601-1987", "ks_c_5601", "ksc5601", "csKSC56011987", NULL };
static const char* s_alias2022kr[] = { "csISO2022KR", "ISO2022KR", NULL };

static const char* s_alias1361[] = { "Johab", NULL };
static const char* s_alias874[] = { "DOS-874", "TIS-620", "iso-8859-11", "windows-874", NULL };
static const char* s_alias65001[] = { "unicode-1-1-utf-8", "unicode-2-0-utf-8", "utf-8", "x-unicode-1-1-utf-8", "x-unicode-2-0-utf-8", NULL };
static const char* s_alias1200[] = { "UTF-16LE", "unicode", "utf-16", NULL };
static const char* s_alias1201[] = { "UTF-16BE", "unicodeFFFE", NULL };
static const char* s_alias65000[] = { "csUnicode11UTF7", "unicode-1-1-utf-7", "unicode-2-0-utf-7", "utf-7", "x-unicode-1-1-utf-7", "x-unicode-2-0-utf-7", NULL };


struct CodePageAlias
{
    const char* standardName;
    const char **names;
};

static CodePageAlias s_aliases[] = {
	{ "ASCII", NULL },
	{ "ISO-8859-1", s_alias28591 },
	{ "ISO-8859-2", s_alias28592 },
	{ "ISO-8859-3", s_alias28593 },
	{ "ISO-8859-4", s_alias28594 },
	{ "ISO-8859-5", s_alias28595 },
	{ "ISO-8859-7", s_alias28597 },
	{ "ISO-8859-9", s_alias28599 },
	{ "ISO-8859-10", NULL },
	{ "ISO-8859-13", s_alias28603 },
	{ "ISO-8859-14", NULL },
	{ "ISO-8859-15", s_alias28605 },
	{ "ISO-8859-16", NULL },
	{ "KOI8-R", NULL },
	{ "KOI8-RU", s_alias21866 },
	{ "CP1250", s_alias1250 },
	{ "CP1251", s_alias1251 },
	{ "CP1252", s_alias1252 },
	{ "CP1253", s_alias1253 },
	{ "CP1254", s_alias1254 },
	{ "CP1257", s_alias1257 },
	{ "CP850", s_alias850 },
	{ "CP866", s_alias866 },
	{ "CP1131", NULL },
	{ "MacRoman", NULL },
	{ "MacCentralEurope", NULL },
	{ "MacIceland", NULL },
	{ "MacCroatian", NULL },
	{ "MacRomania", NULL },
	{ "MacCyrillic", NULL },
	{ "MacUkraine", NULL },
	{ "MacGreek", NULL },
	{ "MacTurkish", NULL },
	{ "ISO-8859-6", s_alias28596 },
	{ "ISO-8859-8", s_alias28598 },
	{ "CP1255", s_alias1255 },
	{ "CP1256", s_alias1256 },
	{ "CP862",  s_alias862 },
	{ "MacHebrew", NULL },
	{ "MacArabic", NULL },
	{ "EUC-JP",  s_alias51932 },
	{ "SHIFT_JIS", s_alias932 },
	{ "CP932", NULL },
	{ "ISO-2022-JP", s_alias2022jp },
	{ "ISO-2022-JP-2", NULL },
	{ "ISO-2022-JP-1", NULL },
	{ "EUC-CN", s_alias51936 },
	{ "HZ", NULL },
	{ "GBK", s_alias936 },
	{ "CP936", NULL },
	{ "GB18030", s_alias54936 },
	{ "EUC-TW", s_alias11643 },
	{ "BIG5",  s_alias950 },
	{ "CP950", NULL },
	{ "BIG5-HKSCS:2001", NULL },
	{ "BIG5-HKSCS:1999", NULL },
	{ "ISO-2022-CN", NULL },
	{ "ISO-2022-CN-EXT", NULL },
	{ "EUC-KR", s_alias51949 },
	{ "CP949", NULL },
	{ "ISO-2022-KR", s_alias2022kr },
	{ "JOHAB", s_alias1361 },
	{ "ARMSCII-8", NULL },
	{ "Georgian-Academy", NULL },
	{ "Georgian-PS", NULL },
	{ "KOI8-T", NULL },
	{ "PT154", NULL },
	{ "RK1048", NULL },
	{ "ISO-8859-11", s_alias874 },
	{ "TIS-620", NULL },
	{ "CP874", NULL },
	{ "MacThai", NULL },
	{ "MuleLao-1", NULL },
	{ "CP1133", NULL },
	{ "VISCII", NULL },
	{ "TCVN", NULL },
	{ "CP1258", NULL },
	{ "HP-ROMAN8", NULL },
	{ "NEXTSTEP", NULL },
	{ "UTF-8", s_alias65001 },
	{ "UCS-2", NULL },
	{ "UCS-2BE", NULL },
	{ "UCS-2LE", NULL },
	{ "UCS-4", NULL },
	{ "UCS-4BE", NULL },
	{ "UCS-4LE", NULL },
	{ "UTF-16", s_alias1200 },
	{ "UTF-16BE", s_alias1201 },
	{ "UTF-16LE", NULL },
	{ "UTF-32",  NULL },
	{ "UTF-32BE", NULL },
	{ "UTF-32LE", NULL },
	{ "UTF-7", s_alias65000 },
	{ "C99", NULL },
	{ "JAVA", NULL },
	{ "UCS-2-INTERNAL", NULL },
	{ "UCS-4-INTERNAL", NULL },
};


static PassOwnPtr<TextCodec> newTextCodecGtk(const TextEncoding& encoding, const void*)
{
    return new TextCodecGtk(encoding);
}


void TextCodecGtk::registerBaseEncodingNames(EncodingNameRegistrar registrar)
{
    registrar("UTF-8", "UTF-8");
}

void TextCodecGtk::registerBaseCodecs(TextCodecRegistrar registrar)
{
    registrar("UTF-8", newTextCodecGtk, 0);
}

void TextCodecGtk::registerExtendedEncodingNames(EncodingNameRegistrar registrar)
{
    int counter = 0;
    while(supportedEncodings[counter])
    {
        const char *encodingName = supportedEncodings[counter];
        registrar(encodingName, encodingName);

        // should probably optimize this
        for(unsigned int i = 0; i < sizeof(s_aliases) / sizeof(CodePageAlias); ++i)
        {
            if(strcmp(s_aliases[i].standardName, encodingName) == 0)
            {
       		if(s_aliases[i].names)
                {
                    for(unsigned int j = 0; s_aliases[i].names[j]; ++j)
                    {
                        if(strcmp(s_aliases[i].names[j], encodingName))
                            registrar(s_aliases[i].names[j], encodingName);
                    }
                }
            }
        }

        counter++;
    }

}


void TextCodecGtk::registerExtendedCodecs(TextCodecRegistrar registrar)
{
    int counter = 0;
    while(supportedEncodings[counter])
    {
        const char *codecName = supportedEncodings[counter];
    	registrar(codecName, newTextCodecGtk, 0);
        counter++;
    }
}

TextCodecGtk::TextCodecGtk(const TextEncoding& encoding)
    : m_encoding(encoding),
      m_contextBytes(NULL),
      m_contextLength(0)
{
}

TextCodecGtk::~TextCodecGtk()
{
}


gchar* TextCodecGtk::gconvert_till_error(const char* str,
                                         gssize length,
                                         const char* to_codeset,
                                         const char* from_codeset,
                                         gsize *bytes_read,
                                         gsize *bytes_written,
                                         GError **error)
{
    //first we try to convert the string 
    GError* internal_error = NULL;
    gchar* result = g_convert (str,
                               length,
                               to_codeset,
                               from_codeset,
                               bytes_read,
                               bytes_written,
                               &internal_error);

    //if there are unsupported characters
    //bytes_read would store the number of bytes that could be successfully converted
    //so we try to convert only that
    if(!result && (*bytes_read < length)  && internal_error )
    {
        result = g_convert ( str,
                             *bytes_read,
                             to_codeset,
                             from_codeset,
                             bytes_read,
                             bytes_written,
                             NULL);
    }

    if(error)
        *error = internal_error;
    else if(internal_error)
        g_error_free(internal_error);

    return result;
}

gchar* TextCodecGtk::gconvert_skipping_unconvertables(const char* str,
                                                      gssize length,
                                                      const char* to_codeset,
                                                      const char* from_codeset,
                                                      gsize *bytes_read,
                                                      gsize *bytes_written,
                                                      GError **error)
{
    //we try to convert the whole thing
    //here we skip parts that could not be converted and continue the conversion
    

    const gchar* remaining_string = str;
    gchar* temp_result = NULL;
    gssize remaining_length = length;
    gsize total_bytes_read = 0;
    gsize total_bytes_written = 0;
    gchar* result = NULL;

 
    do
    {
        GError* internal_error = NULL;
        int remove_unreadable_bytes = 0;
        temp_result = gconvert_till_error(remaining_string,
                                           remaining_length,
                                           to_codeset,
                                           from_codeset,
                                           bytes_read,
                                           bytes_written,
                                           &internal_error);

        if(internal_error)
        {
            remove_unreadable_bytes = 1;
            g_error_free(internal_error);
        }
        else if (temp_result != NULL && (*bytes_read < remaining_length))
        {
            //bug #2676138: gconvert_skipping_unconvertables will go into an infinite loop if there are trailing incomplete characters.
            //In this case, g_convert will not report an error. Still, bytes_read will be less than the passed length.
            remove_unreadable_bytes = (remaining_length - (*bytes_read));
        }

        remaining_length -= (*bytes_read + remove_unreadable_bytes);
        remaining_string += (*bytes_read + remove_unreadable_bytes);
        total_bytes_read += (*bytes_read + remove_unreadable_bytes);
        gsize length_of_prev_result = total_bytes_written;
        total_bytes_written += *bytes_written;

        if(temp_result)
        {
            PrintEncodingLog("TextEncodingGtk::gConvertSkippingUnconvertables InputLength = %u, Bytes Read = %u, Bytes Written = %u\n", remaining_length, *bytes_read, *bytes_written);
            if(result)
            {
                gchar* prev_result = result;
                result = (gchar*)g_malloc(total_bytes_written * sizeof(gchar) + 2); //2 for null termination
                memcpy(result, prev_result, length_of_prev_result);
                memcpy(result + length_of_prev_result, temp_result, *bytes_written);
                result[total_bytes_written] = result[total_bytes_written + 1] = '0';
                g_free(temp_result);
                g_free(prev_result);
            }
            else
                result = temp_result;
        }

#ifdef DEBUG
        if(remaining_length > 0)
        {
            PrintEncodingLog("TextEncodingGtk::gConvertWithContext Falling back\n");
        }
#endif //DEBUG

   }while(remaining_length > 0);

   *bytes_read = total_bytes_read;
   *bytes_written = total_bytes_written;

   return result;
}



char* TextCodecGtk::gConvertWithContext(const char* str,
                                        gssize length,
                                        const char* toCodeset,
                                        const char* fromCodeset,
                                        gsize *bytesRead,
                                        gsize *bytesWritten,
                                        bool flush,
                                        bool stopOnError,
                                        GError **error)
{
    bool saveContext = true;
    gssize lengthToConvert = length;

    //if there is an existing context 
    //then the conversion string should include the context
    char* bytesPrependedWithContext = NULL;
    if(m_contextBytes && m_contextLength)
    {
        lengthToConvert = length + m_contextLength;
        bytesPrependedWithContext = new char[lengthToConvert];
        if(!bytesPrependedWithContext)
        {
            //Out of Memory
            return NULL;
        }
        memcpy(bytesPrependedWithContext, m_contextBytes, m_contextLength);

        if(str && length)
            memcpy(bytesPrependedWithContext + m_contextLength, str, length);

        delete m_contextBytes;
    }

    m_contextBytes = NULL;
    m_contextLength = 0;


    const char* bytesPassedTo_gconvert = (bytesPrependedWithContext) ? bytesPrependedWithContext : str;
    if(!bytesPassedTo_gconvert || !lengthToConvert)
    {
        //there are no bytes
        return NULL;
    }



    ASSERT(bytesPassedTo_gconvert);
    ASSERT(lengthToConvert > 0);
    gchar* resultString = NULL;
    if(!flush)   //if we don't flush then we convert as much as we can and store the rest as context
    {
        resultString = gconvert_till_error(bytesPassedTo_gconvert,
                                           lengthToConvert,
                                           toCodeset,
                                           fromCodeset,
                                           bytesRead,
                                           bytesWritten,
                                           error);
    }
    else
    {
        resultString = gconvert_skipping_unconvertables(bytesPassedTo_gconvert,
                                                       lengthToConvert,
                                                       toCodeset,
                                                       fromCodeset,
                                                       bytesRead,
                                                       bytesWritten,
                                                       error);
    }

    //if some bytes haven't been converted 
    //and if we are not flushing 
    //then store the unconverted bytes as context
    gsize& bytesReadValue = *bytesRead;
    PrintEncodingLog("TextEncodingGtk::gConvertWithContext InputLength = %u, bytesRead = %u Flush = %s Error = %s, %d \n",
                      lengthToConvert, 
                      bytesReadValue, 
                      (flush)? "true": "false", 
                      (*error) ? (*error)->message: "NULL" ,
                      (*error) );


    //if stopOnError is set then we don't save the context
    if(stopOnError && (*error))
        saveContext = false;

    if(saveContext && (bytesReadValue < lengthToConvert) && !flush)
    {
    	m_contextLength = lengthToConvert - bytesReadValue;
    	PrintEncodingLog("TextEncodingGtk::gConvertWithContext Storing Context length = %u\n", m_contextLength);
        m_contextBytes = new char[m_contextLength];
        if(m_contextBytes)
            memcpy(m_contextBytes, bytesPassedTo_gconvert + bytesReadValue, m_contextLength);
        else
        {
            //Out of memory then empty the length
            m_contextLength = 0;
        }
    }

    ASSERT(resultString);
#ifdef DEBUG
    if(!resultString)
        PrintEncodingLog("TextCodecGtk::gConvertWithContext FAILED in decoding: %s\n", m_encoding.name());
#endif

    if(bytesPrependedWithContext)
        delete bytesPrependedWithContext;

    return resultString;
}



String TextCodecGtk::decode(const char* bytes, size_t length, bool flush, bool stopOnError, bool& sawError)
{
    PrintEncodingLog("TextCodecGtk::decode entering, Encoding = %s\n",m_encoding.name());

    if(!flush)
    {
        //if flush then bytes and length are allowed to be empty because the context is used for conversion
        if(!bytes || length <= 0) 
            return "";
    }

    GError *error = NULL;
    gsize bytesRead, bytesWritten;

    gchar* resultCString = gConvertWithContext(bytes, length,
                           "UTF-16",
                           m_encoding.name(),
                           &bytesRead,
                           &bytesWritten,
                           flush,
                           stopOnError,
                           &error);

    if(error)
    {
        PrintEncodingLog("TextCodecGtk::decode Error: %s Bytes Read = %u \n",error->message, bytesRead);
        if(!strncmp(m_encoding.name(), "UTF-8", 6))
            PrintEncodingLog("TextCodecGtk::decode Bytes UTF8 = %s", bytes);

        ASSERT(0);
        sawError = true;
        g_error_free(error);
    }

    Vector<UChar> result;
    if(resultCString)
    {
        int resultLen;
#ifdef DEBUG
        gchar* debugUTF8 = g_utf16_to_utf8(reinterpret_cast<UChar*>(resultCString) + 1,
                                           bytesWritten / sizeof(UChar) - 1,
                                           NULL,
                                           NULL,
                                           NULL);
        if(debugUTF8)
        {
   	    PrintEncodingLog("TextCodecGtk::decode has the following characters : %s \n",debugUTF8);
            g_free(debugUTF8);
        }
#endif
        ASSERT(static_cast<unsigned char>(resultCString[0]) == 0xff);
        ASSERT(static_cast<unsigned char>(resultCString[1]) == 0xfe);
        if(*(reinterpret_cast<UChar*>(resultCString)) == static_cast<UChar>(0xfeff))
        {
            memmove(resultCString, resultCString + 2, bytesWritten - 2);
            PrintEncodingLog("TextCodecGtk::decode, Bytes Converted SUCCESSFULLY from %s to UTF-16\n", m_encoding.name());

            bytesWritten -= 2;
            resultLen =  (bytesWritten / sizeof(UChar)) * sizeof(char);
            result.append(reinterpret_cast<UChar*>(resultCString), resultLen);
        }
        else
        {
            ASSERT(0);
            //result.append(static_cast<UChar>(0xfffd));
        }

        g_free(resultCString);
    }


    return String::adopt(result);
}

CString TextCodecGtk::encode(const UChar* characters, size_t length, UnencodableHandling handling)
{
    if(!characters || !length)
    {
        ASSERT(0);
        return "";
    }

    PrintEncodingLog("TextCodecGtk::encode entering, Encoding = %s\n",m_encoding.name());

    char fallback[2] = { 0, 0 };
    GError *error = NULL;
    gsize bytesRead, bytesWritten;
    switch (handling)
    {
        case QuestionMarksForUnencodables:
            fallback[0] = '?';
            break;
        case EntitiesForUnencodables:
            break;
        case URLEncodedEntitiesForUnencodables:
            break;

        default:
            ASSERT(0);
    }

    UChar* tempString = new UChar[length + 2];
    gssize  lengthInBytes = (length + 1) * sizeof(UChar);
    memcpy(static_cast<void*>(&tempString[1]) , characters, lengthInBytes );

    *(tempString) = 0xfeff;

    ASSERT(tempString);
    gchar* resultCString = g_convert_with_fallback (reinterpret_cast<const gchar*>(tempString),
            lengthInBytes,
            m_encoding.name(), "UTF-16",
            fallback,
            &bytesRead,
            &bytesWritten,
            &error);

#ifdef DEBUG
    gchar* outputStringUTF8 = g_utf16_to_utf8(tempString + 1, length, NULL, NULL, NULL);
    if(outputStringUTF8)
    {
        PrintEncodingLog("TextCodecGtk::encode: has the following characters : %s\n", outputStringUTF8);
        g_free(outputStringUTF8);
    }
#endif

    if(error)
    {
        ASSERT(0);
        PrintEncodingLog("TextCodecGtk::encode ERROR: %s\n",error->message);
        g_error_free(error);
    }

    delete tempString;
    if(!resultCString)
    {
        PrintEncodingLog("TextCodecGtk::encode FAILED in encoding %s\n", m_encoding.name());
        ASSERT(0);
        return "";
    }

    PrintEncodingLog("TextEncodingGtk::encode, Bytes SUCCESSFULLY Converted from UTF-16 (encode) to %s\n", m_encoding.name(), resultCString);
    CString result(resultCString, bytesWritten);
    g_free(resultCString);
    return result;
}




} //WebCore
