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

#ifndef SamplerNodes_h
#define SamplerNodes_h

#if ENABLE(APOLLO_PROFILER)

#include <UString.h>
#include <wtf/RefCounted.h>


namespace JSC {

	class ExecState;
    class ScopeNode;
    class SamplerContext;
    class ProfilerMembersList;
    class JSGlobalObject;
	class JIT;

    class BaseSamplerNode: public WTF::RefCounted<BaseSamplerNode>
    {
		friend class JIT;
    public:
        BaseSamplerNode(void* ptr);
        BaseSamplerNode(uint64_t identifier);
        BaseSamplerNode(uint64_t identifier, uint64_t invocationCount); 
        virtual ~BaseSamplerNode();
        
        uint64_t identifier() const { return m_identifier; }
        void setIdentifier(uint64_t value) { m_identifier = value; }
        
        virtual UString objectType() = 0;
        
        virtual size_t objectSize(bool /*inConstructor*/ = false) const;
        virtual void getMembers(ProfilerMembersList*) const;

        // functions
        uint64_t invocationCount() const { return m_invocationCount; };
        void willExecute() { m_invocationCount ++; }
        virtual UString name();
        
        // called on deadNodes to avoid crashes when all object are deleted in bulk, with disregard to cross-reference
        virtual UString filename() const;
        
        virtual JSGlobalObject* globalObject() const;
        
        inline void* ptr() const { return m_ptr; }
		
#if ENABLE(JIT)
		// needed to track deleted BaseSamplerNode that are still referenced from 
		// jit code
		inline void clearPtr() { m_ptr = reinterpret_cast<void*>(-1); }
#endif

    private:
        uint64_t m_identifier;
        void* m_ptr;
        uint64_t m_invocationCount;
    };

#if ENABLE(JIT)
	// FakeSamplerNode is used to populate the jit code so that
	// every time it has a node to compare with
	class FakeSamplerNode: public BaseSamplerNode
	{
	public:
		FakeSamplerNode();
		virtual UString objectType();
	};
#endif
    
    template <typename DataType>
    class SamplerDataNode;
    
    // JSC::ScopeNode specialization
    
    class SamplerScopeNode: public BaseSamplerNode
    {
    public:
        SamplerScopeNode(ScopeNode* ptr);
        
        virtual UString objectType();
        size_t objectSize(bool /*inConstructor*/) const;
        virtual UString filename() const;
        virtual UString name() { return m_name; }
        virtual void setName(const UString &value) { m_name = value; }
        
        inline ScopeNode* impl() const { return static_cast<ScopeNode*>(ptr()); }
    private:
        UString     m_name;
    };

    // JSC::SamplerContext specialization
    template <>
    class SamplerDataNode<SamplerContext>: public BaseSamplerNode
    {
    public:
        SamplerDataNode<SamplerContext>(SamplerContext* ptr);

        virtual UString objectType();
        size_t objectSize(bool /*inConstructor*/) const;
        virtual UString name();
        
        inline SamplerContext* impl() const { return static_cast<SamplerContext*>(ptr()); }
    };

    class SamplerJSCellNode: public BaseSamplerNode
    {
    public:
        SamplerJSCellNode(uint64_t globalObjectIdentifier, JSCell* ptr, size_t size);
    
        virtual UString objectType();
        virtual size_t objectSize(bool inConstructor = false) const;
        virtual void getMembers(ProfilerMembersList*) const;
    
        virtual UString filename() const;
        virtual UString name();
    
        inline JSCell* impl() const { return static_cast<JSCell*>(ptr()); }
        inline void setObjectType(UString newType) { m_objectType = newType; }
        
        virtual JSGlobalObject* globalObject() const;
    
    private:
        ExecState* execState() const;
        uint64_t m_globalObjectIdentifier;
        size_t m_size;
        UString m_objectType;
    };

    class DeadSamplerNode: public BaseSamplerNode
    {
    public:
        static WTF::PassRefPtr<DeadSamplerNode> createWithDataFrom(BaseSamplerNode* from);
    
        virtual UString objectType() { return m_objectType; }
        virtual size_t objectSize() const { return m_objectSize; }
		virtual UString name();

        // functions
        virtual UString filename() const { return m_filename; }

    private:
        DeadSamplerNode(uint64_t identifier, UString nodeName, UString objectType, size_t objectSize, UString filename, uint64_t invocationCount);
            
        UString     m_objectType;
        size_t      m_objectSize;
        UString     m_filename;
        UString     m_name;
    };

} // namespace JSC

#endif // PLATFORM(APOLLO)

#endif // SamplerNodes_h
