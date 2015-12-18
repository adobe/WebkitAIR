/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Adobe Systems Incorporated.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef ProfilerMembers_h
#define ProfilerMembers_h

#if ENABLE(APOLLO_PROFILER)

#include "UString.h"

namespace JSC {

    class ExecState;
	class SamplerApollo;

    // The sampler uses this classes to iterate over the members of the objects.
    // This is similar to the Property name iterator from JSC, but it is designed
    // to work with any kind of object (not only JSObjects).

    struct ProfilerMembersItem
    {
        ProfilerMembersItem(const UString& name, uint64_t identifier)
            : name(name)
            , identifier(identifier)
        {
        }

        UString name;
        uint64_t identifier;
    };

    class ProfilerMembersList
    {
    public:
        typedef WTF::Vector<ProfilerMembersItem> Members;
        typedef WTF::Vector<ProfilerMembersItem>::const_iterator const_iterator;

        ProfilerMembersList(SamplerApollo* sampler);

        void append(const UString& name, void* ptr);
        const Members* members() const { return &m_members; }
        const_iterator begin() const { return m_members.begin(); }
        const_iterator end() const { return m_members.end(); }

        bool isEmpty() const { return m_members.isEmpty(); }
        size_t size() const { return m_members.size(); }

        // when enumerating JSObject an exec state is needed.
        // for that reason the exec state can be embedded by the 
        // specialized implementation of getMembers
        void setExec(ExecState* exec) { m_exec = exec; }
        ExecState* exec() const { return m_exec; }

    private:
        ExecState* m_exec;
        SamplerApollo* m_sampler;
        Members m_members;
    };

} // namespace JSC

#endif // PLATFORM(APOLLO)

#endif // ProfilerMembers_h
