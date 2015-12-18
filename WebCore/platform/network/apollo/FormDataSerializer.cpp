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

#include <stdio.h>
#include <config.h>
#include "FormDataSerializer.h"
#include <wtf/text/CString.h>

#if PLATFORM(APOLLO_WINDOWS)
#include <windows.h>
#endif

namespace WebCore {

void FormDataSerializer::serialize(Vector<char>& data) const
{
    data.clear();
    
    const Vector<FormDataElement>& elements = m_formData.elements();
    size_t n = elements.size();
    for (size_t i = 0; i < n; ++i) 
    {
        const FormDataElement& e = elements[i];
        if (e.m_type == FormDataElement::data) 
        {
            // In memory content
            size_t oldSize = data.size();
            size_t delta = e.m_data.size();
            data.resize(oldSize + delta);
            memcpy(data.data() + oldSize, e.m_data.data(), delta);
        }
        else if(e.m_type == FormDataElement::encodedFile)
        {
            // File attachment content
            appendFileContents(e.m_filename, data);
        }
    }
}

// Is there really no platform file sys code!?!?!
#if OS(DARWIN)
void FormDataSerializer::appendFileContents(const String &filePath, Vector<char>& data) const
{
    // Default encoding on OSX is UTF-8
    FILE *f = fopen((const char *)filePath.utf8().data(), "rb");
    if(f != NULL)
    {
        // Determine file size
        fseek(f, 0, SEEK_END);
        size_t fileSize = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        size_t oldSize = data.size();
        data.resize(oldSize + fileSize);
        
        void *current = data.data() + oldSize;
        fread(current, fileSize, 1, f);
        fclose(f);
    }
}

#elif OS(WINDOWS)

void FormDataSerializer::appendFileContents(const String &filePath, Vector<char>& data) const
{
    String pathCopy(filePath);

    // Need to use the unicode Open and UTF-16
    WCHAR *fileName = (WCHAR *)pathCopy.charactersWithNullTermination();
    HANDLE f = ::CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(f != INVALID_HANDLE_VALUE)
    {
        // Determine file size
        DWORD fileSize = ::GetFileSize(f, NULL);
        
        size_t oldSize = data.size();
        data.resize(oldSize + fileSize);
        
        void *current = data.data() + oldSize;
        
        DWORD nBytesRead = 0;
        ::ReadFile(f, current, fileSize, &nBytesRead, NULL);
        ::CloseHandle(f);
    }
}

#elif OS(UNIX)

void FormDataSerializer::appendFileContents(const String &filePath, Vector<char>& data) const
{
    //Copied this code from the mac version
    //Assuming the default encoding as UTF-8
    FILE *f = fopen((const char *)filePath.utf8().data(), "rb");
    if(f != NULL)
    {
        // Determine file size
        fseek(f, 0, SEEK_END);
        size_t fileSize = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        size_t oldSize = data.size();
        data.resize(oldSize + fileSize);
        
        void *current = data.data() + oldSize;
        fread(current, fileSize, 1, f);
        fclose(f);
    }
}

#else
    // Need to provide a file read proc here!
#endif 

} // namespace WebCore
