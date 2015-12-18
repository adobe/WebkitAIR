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


#ifndef TextCodecGtk_h
#define TextCodecGtk_h

#include "TextCodec.h"
#include "TextEncoding.h"


namespace WebCore {

class TextCodecGtk : public TextCodec {
    public:
    static void registerBaseEncodingNames(EncodingNameRegistrar);
    static void registerBaseCodecs(TextCodecRegistrar);

    static void registerExtendedEncodingNames(EncodingNameRegistrar);
    static void registerExtendedCodecs(TextCodecRegistrar);

    TextCodecGtk(const TextEncoding&);
    virtual ~TextCodecGtk();

    virtual String decode(const char*, size_t length, bool flush, bool stopOnError, bool& sawError);
    virtual CString encode(const UChar*, size_t length, UnencodableHandling);

    private:
    char* gConvertWithContext(const char* str, 
                               gssize length,
                               const char* toCodeset,
                               const char* fromCodeset,
                               gsize *bytesRead,
                               gsize *bytesWritten,
                               bool flush,
                               bool stopOnError,
                               GError **error);



    //GTK style conversion functions
    //converts till there is a problem with the string
    gchar* gconvert_till_error(const char* str, 
                               gssize length,
                               const char* to_codeset,
                               const char* from_codeset,
                               gsize *bytes_read,
                               gsize *bytes_written,
                               GError **error);

    //converts the whole string skipping parts that it cannot convert
    gchar* gconvert_skipping_unconvertables(const char* str, 
                                          gssize length,
                                          const char* to_codeset,
                                          const char* from_codeset,
                                          gsize *bytes_read,
                                          gsize *bytes_written,
                                          GError **error);

    TextEncoding m_encoding;
    char* m_contextBytes;
    gssize m_contextLength;
};

} // namespace WebCore

#endif // TextCodecGtk_h
