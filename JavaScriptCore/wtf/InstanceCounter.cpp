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

#include "InstanceCounter.h"
#include <wtf/Assertions.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#if OS(UNIX)
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#elif OS(WINDOWS)
#include <windows.h>
#include <winsock.h>
#endif

namespace WTF {

std::auto_ptr<char> InstanceCounter::s_outputFileName;

namespace {
#if OS(UNIX)
void platformWriteToFile(const char* const fileName, const void* const dataToWrite, size_t const numBytesToWrite)
{
    int const fd = open(fileName, O_CREAT | O_WRONLY, S_IREAD | S_IWRITE);
    ASSERT(fd >= 0);
    lseek(fd, 0, SEEK_END);
    write(fd, dataToWrite, numBytesToWrite);
    close(fd);
}

void platformDeleteFile(const char* const fileName)
{
    unlink(fileName);
}

#elif OS(WINDOWS)
void platformWriteToFile(const char* const fileName, const void* const dataToWrite, size_t const numBytesToWrite)
{
    HANDLE const fd = CreateFileA(fileName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    SetFilePointer(fd, 0, 0, FILE_END);
    DWORD bytesWritten = 0;
    WriteFile(fd, dataToWrite, numBytesToWrite, &bytesWritten, 0);
    CloseHandle(fd);
}

void platformDeleteFile(const char* const fileName)
{
    DeleteFileA(fileName);
}

#endif
}

void InstanceCounter::onConstruct(unsigned long* count, unsigned long* maxCount, t_dummyMethod const dummyMethod) throw()
{
    *count = *count + 1;
    ASSERT((*count) > 0);
    if (*count > *maxCount) {
        *maxCount = *count;
        if (s_outputFileName.get()) {
	#if CPU(X86_64)
            uint32_t const dummyMethodAsLong = (uint64_t)dummyMethod;
	#else
            uint32_t const dummyMethodAsLong = (uint32_t)dummyMethod;
	#endif
            uint32_t const dummyMethodAsLongNetworkByteOrder = htonl(dummyMethodAsLong);
            uint32_t const maxCountNetworkByteOrder = htonl(*maxCount);
            time_t const currentTime = time(0);
            uint32_t const currentTimeNetworkdByteOrder = htonl(static_cast<uint32_t>(currentTime));

            uint32_t const dataToWrite[] = { currentTimeNetworkdByteOrder, dummyMethodAsLongNetworkByteOrder, maxCountNetworkByteOrder };
            platformWriteToFile(s_outputFileName.get(), dataToWrite, sizeof(dataToWrite));

            
        }
    }
}


void InstanceCounter::onDestroy(unsigned long* count) throw()
{
    ASSERT((*count) > 0);
    *count = *count - 1;
}

void InstanceCounter::enableLogging(const char* const outputFileName)
{
    size_t const nameLen = strlen(outputFileName) + 1;
    s_outputFileName = std::auto_ptr<char>(new char[nameLen]);
    memcpy(s_outputFileName.get(), outputFileName, nameLen);
    platformDeleteFile(s_outputFileName.get());
}


}
