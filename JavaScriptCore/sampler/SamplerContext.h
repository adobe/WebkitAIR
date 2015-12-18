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

#ifndef SamplerContext_h
#define SamplerContext_h

#if ENABLE(APOLLO_PROFILER)

#if OS(UNIX) && PLATFORM(APOLLO)
	#if HAVE(STDINT_H)
		#include <stdint.h>
	#elif HAVE(INTTYPES_H)
		#include <inttypes.h>
	#else
		#include <sys/types.h>
	#endif
#endif

#define SAMPLER_CONTEXT(context) JSC::EnterSamplerContext __SamplerContext(context)

namespace JSC {

    // This file defines a list of objects used to sample
    // some of the most CPU intensive tasks in WebKit
    // eg:
    //
    //     void doSomeWork()
    //     {
    //         SAMPLER_CONTEXT(JSC::SamplerContextPool::builtins.doSomeWork);
    //         
    //         ... do the hard work here ...
    //     }
    
    class SamplerApollo;
    
    class SamplerContext
    {
    public:
        SamplerContext(const char* name);
        const char* name() const { return m_name; }

    private:
        const char * m_name;
    };
    
    struct SamplerContextPool
    {
        SamplerContextPool();
        
        SamplerContext cssParse;
        SamplerContext cssStyle;
        SamplerContext htmlLayout;
        SamplerContext htmlPaint;
        SamplerContext jsParse;
        SamplerContext htmlParse;
        SamplerContext xmlParse;

        static void registerPool(SamplerApollo* sampler);
        static void unregisterPool(SamplerApollo* sampler);

        static SamplerContextPool builtins;
    };

    class EnterSamplerContext
    {
    public:
        EnterSamplerContext(SamplerContext* context);
        ~EnterSamplerContext();

    private:
        SamplerApollo* m_sampler;
        SamplerContext* m_samplerContext;
    };
 
}

#endif

#endif // SamplerContext_h
