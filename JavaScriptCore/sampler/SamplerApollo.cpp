/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "config.h"

#if ENABLE(APOLLO_PROFILER)

#include "SamplerApollo.h"
#include "JSFunction.h"
#include "SamplerContext.h"
#include "SamplerNodes.h"
#include "Executable.h"

namespace JSC {

SamplerApollo* SamplerApollo::s_sharedEnabledSamplerReference = 0;
SamplerApollo* SamplerApollo::s_samplerInstance = 0;

#if ENABLE(DEBUG_SAMPLER_NODES)
FILE* SamplerApollo::s_debugFile = 0;
#endif

#if ENABLE(JIT)
WTF::RefPtr<FakeSamplerNode> SamplerApollo::s_sharedFakeSamplerNode;
#endif

bool inline seenByProfiler(JSValue function)
{
    if (!function)
        return false;
    
    if (!function.isObject())
        return false;

    return true;
}

void SamplerApollo::enableSampler()
{
    m_constructorNode.clear();
    s_sharedEnabledSamplerReference = this;
	SamplerContextPool::registerPool(this);
    samplingNow = true;
}

void SamplerApollo::disableSampler()
{
    samplingNow = false;
    m_constructorNode.clear();
	SamplerContextPool::unregisterPool(this);
    s_sharedEnabledSamplerReference = 0;
    
    m_liveNodes.clear();
    m_liveNodesByIdentifier.clear();
    m_deadNodesByIdentifier.clear();
}

BaseSamplerNode* SamplerApollo::getSamplerNode(uint64_t identifier)
{
    // cache the result
    if (m_cachedSearchNode.get() && m_cachedSearchNode->identifier() == identifier)
        return m_cachedSearchNode.get();

	WTF::RefPtr<BaseSamplerNode> node = m_liveNodesByIdentifier.get(identifier);
	if (!node.get())
		node = m_deadNodesByIdentifier.get(identifier);
    
    m_cachedSearchNode = node.get();
	return node.get();
}

BaseSamplerNode* SamplerApollo::getSamplerNodeFor(void* ptr)
{
    ASSERT(ptr);
    return m_liveNodes.get(ptr).get();
}

SamplerApollo::SamplerApollo()
    : samplingNow(false)
{
    s_samplerInstance = this;
#if ENABLE(JIT)
	if (!s_sharedFakeSamplerNode.get())
		s_sharedFakeSamplerNode = adoptRef(new FakeSamplerNode());
#endif
#if ENABLE(DEBUG_SAMPLER_NODES)
    if (!s_debugFile)
        s_debugFile = fopen(APOLLO_SAMPLER_LOG_FILE, "a");

    pushLineToLog("SamplerApollo created");
#endif
}

SamplerApollo::~SamplerApollo()
{
    s_samplerInstance = 0;
	SamplerContextPool::unregisterPool(this);
#if ENABLE(DEBUG_SAMPLER_NODES)
    if (s_debugFile) {
        pushLineToLog("SamplerApollo destroyed");
        fclose(s_debugFile);
    }
#endif
}

#if ENABLE(DEBUG_SAMPLER_NODES)
void SamplerApollo::pushLineToLog(const char* value)
{
    if (!s_debugFile)
        return;

    fputs(value, s_debugFile);
    fputc('\n', s_debugFile);
}
#endif
        
BaseSamplerNode* SamplerApollo::willExecute(ExecState* exec, JSValue function)
{
    int32_t lineNo = 0;
    return willExecute(exec, function, lineNo);
}

BaseSamplerNode* SamplerApollo::willConstruct(ExecState* exec, JSValue function)
{
    int32_t lineNo = 0;
    return willConstruct(exec, function, lineNo);
}

BaseSamplerNode* SamplerApollo::willExecute(ExecState* exec, JSValue function, int32_t& lineNo)
{
    ASSERT(lineNo == 0);
    
    if (!seenByProfiler(function))
        return 0;

    JSObject* object = asObject(function);
    
    if (object->inherits(&JSFunction::info)) {
        JSFunction* jsFunction = asFunction(function);
        // we should take lineno only for non host functions
        // otherwise we end up with wrong line numbers for functions like
        // DOM functions, which should have 0 as line number
        // see bug 2587164
        if (!jsFunction->isHostFunction())
			lineNo = jsFunction->jsExecutable()->lineNo();
    }
    
    BaseSamplerNode* samplerNode = getSamplerNodeFor(static_cast<JSCell*>(object));
    if (!samplerNode) {
        // try to inject the object now
        // as it might have been created while sampling was disabled
        // this can happen when the sample buffer runs out of space
        
		ASSERT(exec->dynamicGlobalObject());

        uint64_t globalObjectIdentifier = 0;
        
        JSGlobalObject* globalObject = exec->dynamicGlobalObject();
        
        BaseSamplerNode* globalObjectSample = getSamplerNodeFor(globalObject);
        if (globalObjectSample)
            globalObjectIdentifier = globalObjectSample->identifier();
        
        PassRefPtr<SamplerJSCellNode> samplerNodeRef = adoptRef(new SamplerJSCellNode(globalObjectIdentifier, static_cast<JSCell*>(object), sizeof(JSObject)));
        samplerNode = samplerNodeRef.get();
		
		// the sampler might be stopped by the time we get here
		// and register node will not give an id
		// in this cases we just fail here
        if (!registerNode(samplerNodeRef))
			return 0;
    }
    samplerDidEnterFunction(samplerNode->identifier(), lineNo);

    // do not increment invocationCount if sampling is paused
    // however, we called samplerDidEnterFunction because we need to keep the callstack in sync
    if (UNLIKELY(!samplingNow))
        return samplerNode;

    samplerNode->willExecute();
	
	return samplerNode;
}

void SamplerApollo::willExecuteCached(ExecState*, BaseSamplerNode* functionNode, int32_t lineNo)
{
	ASSERT(functionNode);
	ASSERT(functionNode->identifier());
	
	// sending lineno 0 as the first line of code should update the line
	samplerDidEnterFunction(functionNode->identifier(), lineNo);

	// do not increment invocationCount if sampling is paused
    // however, we called samplerDidEnterFunction because we need to keep the callstack in sync
    if (UNLIKELY(!samplingNow))
        return;
		
	functionNode->willExecute();
}

// This method is still needed because the throwException uses the function register instead 
// of the already found identifier
void SamplerApollo::didExecute(ExecState*, JSValue function)
{
    if (!seenByProfiler(function))
        return;
    
    JSObject* object = asObject(function);
	BaseSamplerNode* samplerNode = getSamplerNodeFor(static_cast<JSCell*>(object));
	if (!samplerNode)
		return;
    
	samplerDidExitFunction(samplerNode->identifier());
}

void SamplerApollo::didExecuteCached(ExecState*, BaseSamplerNode* functionNode)
{
	samplerDidExitFunction(functionNode->identifier());
}

BaseSamplerNode* SamplerApollo::willConstruct(ExecState* exec, JSValue function, int32_t& lineNo)
{
    ASSERT(!m_constructorNode.get());
    ASSERT(lineNo == 0);
    
	BaseSamplerNode* samplerNode = willExecute(exec, function, lineNo);
	if (!samplerNode)
		return 0;
	
	m_constructorNode = static_cast<SamplerJSCellNode*>(samplerNode);
	
	return samplerNode;
}

void SamplerApollo::willConstructCached(ExecState* exec, BaseSamplerNode* functionNode, int32_t lineNo)
{
	ASSERT(functionNode);
	m_constructorNode = static_cast<SamplerJSCellNode*>(functionNode);
	
	willExecuteCached(exec, functionNode, lineNo);
}

void SamplerApollo::willExecute(ExecState*, ScriptExecutable* scriptExecutable)
{
#if ENABLE(CONTEXT_SAMPLING)
    BaseSamplerNode* scriptExecutableNode = static_cast<BaseSamplerNode*>(getSamplerNodeFor(scriptExecutable));
    if (!scriptExecutableNode)
        return;
    
    samplerDidEnterFunction(scriptExecutableNode->identifier(), scriptExecutable->lineNo());

    if (UNLIKELY(!samplingNow))
        return;

    scriptExecutableNode->willExecute();

#else
    (void)scriptExecutable;
#endif
}

void SamplerApollo::didExecute(ExecState*, ScriptExecutable* scriptExecutable)
{
#if ENABLE(CONTEXT_SAMPLING)
    BaseSamplerNode* samplerNode = getSamplerNodeFor(scriptExecutable);
    if (!samplerNode)
        return;
    
    samplerDidExitFunction(samplerNode->identifier());
#else
    (void)scriptExecutable;
#endif    
}

void SamplerApollo::willExecute(SamplerContext* samplerContext)
{
    BaseSamplerNode* samplerNode = getSamplerNodeFor(samplerContext);
    if (!samplerNode)
        return;
    
    samplerDidEnterFunction(samplerNode->identifier(), 0);

    // do not increment invocationCount if sampling is paused
    // however, we called samplerDidEnterFunction because we need to keep the callstack in sync
    if (UNLIKELY(!samplingNow))
        return;

    samplerNode->willExecute();
}

void SamplerApollo::didExecute(SamplerContext* samplerContext)
{
    BaseSamplerNode* samplerNode = getSamplerNodeFor(samplerContext);
    if (!samplerNode)
        return;
    
    samplerDidExitFunction(samplerNode->identifier());
}

void SamplerApollo::registerJSCell(JSGlobalObject* globalObject, JSCell* cell, size_t size)
{
    WTF::PassRefPtr<SamplerJSCellNode> constructorNode = m_constructorNode.release();
    
    if (UNLIKELY(!samplingNow))
        return;
    
    uint64_t globalObjectIdentifier = 0;
        
    if (globalObject) {
        BaseSamplerNode* globalObjectSample = getSamplerNodeFor(globalObject);
        if (globalObjectSample)
            globalObjectIdentifier = globalObjectSample->identifier();
    }
    
    PassRefPtr<SamplerJSCellNode> samplerNodeRef = adoptRef(new SamplerJSCellNode(globalObjectIdentifier, cell, size));
    SamplerJSCellNode *samplerNode = samplerNodeRef.get(); 
    
	if (registerNode(samplerNodeRef)) {
        // cannot check the type because we are called directly from operator new
        if (constructorNode.get() /* && cell->isObject() */) {
			
			JSCell * const cell = constructorNode->impl(); 
			// we don't want to make standard objects to show like Functions
			// Internal construct for String,Boolean,... inherits directly InternalFunctions
			if (! (cell && cell->getObject() && cell->getObject()->inherits(&InternalFunction::info) && !cell->getObject()->inherits(&JSFunction::info)) ) {
                // try to set the name from the function
                samplerNode->setObjectType(constructorNode->name());
			}
        }
    }
}

bool SamplerApollo::registerNode(WTF::PassRefPtr<BaseSamplerNode> node)
{
    if (UNLIKELY(!samplingNow))
        return false; 

    // check if the object was allocated earlier or if maybe the old object
    // didn't trigger deallocation
    ASSERT(getSamplerNodeFor(node->ptr()) == 0);
    const uint64_t identifier = samplerDidAllocate(node.get());
    if (identifier) {
        node->setIdentifier(identifier);
        m_liveNodes.add(node->ptr(), node.get());
        m_liveNodesByIdentifier.add(node->identifier(), node.get());
        
        // the references from liveNodes maps + pass ref ptr
        ASSERT(node->refCount() == 3);
        return true;
    }
    return false;
}

void SamplerApollo::unregisterNode(WTF::PassRefPtr<BaseSamplerNode> node)
{
    ASSERT(node.get());
    
    // invalidate the cached node if it is going to be deleted    
    if (m_cachedSearchNode.get() == node.get())
        m_cachedSearchNode.clear();
    
    m_liveNodes.remove(node->ptr());
    m_liveNodesByIdentifier.remove(node->identifier());
    
#if !ENABLE(JIT)
    // the references from RefPtr
    ASSERT(node->hasOneRef());
#endif
    
    // when sampling is stopped don't bother to keep the dead object.
    if (samplerWillDeallocate(node.get())) {
        WTF::PassRefPtr<DeadSamplerNode> deadNode = DeadSamplerNode::createWithDataFrom(node.get());
        ASSERT(deadNode->hasOneRef());			
        m_deadNodesByIdentifier.add(node->identifier(), deadNode.get());
    }

#if ENABLE(JIT)
	node->clearPtr();
#endif
}

void SamplerApollo::unregisterNodeHelper(void* ptr)
{
    ASSERT(ptr);
    WTF::RefPtr<BaseSamplerNode> node(getSamplerNodeFor(ptr));
    if(node.get())
        unregisterNode(node.release());
}

void SamplerApollo::clearDeadObjects()
{
#if ENABLE(DEBUG_SAMPLER_NODES)
    pushLineToLog("clearDeadObjects");
#endif
    m_cachedSearchNode.clear();
#ifdef _DEBUG
	SamplerNodesMap::const_iterator end = m_deadNodesByIdentifier.end();
	for (SamplerNodesMap::const_iterator it = m_deadNodesByIdentifier.begin(); it != end; ++it) {
		ASSERT(it->second->hasOneRef());
	}
#endif
    m_deadNodesByIdentifier.clear();
}
    


} // namespace JSC

#endif // PLATFORM(APOLLO)
