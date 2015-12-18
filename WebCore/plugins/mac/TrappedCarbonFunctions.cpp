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

#include "config.h"
#include "npapi.h"
#if PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
#include "TrappedCarbonFunctions.h"
#include "macho/MachOImportTrap.h"
#include <wtf/Assertions.h>

#include <Carbon/Carbon.h>

namespace WebCore {

namespace {

::Boolean trappedIsWindowHilited(::WindowRef const window)
{
    return window ? ::IsWindowHilited(window) : true;
}

struct TrappedCarbonFunctionsTableEntry {
    const char* const functionName;
    const void* const functionAddress;
};

static const TrappedCarbonFunctionsTableEntry trappedCarbonFunctionsTable[] = {
    { "_IsWindowHilited", (const void*)trappedIsWindowHilited }
};

static const size_t numTrappedCarbonFunctions = sizeof(trappedCarbonFunctionsTable) / sizeof(TrappedCarbonFunctionsTableEntry);

static inline void trapCarbonFunction(const Dl_info* const info, const TrappedCarbonFunctionsTableEntry* const entry)
{
    ASSERT(info);
    ASSERT(entry);
    MachImage::ImportEntry* const importEntry = MachImage::getImportEntry(info, entry->functionName);
    ASSERT(importEntry);
    if (importEntry)
        MachImage::setImportEntryTarget(info, importEntry, entry->functionAddress);
}

}

void trapCarbonFunctions(const Dl_info* const info)
{
    ASSERT(info);
    for (unsigned i = 0; i < numTrappedCarbonFunctions; ++i) {
        trapCarbonFunction(info, &(trappedCarbonFunctionsTable[i]));
    }
}


}
#endif // PLATFORM(APOLLO) && !defined(NP_NO_CARBON)
