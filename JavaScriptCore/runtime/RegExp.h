/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (C) 2009 Torch Mobile, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef RegExp_h
#define RegExp_h

#include "UString.h"
#include "ExecutableAllocator.h"
#include <wtf/Forward.h>
#include <wtf/RefCounted.h>
#include "yarr/RegexJIT.h"
#include "yarr/RegexInterpreter.h"

struct JSRegExp;

namespace JSC {

    class JSGlobalData;

    class RegExp : public RefCounted<RegExp> {
    public:
        static PassRefPtr<RegExp> create(JSGlobalData* globalData, const UString& pattern);
        static PassRefPtr<RegExp> create(JSGlobalData* globalData, const UString& pattern, const UString& flags);
#if !ENABLE(YARR)
        ~RegExp();
#endif

        bool global() const { return (m_flagBits & Global) != 0; }
        bool ignoreCase() const { return (m_flagBits & IgnoreCase) != 0; }
        bool multiline() const { return (m_flagBits & Multiline) != 0; }

        const UString& pattern() const { return m_pattern; }

        bool isValid() const { return !m_constructionError; }
        const char* errorMessage() const { return m_constructionError; }

        int match(const UString&, int startOffset, Vector<int, 32>* ovector = 0);
        unsigned numSubpatterns() const { return m_numSubpatterns; }

    private:
        RegExp(JSGlobalData* globalData, const UString& pattern);
        RegExp(JSGlobalData* globalData, const UString& pattern, const UString& flags);

        void compile(JSGlobalData*);

        enum FlagBits { Global = 1, IgnoreCase = 2, Multiline = 4 };

        UString m_pattern; // FIXME: Just decompile m_regExp instead of storing this.
        int m_flagBits;
        const char* m_constructionError;
        unsigned m_numSubpatterns;
        UString m_lastMatchString;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
        int m_lastMatchStart;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        Vector<int, 32> m_lastOVector;

#if ENABLE(YARR_JIT)
        Yarr::RegexCodeBlock m_regExpJITCode;
#elif ENABLE(YARR)
        OwnPtr<Yarr::BytecodePattern> m_regExpBytecode;
#else
        JSRegExp* m_regExp;
#endif
    };

} // namespace JSC

#endif // RegExp_h
