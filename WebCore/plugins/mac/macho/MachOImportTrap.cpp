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

#include "MachOImportTrap.h"
#include "MachOImage.h"

#include <string.h>
#include <dlfcn.h>

namespace MachImage {

ImportEntry* getImportEntry(const Dl_info* const dlInfo, const char* const importName)
{
    const void* const imageBase = dlInfo->dli_fbase;
    MachOImage machImage(imageBase);
    MachOImage::IndirectSymbolTable iSymTable(machImage.getIndirectSymbolTable());
    
    uint32_t const nSymbols = iSymTable.size();
    uint32_t i = 0;
    const unsigned char* functionAddress = 0;
    while ((!functionAddress) && (i < nSymbols)) {
        uint32_t entryOffset;
        uint32_t entryVMAddress;
        const char* entryName;
        iSymTable.getEntry(&entryOffset, &entryVMAddress, &entryName, i);
        if (strcmp(entryName, importName) == 0) {
            functionAddress = reinterpret_cast<unsigned char*>(dlInfo->dli_fbase) + entryVMAddress;
        }
        ++i;
    }
    return reinterpret_cast<ImportEntry*>(const_cast<unsigned char*>(functionAddress));
}

const void* getImportEntryTarget(const Dl_info* const dlInfo, const ImportEntry* const entry)
{
    const void* const imageBase = dlInfo->dli_fbase;
    MachOImage machImage(imageBase);
    cpu_type_t cpuType = machImage.cpuType();
    if (cpuType == CPU_TYPE_X86) {
        cpu_subtype_t const cpuSubType = machImage.cpuSubType();
        if (cpuSubType == CPU_SUBTYPE_I386_ALL) {
            const unsigned char* const entryBytes = reinterpret_cast<const unsigned char*>(entry);
            const unsigned char* const offsetBytes = entryBytes + 1;
            const uint32_t* const offset = reinterpret_cast<const uint32_t*>(offsetBytes);
            const unsigned char* const targetBytes = entryBytes + *offset + 5;
            return targetBytes;
        }
    }
    else if (cpuType == CPU_TYPE_POWERPC) {
        cpu_subtype_t const cpuSubType = machImage.cpuSubType();
        if (cpuSubType == CPU_SUBTYPE_POWERPC_ALL) {
            const void * const * const target = reinterpret_cast<const void* const *>(entry);
            return *target;
        }
    }
    return 0;
}

void setImportEntryTarget(const Dl_info* const dlInfo, ImportEntry* const entry, const void* const newTarget)
{
    const void* const imageBase = dlInfo->dli_fbase;
    MachOImage machImage(imageBase);
    cpu_type_t cpuType = machImage.cpuType();
    if (cpuType == CPU_TYPE_X86) {
        cpu_subtype_t const cpuSubType = machImage.cpuSubType();
        if (cpuSubType == CPU_SUBTYPE_I386_ALL) {
            unsigned char* const entryBytes = reinterpret_cast<unsigned char*>(entry);
            uint32_t const offsetValue = reinterpret_cast<const unsigned char*>(newTarget) - (entryBytes + 5);
            unsigned char* const offsetBytes = entryBytes + 1;
            uint32_t* const offset = reinterpret_cast<uint32_t*>(offsetBytes);
            *offset = offsetValue;
        }
    }
    else if (cpuType == CPU_TYPE_POWERPC) {
        cpu_subtype_t const cpuSubType = machImage.cpuSubType();
        if (cpuSubType == CPU_SUBTYPE_POWERPC_ALL) {
            void const * * const target = reinterpret_cast<void const * *>(entry);
            *target = newTarget;
        }
    }
}
}
