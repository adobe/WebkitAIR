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

#ifndef MachOImage_h
#define MachOImage_h

#include <mach/machine.h>
#include <architecture/byte_order.h>
#include <unistd.h>

#include <mach-o/nlist.h>
struct load_command;
struct mach_header;
struct symtab_command;
struct dysymtab_command;
struct section;
struct segment_command;
namespace MachImage {

class MachOImage {
public:
    class LoadCommandIterator {
        friend class MachOImage;
    public:
        LoadCommandIterator(const LoadCommandIterator&);
        LoadCommandIterator& operator=(const LoadCommandIterator&);
        bool operator==(const LoadCommandIterator&);
        inline bool operator !=(const LoadCommandIterator& other) { return ! (*this == other); }
        
        const load_command* operator*() const;
        
        LoadCommandIterator operator++(int);
        LoadCommandIterator& operator++();
        
    private:
        LoadCommandIterator(const load_command* const, bool);
        const load_command* getNext() const;
        const load_command* m_command;
        bool m_bigEndian;
    };

    
    class SymbolTable {
        friend class MachOImage;
    public:
        SymbolTable(const SymbolTable&);
        SymbolTable& operator=(const SymbolTable&);
        void getEntry(struct nlist* const symDest, char const * * const symNameDest, off_t const n) const;
        inline uint32_t size() const { return m_nEntries; }
        inline bool bigEndian() const { return m_bigEndian; }
    private:
        SymbolTable(const struct nlist* const tableStart, uint32_t const nEntries, const char* const strTable, bool const bigEndian);
        const struct nlist* m_tableStart;
        uint32_t m_nEntries;
        const char* m_strTable;
        bool m_bigEndian;
    };
    
    class IndirectSymbolTable {
        friend class MachOImage;
    public:
        IndirectSymbolTable(const IndirectSymbolTable&);
        IndirectSymbolTable& operator=(const IndirectSymbolTable&);
        void getEntry(uint32_t* const entryFileOffset, uint32_t* const entryVMAddr, char const * * const entryName, off_t const n) const;
        uint32_t size() const;
    private:
        IndirectSymbolTable(const unsigned char* const imageStart
                           , const SymbolTable& symbols
                           , const uint32_t* const indirectSymbols
                           , const struct section* const tableSection);
                           
        uint32_t stride() const;
        const unsigned char* m_imageStart;
        SymbolTable m_symbols;
        const uint32_t* m_indirectSymbols;
        const struct section* m_tableSection;
    };


    inline MachOImage() : m_headerAddress(0) {}
    inline MachOImage(const void* headerAddress) : m_headerAddress(headerAddress) {}
    inline MachOImage(const MachOImage& other) : m_headerAddress(other.m_headerAddress) {}
    inline MachOImage& operator=(const MachOImage& other) { m_headerAddress = other.m_headerAddress; return *this; };
    
    inline bool valid() const { return bigEndian() || littleEndian(); }
    cpu_type_t cpuType() const;
    cpu_subtype_t cpuSubType() const;

    LoadCommandIterator loadCommandsBegin() const;
    LoadCommandIterator loadCommandsEnd() const;
    
    static inline uint32_t imageIntToHostInt(bool const isBigEndian, uint32_t const i)
    {
        return isBigEndian ? CFSwapInt32BigToHost(i) : CFSwapInt32LittleToHost(i);
    }

    static inline int32_t imageIntToHostInt(bool const isBigEndian, int32_t const i)
    {
        return static_cast<int32_t>(isBigEndian ? CFSwapInt32BigToHost(static_cast<uint32_t>(i)) : CFSwapInt32LittleToHost(static_cast<uint32_t>(i)));
    }
    
    static inline int16_t imageIntToHostInt(bool const isBigEndian, int16_t const i)
    {
        return static_cast<int16_t>(isBigEndian ? CFSwapInt16BigToHost(static_cast<uint16_t>(i)) : CFSwapInt16LittleToHost(static_cast<uint16_t>(i)));
    }
    
    bool getSymTabCommand(symtab_command* dest) const;
    bool getDySymTabCommand(dysymtab_command* dest) const;
    bool getLinkEditSegmentCommand(segment_command* dest) const;
    SymbolTable getSymbolTable() const;
    
    IndirectSymbolTable getIndirectSymbolTable() const;
    

private:
    off_t getLinkEditSlideAdjustment() const;
    inline const mach_header* header() const { return reinterpret_cast<const mach_header*>(m_headerAddress); }
    inline const unsigned char* imageStart() const { return reinterpret_cast<const unsigned char*>(m_headerAddress); }
    bool bigEndian() const;
    bool littleEndian() const;
    
    template <typename T>
    inline T imageIntToHostInt(T const i) const
    {
        return imageIntToHostInt(bigEndian(), i);
    }
    
    
    const uint32_t* getIndirectSymbolTableInfo() const;

    const void* m_headerAddress;
    
};

}


#endif
