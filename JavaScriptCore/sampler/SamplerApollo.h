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

#ifndef SamplerApollo_h
#define SamplerApollo_h

#if ENABLE(APOLLO_PROFILER)

#include "SamplerNodes.h"

// to enable sampler logging set DEBUG_SAMPLER_NODES 1 in the Platform.h

namespace JSC {

    class JSGlobalObject;
    class SamplerApollo;
    class BaseSamplerNode;
    class SamplerContext;
    class ScriptExecutable;

	typedef WTF::HashMap<uint64_t, WTF::RefPtr<BaseSamplerNode> > SamplerNodesMap;
	typedef WTF::HashMap<void*, WTF::RefPtr<BaseSamplerNode> > LiveNodesMap;

    class SamplerApollo {
    public:
        static SamplerApollo** enabledSamplerReference()
        {
            return &s_sharedEnabledSamplerReference;
        }
        
        SamplerApollo();
        virtual ~SamplerApollo();

        static SamplerApollo* samplerInstance() { return s_samplerInstance; }
        static bool enabled() { return s_sharedEnabledSamplerReference != 0; }

        // Methods needed to update the callstack
        BaseSamplerNode* willExecute(ExecState*, JSValue function);                         // called from Interpreter
        BaseSamplerNode* willExecute(ExecState*, JSValue function, int32_t& lineNo);		// called from  CTI (should fill the lineNo)
		void willExecuteCached(ExecState*, BaseSamplerNode* functionNode, int32_t lineNo);  // called from Interpreter or CTI
        void willExecute(ExecState*, ScriptExecutable* scriptExecutable);
        void willExecute(SamplerContext* samplerContext);   // called from c++ functions like Paint, Layout, CSS etc.
        
        void didExecute(ExecState*, JSValue function);						// called from Interpreter
		void didExecuteCached(ExecState*, BaseSamplerNode* functionNode);   // called from Interpreter or CTI
        void didExecute(ExecState*, ScriptExecutable* scriptExecutable);
        void didExecute(SamplerContext* samplerContext);    // called from c++ functions like Paint, Layout, CSS etc.
        
        // Methods needs to enable constructor naming (like Circle in "new Circle()")
        BaseSamplerNode* willConstruct(ExecState*, JSValue function);                   // called from Interpreter
        BaseSamplerNode* willConstruct(ExecState*, JSValue function, int32_t& lineNo); // (should fill the lineNo)
		void willConstructCached(ExecState*, BaseSamplerNode* functionNode, int32_t lineNo);

        bool registerNode(WTF::PassRefPtr<BaseSamplerNode> node);
        void unregisterNode(WTF::PassRefPtr<BaseSamplerNode> node);

        // Helper method that call unregister for a node based on the object ptr
        void unregisterNodeHelper(void* ptr);
        
        // specialized register method for JSCell
        void registerJSCell(JSGlobalObject* globalObject, JSCell* ptr, size_t size);

        // called from Sampler when it doesn't need
        // the data about dead objects anymore
        void clearDeadObjects();

        // looks up a Node based on its pointer
        BaseSamplerNode* getSamplerNodeFor(void* ptr);

#if ENABLE(JIT)
		static FakeSamplerNode* getFakeSamplerNode() { return s_sharedFakeSamplerNode.get(); }
#endif

        // looks up a Node based on its identifier
        BaseSamplerNode* getSamplerNode(uint64_t identifier);

    protected:
        void enableSampler();
        void disableSampler();

        // true when not paused
        bool samplingNow;
        
    public:
        // the following methods are implemented in ApolloWebKit
        virtual void samplerDidEnterFunction(uint64_t identifier, int32_t lineno) = 0;
        virtual void samplerDidExitFunction(uint64_t identifier) = 0;

        virtual void atStatement(int line) = 0;

        // this method is also responsible to generate and return the identifier
        virtual uint64_t samplerDidAllocate(BaseSamplerNode* node) = 0;
        
        // if samplerWillDeallocate returns false we will not copy the dead object properties
        virtual bool samplerWillDeallocate(BaseSamplerNode* node) = 0;
    
    public:

#if ENABLE(DEBUG_SAMPLER_NODES)
        static void pushLineToLog(const char* value);
#endif

    private:
        LiveNodesMap m_liveNodes;
        SamplerNodesMap m_liveNodesByIdentifier;
        
        // keep them separate so that we can clean it at once
        SamplerNodesMap m_deadNodesByIdentifier;
        
        WTF::RefPtr<BaseSamplerNode> m_cachedSearchNode;
    
        static SamplerApollo* s_sharedEnabledSamplerReference;
        static SamplerApollo* s_samplerInstance;
        
        WTF::RefPtr<SamplerJSCellNode> m_constructorNode;

#if ENABLE(JIT)
		static WTF::RefPtr<FakeSamplerNode> s_sharedFakeSamplerNode;
#endif

#if ENABLE(DEBUG_SAMPLER_NODES)
        static FILE* s_debugFile;
#endif
    };
    
} // namespace JSC

#endif // PLATFORM(APOLLO)

#endif // SamplerApollo_h
