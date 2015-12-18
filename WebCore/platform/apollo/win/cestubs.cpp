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
#include <config.h>
#include <windows.h>
#include "cestubs.h"

extern "C" VOID WINAPI OutputDebugStringA(const char* lpOutputString)
{
    WebCore::String wStr = WebCore::String::fromUTF8(lpOutputString);
    if(!wStr.isEmpty())
        OutputDebugStringW(wStr.charactersWithNullTermination());
}

extern "C" HANDLE WINAPI CreateFileA(
    const char* lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    WebCore::String wStr = WebCore::String::fromUTF8(lpFileName);
    HANDLE ret = INVALID_HANDLE_VALUE;

    if(!wStr.isEmpty())
        CreateFileW(wStr.charactersWithNullTermination(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    return ret;
}

extern "C" BOOL WINAPI DeleteFileA(LPCSTR lpFileName)
{
    WebCore::String wStr = WebCore::String::fromUTF8(lpFileName);
    return DeleteFileW(wStr.charactersWithNullTermination());
}

extern "C" BOOL WINAPI CreateDirectoryA(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    BOOL bRet = FALSE;
    WebCore::String wStr = WebCore::String::fromUTF8(lpPathName);
    
    if(!wStr.isEmpty())
        bRet = CreateDirectoryW(wStr.charactersWithNullTermination(), lpSecurityAttributes);

    return bRet;
}

// in wince glyphs are actually characters
extern "C" BOOL WINAPI GetCharWidthI(HDC hdc, UINT first, UINT count, LPWORD glyphs, LPINT buffer) 
{
	ASSERT(glyphs == 0);	// Unsupported
    return GetCharWidth32(hdc, first, first + count - 1, buffer);

/*  this is the code if glyphs are actually glyphs

    ABC *abc; 
    unsigned int i; 
  
    if (!(abc = HeapAlloc(GetProcessHeap(), 0, count * sizeof(ABC)))) 
        return FALSE; 
 
    if (!GetCharABCWidthsI(hdc, first, count, glyphs, abc)) 
    { 
        HeapFree(GetProcessHeap(), 0, abc); 
        return FALSE; 
    } 
 
    for (i = 0; i < count; i++) 
        buffer[i] = abc->abcA + abc->abcB + abc->abcC; 
 
    HeapFree(GetProcessHeap(), 0, abc); 
    return TRUE; 
	*/
 } 