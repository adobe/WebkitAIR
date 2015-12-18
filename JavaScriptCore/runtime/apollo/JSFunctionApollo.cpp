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
#include "config.h"
#include "JSFunction.h"

#if ENABLE(APOLLO_PROFILER)
#include "Nodes.h"
#include "Executable.h"
#endif

#if PLATFORM(APOLLO)

#include "ProfilerMembers.h"

namespace JSC {

#if ENABLE(APOLLO_PROFILER)
void JSFunction::getMembersForProfiler(ProfilerMembersList* members)
{
    if (!isHostFunction() && scopeChain().node()) {
        const UString activationString("[Activation]");
        int activationCount = 0;

        ScopeChainIterator scopeEnd = scopeChain().end();
        for (ScopeChainIterator scopeIter = scopeChain().begin(); scopeIter != scopeEnd; ++scopeIter, activationCount++) {
            JSObject* o = *scopeIter;
            members->append(makeString(activationString, UString::from(activationCount)), static_cast<JSCell*>(o));
        }
    }

    // apollo integrate
    if (!isHostFunctionNonInline()) {
        FunctionExecutable* funExecPtr = jsExecutable();
        //anicau: there used to be a codeblock reference passed here, information no longer available.
        //ProfilerMembersList::append uses the void* pointer to retrieve the associated sampler object.
        //based on the samplerObject->identifier() and "[Code Block]" string a new ProfilerMembersItem
        //is created and appended to the ProfilerMembersList::m_members list. 
        
        //Just now the sampler object is created directly on the FunctionExecutable.

        //anicau: TODO. Searched the entire codebase, couldn't find any constant named "[Code Block]"/"Code Block".
        //We need to determine if this is just a description or if it's some constant used to interface with other 
        //features/functionalities. 

        if (funExecPtr)
			members->append("[Code Block]", funExecPtr);
	}

    JSObject::getMembersForProfiler(members);
}
#endif


}

#endif

