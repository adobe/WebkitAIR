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
 
#include "MachOImage.h"
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <stddef.h>
#include <string.h>
namespace MachImage {


MachOImage::LoadCommandIterator::LoadCommandIterator(const load_command* const commandAddress, bool const bigEndian)
    : m_command(commandAddress)
    , m_bigEndian(bigEndian)
{
}

MachOImage::LoadCommandIterator::LoadCommandIterator(const MachOImage::LoadCommandIterator& other)
    : m_command(other.m_command)
    , m_bigEndian(other.m_bigEndian)
{
}

MachOImage::LoadCommandIterator& MachOImage::LoadCommandIterator::operator=(const MachOImage::LoadCommandIterator& other)
{
    m_command = other.m_command;
    m_bigEndian = other.m_bigEndian;
    return *this;
}

bool MachOImage::LoadCommandIterator::operator==(const LoadCommandIterator& other)
{
    return m_command == other.m_command;
}

const load_command* MachOImage::LoadCommandIterator::operator*() const
{
    return m_command;
}
        
MachOImage::LoadCommandIterator MachOImage::LoadCommandIterator::operator++(int)
{
    MachOImage::LoadCommandIterator const result(*this);
    m_command = getNext();
    return result;
}

MachOImage::LoadCommandIterator& MachOImage::LoadCommandIterator::operator++()
{
    m_command = getNext();
    return *this;
}

const load_command* MachOImage::LoadCommandIterator::getNext() const
{
    uint32_t const currCommandSize = MachOImage::imageIntToHostInt(m_bigEndian, m_command->cmdsize);
    const unsigned char* const currCommandBytes = reinterpret_cast<const unsigned char*>(m_command);
    const unsigned char* const nextCommandBytes = currCommandBytes + currCommandSize;
    return reinterpret_cast<const load_command*>(nextCommandBytes);
}


MachOImage::SymbolTable::SymbolTable(const MachOImage::SymbolTable& other)
    : m_tableStart(other.m_tableStart)
    , m_nEntries(other.m_nEntries)
    , m_strTable(other.m_strTable)
    , m_bigEndian(other.m_bigEndian)
{
}

MachOImage::SymbolTable& MachOImage::SymbolTable::operator=(const MachOImage::SymbolTable& other)
{
    m_tableStart = other.m_tableStart;
    m_nEntries = other.m_nEntries;
    m_strTable = other.m_strTable;
    m_bigEndian = other.m_bigEndian;
    return *this;
}

void MachOImage::SymbolTable::getEntry(struct nlist* const dest, char const * * const symNameDest, off_t const n) const
{
    dest->n_un.n_strx = MachOImage::imageIntToHostInt(m_bigEndian, m_tableStart[n].n_un.n_strx);
    dest->n_type = m_tableStart[n].n_type;
    dest->n_sect = m_tableStart[n].n_sect;
    dest->n_desc = MachOImage::imageIntToHostInt(m_bigEndian, m_tableStart[n].n_desc);
    dest->n_value = MachOImage::imageIntToHostInt(m_bigEndian, m_tableStart[n].n_value);
    *symNameDest = m_strTable + dest->n_un.n_strx;
}

MachOImage::SymbolTable::SymbolTable(const struct nlist* const tableStart, uint32_t const nEntries, const char* const strTable, bool const bigEndian)
    : m_tableStart(tableStart)
    , m_nEntries(nEntries)
    , m_strTable(strTable)
    , m_bigEndian(bigEndian)
{
}


MachOImage::IndirectSymbolTable::IndirectSymbolTable(const MachOImage::IndirectSymbolTable& other)
    : m_imageStart(other.m_imageStart)
    , m_symbols(other.m_symbols)
    , m_indirectSymbols(other.m_indirectSymbols)
    , m_tableSection(other.m_tableSection)
{
}

MachOImage::IndirectSymbolTable& MachOImage::IndirectSymbolTable::operator=(const MachOImage::IndirectSymbolTable& other)
{
    m_imageStart = other.m_imageStart;
    m_symbols = other.m_symbols;
    m_indirectSymbols = other.m_indirectSymbols;
    m_tableSection = other.m_tableSection;
    return *this;
}

namespace {
    static const char str_LOCAL[] = "LOCAL";
    static const char str_LOCALABSOLUTE[] = "LOCAL ABSOLUTE";
}

void MachOImage::IndirectSymbolTable::getEntry(uint32_t* const entryFileOffset, uint32_t* const entryVMAddr, char const * * const entryName, off_t const n) const
{
    uint32_t const indirectSymbolOffset = MachOImage::imageIntToHostInt(m_symbols.bigEndian(), m_tableSection->reserved1);
    uint64_t const indirectSymbolIndex = n + indirectSymbolOffset;
    uint32_t const indirectSymbolInfo = MachOImage::imageIntToHostInt(m_symbols.bigEndian(), m_indirectSymbols[indirectSymbolIndex]);
    if (indirectSymbolInfo == INDIRECT_SYMBOL_LOCAL)
        *entryName = str_LOCAL;
    else if (indirectSymbolInfo == (INDIRECT_SYMBOL_LOCAL | INDIRECT_SYMBOL_ABS))
        *entryName = str_LOCALABSOLUTE;
    else {
        struct nlist symbolEntry;
        m_symbols.getEntry(&symbolEntry, entryName, indirectSymbolInfo);
    }
    uint32_t const entryOffset = static_cast<uint32_t>(n) * stride();
    *entryFileOffset = MachOImage::imageIntToHostInt(m_symbols.bigEndian(), m_tableSection->offset) + entryOffset;
    *entryVMAddr = MachOImage::imageIntToHostInt(m_symbols.bigEndian(), m_tableSection->addr) + entryOffset;
}

uint32_t MachOImage::IndirectSymbolTable::size() const
{
    uint32_t const nEntries = MachOImage::imageIntToHostInt(m_symbols.bigEndian(), m_tableSection->size) / stride();
    return nEntries;
}

MachOImage::IndirectSymbolTable::IndirectSymbolTable(const unsigned char* const imageStart
                                                    , const SymbolTable& symbols
                                                    , const uint32_t* const indirectSymbols
                                                    , const section* const tableSection)
    : m_imageStart(imageStart)
    , m_symbols(symbols)
    , m_indirectSymbols(indirectSymbols)
    , m_tableSection(tableSection)
{
}

uint32_t MachOImage::IndirectSymbolTable::stride() const
{
    bool const stubs = (MachOImage::imageIntToHostInt(m_symbols.bigEndian(), m_tableSection->flags) & SECTION_TYPE) == S_SYMBOL_STUBS;
    return stubs ? MachOImage::imageIntToHostInt(m_symbols.bigEndian(), m_tableSection->reserved2) : sizeof(uint32_t);
}

namespace {
#if defined(__BIG_ENDIAN__)
static const uint32_t bigEndianMagicNumber = MH_MAGIC;
static const uint32_t littleEndianMagicNumber = MH_CIGAM;
#else
static const uint32_t bigEndianMagicNumber = MH_CIGAM;
static const uint32_t littleEndianMagicNumber = MH_MAGIC;
#endif
}

MachOImage::LoadCommandIterator MachOImage::loadCommandsBegin() const
{
    const mach_header* const beyondHeader = header() + 1;
    return MachOImage::LoadCommandIterator(reinterpret_cast<const load_command*>(beyondHeader), bigEndian());
}

MachOImage::LoadCommandIterator MachOImage::loadCommandsEnd() const
{
    const mach_header* const theHeader = header();
    const mach_header* const beyondHeader = theHeader + 1;
    const unsigned char* const loadCommandsBytes = reinterpret_cast<const unsigned char*>(beyondHeader);
    uint32_t const numLoadCommandsBytes = imageIntToHostInt(theHeader->sizeofcmds);
    const unsigned char* const endOfLoadCommandBytes = loadCommandsBytes + numLoadCommandsBytes;
    return MachOImage::LoadCommandIterator(reinterpret_cast<const load_command*>(endOfLoadCommandBytes), bigEndian());
}

bool MachOImage::getSymTabCommand(symtab_command* dest) const
{
    if (!dest)
        return false;
        
    MachOImage::LoadCommandIterator currCommand = loadCommandsBegin();
    MachOImage::LoadCommandIterator const commandsEnd = loadCommandsEnd();
    
    bool found = false;
    while ((!found) && (currCommand != commandsEnd)) {
        const load_command* const command = *currCommand;
        found = imageIntToHostInt(command->cmd) == LC_SYMTAB;
        if (!found)
            ++currCommand;
    }
    
    if (found) {
        const symtab_command* const rawSymCommand = reinterpret_cast<const symtab_command*>(*currCommand);
        dest->cmd = imageIntToHostInt(rawSymCommand->cmd);
        dest->cmdsize = imageIntToHostInt(rawSymCommand->cmdsize);
        dest->symoff = imageIntToHostInt(rawSymCommand->symoff);
        dest->nsyms = imageIntToHostInt(rawSymCommand->nsyms);
        dest->stroff = imageIntToHostInt(rawSymCommand->stroff);
        dest->strsize = imageIntToHostInt(rawSymCommand->strsize);
    }
    return found;
}


bool MachOImage::getDySymTabCommand(dysymtab_command* dest) const
{
    if (!dest)
        return false;
        
    MachOImage::LoadCommandIterator currCommand = loadCommandsBegin();
    MachOImage::LoadCommandIterator const commandsEnd = loadCommandsEnd();
    
    bool found = false;
    while ((!found) && (currCommand != commandsEnd)) {
        const load_command* const command = *currCommand;
        found = imageIntToHostInt(command->cmd) == LC_DYSYMTAB;
        if (!found)
            ++currCommand;
    }
    
    if (found) {
        const dysymtab_command* const rawSymCommand = reinterpret_cast<const dysymtab_command*>(*currCommand);
        dest->cmd = imageIntToHostInt(rawSymCommand->cmd);
        dest->cmdsize = imageIntToHostInt(rawSymCommand->cmdsize);
        dest->ilocalsym = imageIntToHostInt(rawSymCommand->ilocalsym);
        dest->nlocalsym = imageIntToHostInt(rawSymCommand->nlocalsym);
        dest->iextdefsym = imageIntToHostInt(rawSymCommand->iextdefsym);
        dest->nextdefsym = imageIntToHostInt(rawSymCommand->nextdefsym);
        dest->iundefsym = imageIntToHostInt(rawSymCommand->iundefsym);
        dest->nundefsym = imageIntToHostInt(rawSymCommand->nundefsym);
        dest->tocoff = imageIntToHostInt(rawSymCommand->tocoff);
        dest->ntoc = imageIntToHostInt(rawSymCommand->ntoc);
        dest->modtaboff = imageIntToHostInt(rawSymCommand->modtaboff);
        dest->nmodtab = imageIntToHostInt(rawSymCommand->nmodtab);
        dest->extrefsymoff = imageIntToHostInt(rawSymCommand->extrefsymoff);
        dest->nextrefsyms = imageIntToHostInt(rawSymCommand->nextrefsyms);
        dest->indirectsymoff = imageIntToHostInt(rawSymCommand->indirectsymoff);
        dest->nindirectsyms = imageIntToHostInt(rawSymCommand->nindirectsyms);
        dest->extreloff = imageIntToHostInt(rawSymCommand->extreloff);
        dest->nextrel = imageIntToHostInt(rawSymCommand->nextrel);
        dest->locreloff = imageIntToHostInt(rawSymCommand->locreloff);
        dest->nlocrel = imageIntToHostInt(rawSymCommand->nlocrel);
    }
    return found;
}

static size_t const maxSegNameSize = 16;

bool MachOImage::getLinkEditSegmentCommand(segment_command* dest) const
{
    if (!dest)
        return false;
    MachOImage::LoadCommandIterator currCommand = loadCommandsBegin();
    MachOImage::LoadCommandIterator const commandsEnd = loadCommandsEnd();
    
    const segment_command* rawSegCommand = 0;
    while ((!rawSegCommand) && (currCommand != commandsEnd)) {
        const load_command* const command = *currCommand;
        if (imageIntToHostInt(command->cmd) == LC_SEGMENT) {
            rawSegCommand = reinterpret_cast<const segment_command*>(command);
            if(strncasecmp(rawSegCommand->segname, "__LINKEDIT", maxSegNameSize) != 0)
                rawSegCommand = 0;
        }
        if (!rawSegCommand)
            ++currCommand;
    }
    if (rawSegCommand) {
        dest->cmd = imageIntToHostInt(rawSegCommand->cmd);
        dest->cmdsize = imageIntToHostInt(rawSegCommand->cmdsize);
        dest->vmaddr = imageIntToHostInt(rawSegCommand->vmaddr);
        dest->vmsize = imageIntToHostInt(rawSegCommand->vmsize);
        dest->fileoff = imageIntToHostInt(rawSegCommand->fileoff);
        dest->filesize = imageIntToHostInt(rawSegCommand->filesize);
        dest->maxprot = imageIntToHostInt(rawSegCommand->maxprot);
        dest->initprot = imageIntToHostInt(rawSegCommand->initprot);
        dest->nsects = imageIntToHostInt(rawSegCommand->nsects);
        dest->flags = imageIntToHostInt(rawSegCommand->flags);
        memcpy(dest->segname, rawSegCommand->segname, maxSegNameSize);
        return true;
    }
    return false;
}

const uint32_t* MachOImage::getIndirectSymbolTableInfo() const
{
    dysymtab_command dySymCommand;
    getDySymTabCommand(&dySymCommand);
    const unsigned char* const indirectSymbolsStart = imageStart() + dySymCommand.indirectsymoff + getLinkEditSlideAdjustment();
    return reinterpret_cast<const uint32_t*>(indirectSymbolsStart);
}

namespace {
    static const char __IMPORT[] = "__IMPORT";
    static const char __DATA[] = "__DATA";
}

MachOImage::IndirectSymbolTable MachOImage::getIndirectSymbolTable() const
{
    MachOImage::LoadCommandIterator currCommand(loadCommandsBegin());
    MachOImage::LoadCommandIterator const commandsEnd(loadCommandsEnd());
    const struct section* stubsOrPointersSection = 0;
    while ((!stubsOrPointersSection) && (currCommand != commandsEnd)) {
        const load_command* const lc = *currCommand;
        if (imageIntToHostInt(lc->cmd) == LC_SEGMENT) {
            const segment_command* const segCommand = reinterpret_cast<const segment_command*>(lc);
            uint32_t const nSections = imageIntToHostInt(segCommand->nsects);
            if (nSections) {
                const char* const segmentName = segCommand->segname;
                bool const isImportSegment = strcmp(segmentName, __IMPORT) == 0;
                bool const isDataSegment = strcmp(segmentName, __DATA) == 0;
                if (isImportSegment || isDataSegment) {
                    const segment_command* const beyondSegCommand = segCommand + 1;
                    const struct section* currSection = reinterpret_cast<const struct section*>(beyondSegCommand);
                    unsigned i = 0;
                    while ((!stubsOrPointersSection) && (i < nSections)) {
                        uint32_t const sectionFlags = imageIntToHostInt(currSection->flags);
                        uint32_t const sectionType = sectionFlags & SECTION_TYPE;
                        if ((isImportSegment && (sectionType == S_SYMBOL_STUBS)) || (isDataSegment && (sectionType == S_LAZY_SYMBOL_POINTERS)))
                            stubsOrPointersSection = currSection;
                        ++currSection;
                        ++i;
                    }
                }
            }
                
        }
        ++currCommand;
    }
    return MachOImage::IndirectSymbolTable(0, getSymbolTable(), getIndirectSymbolTableInfo(), stubsOrPointersSection);
}

MachOImage::SymbolTable MachOImage::getSymbolTable() const
{
    symtab_command symCommand;
    getSymTabCommand(&symCommand);
    
    off_t const linkEditSlideAdjustment = getLinkEditSlideAdjustment();
    
    const unsigned char* const symbolsStart = imageStart() + symCommand.symoff + linkEditSlideAdjustment;
    const char* const strTableStart = reinterpret_cast<const char*>(imageStart() + symCommand.stroff + linkEditSlideAdjustment);
    return MachOImage::SymbolTable(reinterpret_cast<const struct nlist*>(symbolsStart), symCommand.nsyms, strTableStart, bigEndian());
}


off_t MachOImage::getLinkEditSlideAdjustment() const
{
    off_t linkEditSlideAdjustment = 0;

    intptr_t vmAddrSlide = 0;

    uint32_t const numLoadedImages = _dyld_image_count();
    uint32_t currImageIndex = 0;
    while ((!vmAddrSlide) && (currImageIndex < numLoadedImages)) {
        const mach_header* const currHeader = _dyld_get_image_header(currImageIndex);
        if (currHeader == m_headerAddress)
            vmAddrSlide = _dyld_get_image_vmaddr_slide(currImageIndex);
        ++currImageIndex;
    }
    
    if (vmAddrSlide) {
        segment_command linkEditSegment;
        getLinkEditSegmentCommand(&linkEditSegment);
        
        linkEditSlideAdjustment = (linkEditSegment.vmaddr + vmAddrSlide - linkEditSegment.fileoff) - reinterpret_cast<intptr_t>(imageStart());
        
    }
    return linkEditSlideAdjustment;  
    
}

bool MachOImage::bigEndian() const  {
    const mach_header* const theHeader = header();
    if (!theHeader)
        return false;
    return theHeader->magic == bigEndianMagicNumber;
}

bool MachOImage::littleEndian() const  {
    const mach_header* const theHeader = header();
    if (!theHeader)
        return false;
    return theHeader->magic == littleEndianMagicNumber;
}


cpu_type_t MachOImage::cpuType() const
{
    const mach_header* const theHeader = header();
    return imageIntToHostInt(theHeader->cputype);
}

cpu_subtype_t MachOImage::cpuSubType() const
{
    const mach_header* const theHeader = header();
    return imageIntToHostInt(theHeader->cpusubtype);
}

}
