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
#ifndef InstanceCounter_h
#define InstanceCounter_h

#include <wtf/Assertions.h>
#include <memory>

namespace WTF {

    class InstanceCounter
    {
    public:
        typedef void (*t_dummyMethod)();
        static void onConstruct(unsigned long* count, unsigned long* maxCount, t_dummyMethod const dummyMethodPtr) throw();
        static void onDestroy(unsigned long* count) throw();
        static void enableLogging(const char* const outputFileName);
    private:
        static std::auto_ptr<char> s_outputFileName;

        // Not implemented....
        InstanceCounter(const InstanceCounter&);
        InstanceCounter& operator=(const InstanceCounter&);
        ~InstanceCounter() throw();
    };


    template <class t_ClassToCount> class InstanceCounterTemplate
    {
    public:
        InstanceCounterTemplate() throw();
        ~InstanceCounterTemplate() throw();
        static void onConstruct() throw();
        static void onDestroy() throw();
    private:
        static unsigned long s_count;
        static unsigned long s_maxCount;

        static void dummyMethod() throw();

        // Not implemented....
        InstanceCounterTemplate(const InstanceCounterTemplate<t_ClassToCount>&);
        InstanceCounterTemplate<t_ClassToCount>& operator=(const InstanceCounterTemplate<t_ClassToCount>&);
    };

    template <class t_ClassToCount>
    inline InstanceCounterTemplate<t_ClassToCount>::InstanceCounterTemplate() throw()
    {
        onConstruct();
    }

    template <class t_ClassToCount>
    inline InstanceCounterTemplate<t_ClassToCount>::~InstanceCounterTemplate() throw()
    {
        onDestroy();
    }

    template <class t_ClassToCount>
    inline void InstanceCounterTemplate<t_ClassToCount>::onConstruct() throw()
    {
        InstanceCounter::onConstruct(&s_count, &s_maxCount, dummyMethod);
    }

    template <class t_ClassToCount>
    inline void InstanceCounterTemplate<t_ClassToCount>::onDestroy() throw()
    {
        InstanceCounter::onDestroy(&s_count);
    }

    template <class t_ClassToCount>
    void InstanceCounterTemplate<t_ClassToCount>::dummyMethod() throw()
    {
        // can not be empty to avoid code folding.
        // should never be called.
        ASSERT(false);
        s_count = 0;
    }

    template <class t_ClassToCount>
    unsigned long InstanceCounterTemplate<t_ClassToCount>::s_count = 0;

    template <class t_ClassToCount>
    unsigned long InstanceCounterTemplate<t_ClassToCount>::s_maxCount = 0;

#if ENABLE(INSTANCE_COUNTS)
#define INSTANCE_COUNTER(className) WTF::InstanceCounterTemplate<className> m_instanceCounter
#define INSTANCE_COUNTER_CONSTRUCT(className) WTF::InstanceCounterTemplate<className>::onConstruct()
#define INSTANCE_COUNTER_DESTRUCT(className) WTF::InstanceCounterTemplate<className>::onDestroy()
#else
#define INSTANCE_COUNTER(className)
#define INSTANCE_COUNTER_CONSTRUCT(className)
#define INSTANCE_COUNTER_DESTRUCT(className)
#endif

    
}

#endif
