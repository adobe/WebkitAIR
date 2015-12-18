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

#include "SamplerNodes.h"
#include "Executable.h"
#include "JSFunction.h"
#include "Nodes.h"
#include "ProfilerMembers.h"
#include "SamplerContext.h"
#include "SamplerApollo.h"

namespace JSC {

static UString fromUint64(uint64_t l)
{
    UChar buf[1 + sizeof(l) * 3];
    UChar* end = buf + sizeof(buf) / sizeof(UChar);
    UChar* p = end;
        
    if (l == 0)
        *--p = '0';
    else {
        while (l) {
            *--p = static_cast<unsigned short>((l % 10) + '0');
            l /= 10;
        }
    }
        
    return UString(p, static_cast<int>(end - p));
    }
    
// ------------------------------ BaseSamplerNode -----------------------------

BaseSamplerNode::BaseSamplerNode(void* ptr)
    : m_identifier(0)
    , m_ptr(ptr)
    , m_invocationCount(0)

{
}

BaseSamplerNode::BaseSamplerNode(uint64_t identifier)
    : m_identifier(identifier)
    , m_ptr(0)
    , m_invocationCount(0)
{
}
    
BaseSamplerNode::BaseSamplerNode(uint64_t identifier, uint64_t invocationCount)
    : m_identifier(identifier)
    , m_ptr(0)
    , m_invocationCount(invocationCount)
    
{
 }

BaseSamplerNode::~BaseSamplerNode()
{
}

size_t BaseSamplerNode::objectSize(bool /*inConstructor*/) const
{
    return 0; 
}

void BaseSamplerNode::getMembers(ProfilerMembersList*) const
{
}

UString BaseSamplerNode::name()
{
	return UString();
}

UString BaseSamplerNode::filename() const 
{
    return UString(); 
}

JSGlobalObject* BaseSamplerNode::globalObject() const
{
    return 0;
}

#if ENABLE(JIT)
// ------------------------------ FakeSamplerNode -----------------------------

FakeSamplerNode::FakeSamplerNode()
	: BaseSamplerNode(reinterpret_cast<void*>(-1))
{
}

UString FakeSamplerNode::objectType()
{
	return "FakeSamplerNode";
}
#endif

// ------------------------------ SamplerDataNode<SamplerContext> -----------------------------

SamplerDataNode<SamplerContext>::SamplerDataNode(SamplerContext* ptr)
    : BaseSamplerNode(ptr)
{
}

UString SamplerDataNode<SamplerContext>::objectType()
{
    return UString("[SamplerContext]");
}

UString SamplerDataNode<SamplerContext>::name()
{
    return UString(impl()->name());
}

size_t SamplerDataNode<SamplerContext>::objectSize(bool /*inConstructor*/) const 
{
    return sizeof(SamplerContext);
}

// ------------------------------ SamplerScopeNode -----------------------------
SamplerScopeNode::SamplerScopeNode(ScopeNode* ptr)
    : BaseSamplerNode(ptr), m_name(BaseSamplerNode::name())
{
}

UString SamplerScopeNode::objectType()
{
    return UString("[JS Code Block]");
}

UString SamplerScopeNode::filename() const
{
    if (!impl()->source().provider()) {
#if ENABLE(DEBUG_SAMPLER_NODES)
        SamplerApollo::pushLineToLog("SamplerScopeNode::filename() has no source provider");
#endif
        return UString();
    }

    UString sourceURL = impl()->sourceURL();

    if (sourceURL.isEmpty())
        sourceURL = makeString("EvalScript", UString::from((long)impl()->sourceID()), ".js");

    return sourceURL;
}

size_t SamplerScopeNode::objectSize(bool /*inConstructor*/) const 
{
    if (!impl()->data())
        return sizeof(ScopeNode);
    
	// apollo - compile - integrate - 58803
	// children function has been removed and the vector size is no
	// longer accessible;
	// However, this size computation doesn't look correct either way
    // return sizeof(ScopeNode) + impl()->children().capacity() * (sizeof(StatementNode) + sizeof(RefPtr<StatementNode>));
	return sizeof(ScopeNode) + sizeof(ScopeNodeData);
}

// ------------------------------ SamplerJSCellNode -----------------------------

SamplerJSCellNode::SamplerJSCellNode(uint64_t globalObjectIdentifier, JSCell* ptr, size_t size)
    : BaseSamplerNode(ptr)
    , m_globalObjectIdentifier(globalObjectIdentifier)
    , m_size(size)
{
}

UString SamplerJSCellNode::objectType()
{
    return m_objectType.isEmpty() ? impl()->typeName() : m_objectType;
}

size_t SamplerJSCellNode::objectSize(bool inConstructor) const
{
    if (inConstructor)
        return m_size;

	return impl()->instanceSize() + m_size;
}

void SamplerJSCellNode::getMembers(ProfilerMembersList* members) const
{
    JSCell * const cell = impl(); 
    if (!cell->structure() || !cell->isObject())
        return;
    
    JSObject* object = cell->getObject();
    ExecState* exec = execState();
    if (!exec)
        return;
    
    //the dynamic global object should be set to the global object of the runtime object context
    DynamicGlobalObjectScope globalObjectScope(exec, exec->dynamicGlobalObject());
    members->setExec(exec);
    object->getMembersForProfiler(members);
}

UString SamplerJSCellNode::filename() const
{
	JSCell * const cell = impl(); 
    if (!cell->structure() || !cell->isObject()) {
#if ENABLE(DEBUG_SAMPLER_NODES)
        SamplerApollo::pushLineToLog("SamplerJSCellNode::filename() the cell has no structure or is not an object");
#endif
        return UString();
    }
    
    JSObject* object = cell->getObject();
    if (!object->inherits(&JSFunction::info)) {
#if ENABLE(DEBUG_SAMPLER_NODES)
        SamplerApollo::pushLineToLog("SamplerJSCellNode::filename() the object doesn't inherit from JSFunction::info");
#endif
		return UString();
    }
		
	JSFunction* function = static_cast<JSFunction*>(object);

	if (function->isHostFunction())
	{
#if ENABLE(DEBUG_SAMPLER_NODES)
        SamplerApollo::pushLineToLog("SamplerJSCellNode::filename() the object is a Host Function");
#endif
		//Host Functions do not store a range of info that the FunctionExecutable provides
		return UString();
	}

	FunctionExecutable* executable = function->jsExecutable();
	ASSERT(executable);
    
    if (!executable->source().provider()) {
#if ENABLE(DEBUG_SAMPLER_NODES)
        SamplerApollo::pushLineToLog("SamplerJSCellNode::filename() the body has no source provider");
#endif
        return UString();
    }

    UString sourceURL = executable->sourceURL();

    if (sourceURL.isEmpty())
		sourceURL = makeString("EvalScript", UString::from((long)executable->sourceID()), ".js");

    return sourceURL;
}

UString SamplerJSCellNode::name()
{
	JSCell * const cell = impl(); 
    if (!cell->structure() || !cell->isObject())
        return UString("<Not a function>");
    
    JSObject* object = cell->getObject();
	if (!object->inherits(&JSFunction::info))
	{
		// take into account internal functions, like Math.random()
		if (!object->inherits(&InternalFunction::info)) {
			UString typeName = cell->typeName();
			return typeName;
		}
		else {
			InternalFunction *internal = static_cast<InternalFunction*>(object);
			
			ExecState* execState = SamplerJSCellNode::execState();
			// if no execState, create a new internal function identifier
			if (!execState) {
				
				UString internalName = makeString("<internal>", fromUint64(identifier()));
				return internalName;
			}
			
			UString name = internal->calculatedDisplayName(execState);
			if (name.isEmpty()) {
				name = makeString("<internal>", fromUint64(identifier()));
			}
			
			return name;
		}
	}
		
	JSFunction* function = static_cast<JSFunction*>(object);
	if (function->isHostFunction())
	{
#if ENABLE(DEBUG_SAMPLER_NODES)
        SamplerApollo::pushLineToLog("SamplerJSCellNode::filename() the object is a Host Function");
#endif
		//Host Functions do not store a range of info that the FunctionExecutable provides
		return UString();
	}
	
	ExecState* exec = SamplerJSCellNode::execState();
	if (!exec)
	{
        UString functionName;

#if ENABLE(APOLLO_PROFILER)
        // try to take the name from the function executable
        ASSERT(function->jsExecutable());
        if (function->jsExecutable())
		{
            functionName = function->jsExecutable()->name().ustring();
			if (functionName.isEmpty())
				functionName = function->jsExecutable()->contextualName();
		}
#endif
		if (functionName.isEmpty())
			functionName = makeString("<anonymous>", fromUint64(identifier()));
		
		return functionName;
	}
	
	UString name = function->calculatedDisplayName(exec);

	if (name.isEmpty()) {
#if ENABLE(APOLLO_PROFILER)
		// try to take the name from the function executable
        ASSERT(function->jsExecutable());
        if (function->jsExecutable())
		{
            name = function->jsExecutable()->name().ustring();
			if (name.isEmpty())
				name = function->jsExecutable()->contextualName();
		}
#endif
        if (name.isEmpty())
			name = makeString("<anonymous>", fromUint64(identifier()));
	}
	
	return name;
}

ExecState* SamplerJSCellNode::execState() const
{
    JSGlobalObject* globalObject = SamplerJSCellNode::globalObject();
    if (!globalObject)
        return 0;
    
    return globalObject->globalExec();
}
    
JSGlobalObject* SamplerJSCellNode::globalObject() const
    {
    JSGlobalObject* globalObject;
    
    if (!m_globalObjectIdentifier) {
    
        // the global object might be exactly this object
        JSCell* const cell = impl();
        if (!cell->structure() || !cell->isObject())
            return 0;
        
        JSObject* object = cell->getObject();
        if (!object->isGlobalObject())
            return 0;
        
        globalObject = static_cast<JSGlobalObject*>(object);

    } else {

        SamplerApollo* sampler = SamplerApollo::samplerInstance();
        if (!sampler)
            return 0;
        
        BaseSamplerNode* baseSamplerNode = sampler->getSamplerNode(m_globalObjectIdentifier);
        if (!baseSamplerNode)
            return 0;
        
        if (!baseSamplerNode->ptr())
            return 0;
        
        globalObject = static_cast<JSGlobalObject*>(baseSamplerNode->ptr());
    
    }

    ASSERT(globalObject);
    return globalObject;
}

DeadSamplerNode::DeadSamplerNode(uint64_t identifier, UString nodeName, UString objectType, size_t objectSize, UString filename, uint64_t invocationCount)
    : BaseSamplerNode(identifier, invocationCount)
    , m_objectType(objectType)
    , m_objectSize(objectSize)
    , m_filename(filename)
    , m_name(nodeName)
{
}

WTF::PassRefPtr<DeadSamplerNode> DeadSamplerNode::createWithDataFrom(BaseSamplerNode* source)
{
#if ENABLE(DEBUG_SAMPLER_NODES)
    UString logLine("DeadSamplerNode::createWithDataFrom for id ");
	logLine = makeString(logLine, fromUint64(source->identifier()));
    SamplerApollo::pushLineToLog(logLine.UTF8String().data());
#endif
    return adoptRef(new DeadSamplerNode(
        source->identifier(),
        source->name(),
        source->objectType(),
        source->objectSize(),
        source->filename(),
        source->invocationCount()));
}

UString DeadSamplerNode::name() 
{ 
	return m_name;
}

}

#endif // PLATFORM(APOLLO)
