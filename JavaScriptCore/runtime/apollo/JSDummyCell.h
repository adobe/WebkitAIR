/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
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
#ifndef JSDummyCell_h
#define JSDummyCell_h

#if PLATFORM(APOLLO)

#include "JSCell.h"

namespace JSC {

// apollo - compile - integrate - 58803
// olaru:
// At WK rev 52176, the Number Heap was replaced by a statically allocated
// block of JSCell - this introduces code that instantiates dummy JSCells
// on the Heap as number placeholders.
// We want to keep JSCell abstract, along with our purely virtual profiler
// functions. Creating JSDummyCell - a dummy class to make it non-abstract
// and be and instantiate it instead of JSCell

class JSDummyCell : public JSCell{
		friend class Heap;
	public:
		virtual UString typeName() { return "DummyNumberCell"; }
        virtual size_t instanceSize() const { return sizeof(JSCell); }
        virtual void getMembersForProfiler(ProfilerMembersList*) { }
		
	private:
		explicit JSDummyCell(Structure* structure) : JSCell(structure) { }
};

} //namespace JSC

#endif //PLATFORM(APOLLO)

#endif

