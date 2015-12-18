/*
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

#include <config.h>
#include <wtf/Platform.h>

#if ENABLE(APOLLO_PROFILER)

#if PLATFORM(APOLLO_UNIX)
#include "../../WebKit.apolloproj/WebKitPrefix.h"
#endif
#include "WebSamplerImpl.h"
#include "runtime/JSLock.h"
#include "parser/Parser.h"
#include "runtime/CollectorHeapIterator.h"
#include "JSDOMWindow.h"
#include "sampler/ProfilerMembers.h"
#include <FrameLoaderClientApollo.h>

#if ENABLE(DEBUG_SAMPLER_NODES)
#define DEBUG_SAMPLER_API_BEGIN(name, identifier) \
    WebCore::String __logLine; \
    __logLine.append(name); \
    __logLine.append("("); \
    __logLine.append(WebCore::String::number(identifier)); \
    __logLine.append(") = ");
#define DEBUG_SAMPLER_API_RESULT(result) \
    __logLine.append(result); \
    pushLineToLog(__logLine.utf8().data());
#define DEBUG_SAMPLER_LOG(data) \
    pushLineToLog(data);
#else
#define DEBUG_SAMPLER_API_BEGIN(name, identifier)
#define DEBUG_SAMPLER_API_RESULT(result)
#define DEBUG_SAMPLER_LOG(data)
#endif

namespace WebKitApollo {
    extern WebKitAPIHostFunctions* g_HostFunctions;
    
    static WebSampler* s_webSamplerRef = 0;
    
    WebSamplerImpl::WebSamplerImpl()
        : m_recompileTimer(this, &WebSamplerImpl::recompileAllJSFunctions)
    {
        ASSERT(WebKitApollo::g_HostFunctions->getWebSamplerListener);
        m_webSamplerListener = WebKitApollo::g_HostFunctions->getWebSamplerListener();
        if (m_webSamplerListener) 
        {
            s_webSamplerRef = getWebSampler();
            m_webSamplerListener->m_pVTable->setWebSamplerRef(m_webSamplerListener, &s_webSamplerRef);
        }
    }
    
    WebSamplerImpl::~WebSamplerImpl()
    {
        if(s_webSamplerRef)
        {
            // when s_webSamplerRef is set to NULL we are sure the AIR part destroyed before us
            ASSERT(m_webSamplerListener);
            m_webSamplerListener->m_pVTable->setWebSamplerRef(m_webSamplerListener, 0);
            m_webSamplerListener = 0;
            s_webSamplerRef = 0;
        }
    }
  
    void WebSamplerImpl::samplerDidEnterFunction(uint64_t identifier, int32_t lineno)
    {
		if(!s_webSamplerRef)
            return;
        
        ASSERT(m_webSamplerListener);
		
        m_webSamplerListener->m_pVTable->didEnterFunction(m_webSamplerListener, identifier, lineno);
    }
    
    void WebSamplerImpl::atStatement(int line)
    {
        if(!s_webSamplerRef)
            return;
        
        ASSERT(m_webSamplerListener);
        
        m_webSamplerListener->m_pVTable->atStatement(m_webSamplerListener, line);
    }
    
    void WebSamplerImpl::samplerDidExitFunction(uint64_t identifier)
    {
        if(!s_webSamplerRef)
            return;
        ASSERT(m_webSamplerListener);
        
        m_webSamplerListener->m_pVTable->didExitFunction(m_webSamplerListener, identifier);        
    }
    
    uint64_t WebSamplerImpl::samplerDidAllocate(JSC::BaseSamplerNode* node)
    {
        if(!s_webSamplerRef)
            return 0;
        
        ASSERT(m_webSamplerListener);
        
        const bool inConstructor = true;
        return m_webSamplerListener->m_pVTable->didAllocateObject(m_webSamplerListener, node->ptr(), node->objectSize(inConstructor));
    }
    
    bool WebSamplerImpl::samplerWillDeallocate(JSC::BaseSamplerNode* node)
    {
        if(!s_webSamplerRef)
            return false;
        
        ASSERT(m_webSamplerListener);
        
        m_webSamplerListener->m_pVTable->didDeallocateObject(m_webSamplerListener, node->ptr(), node->objectSize());
		return true;
    }
    
    void WebSamplerImpl::startSampling()
    {
        DEBUG_SAMPLER_LOG("startSampling")
        if (!m_webSamplerListener)
            return;

        if(!enabled())
        {
            enableSampler();
            recompileAllJSFunctionsSoon();
        }
        samplingNow = true;
    }
    
    void WebSamplerImpl::stopSampling()
    {
        DEBUG_SAMPLER_LOG("stopSampling")
		if(enabled())
        {
            disableSampler();
            recompileAllJSFunctionsSoon();
        }
    }

    void WebSamplerImpl::pauseSampling()
    {
        DEBUG_SAMPLER_LOG("pauseSampling")
        if (!enabled() || !samplingNow)
            return;
        samplingNow = false;
    }

    WebString* WebSamplerImpl::getObjectType(uint64_t identifier)
    {
        DEBUG_SAMPLER_API_BEGIN("getObjectType", identifier)
        WTF::RefPtr<JSC::BaseSamplerNode> samplerNode(getSamplerNode(identifier));
        if (!samplerNode.get()) {
            DEBUG_SAMPLER_API_RESULT("null - node not found")
            return 0;
        }
        
        JSC::UString result = samplerNode->objectType();
        if (result.isEmpty()) {
            DEBUG_SAMPLER_API_RESULT("null - result is empty")
            return 0;
        }
        
		DEBUG_SAMPLER_API_RESULT((WebCore::ustringToString(result)))
        return WebCore::String(WebCore::ustringToString(result)).webString();
    }
    
    size_t WebSamplerImpl::getObjectSize(uint64_t identifier)
    {
        DEBUG_SAMPLER_API_BEGIN("getObjectSize", identifier)
        WTF::RefPtr<JSC::BaseSamplerNode> node(getSamplerNode(identifier));
        if (!node.get()) {
            DEBUG_SAMPLER_API_RESULT("null - node not found")
            return 0;
        }
        
        size_t objectSize = node->objectSize();
        DEBUG_SAMPLER_API_RESULT(WebCore::String::number(objectSize))
        return objectSize;
    }
	
	WebString* WebSamplerImpl::getObjectFilename(uint64_t identifier)
    {
        DEBUG_SAMPLER_API_BEGIN("getObjectFilename", identifier)
        WTF::RefPtr<JSC::BaseSamplerNode> samplerNode(getSamplerNode(identifier));
        if (!samplerNode.get()) {
            DEBUG_SAMPLER_API_RESULT("null - node not found")
            return 0;
        }
        
        JSC::UString jscSourceURL = samplerNode->filename();
        
        if (jscSourceURL.isEmpty()) {
            DEBUG_SAMPLER_API_RESULT("null - result is empty")
            return 0;
        }
        
        WebCore::String coreSourceURL = WebCore::String(WebCore::ustringToString(jscSourceURL));
        
        JSC::JSGlobalObject* globalObject = samplerNode->globalObject();
        if (globalObject)
        {
            WebCore::Frame *frame = static_cast<WebCore::JSDOMWindow*>(globalObject)->impl()->frame();
            if (!coreSourceURL.isNull()) {
                WebCore::KURL sourceURL(WebCore::ParsedURLString, coreSourceURL);
                WebCore::KURL mappedURL;

                if (WebCore::FrameLoaderClientApollo::mapFrameUrl(frame, sourceURL, &mappedURL))
                    coreSourceURL = mappedURL.string();
            }
        }

        DEBUG_SAMPLER_API_RESULT(coreSourceURL)

        return coreSourceURL.webString();
    }
	
	WebString* WebSamplerImpl::getObjectName(uint64_t identifier)
    {
        DEBUG_SAMPLER_API_BEGIN("getObjectName", identifier)
        WTF::RefPtr<JSC::BaseSamplerNode> samplerNode(getSamplerNode(identifier));
        if (!samplerNode.get()) {
            DEBUG_SAMPLER_API_RESULT("null - node not found")
            return 0;
        }

        JSC::UString result = samplerNode->name();
        if (result.isEmpty()) {
            DEBUG_SAMPLER_API_RESULT("null - result is empty")
            return 0;
        }

		DEBUG_SAMPLER_API_RESULT(WebCore::ustringToString(result))
        return WebCore::String(WebCore::ustringToString(result)).webString();
    }
    
    uint64_t WebSamplerImpl::getObjectCallCount(uint64_t identifier)
    {
        DEBUG_SAMPLER_API_BEGIN("getObjectCallCount", identifier)
        WTF::RefPtr<JSC::BaseSamplerNode> node(getSamplerNode(identifier));
        if (!node.get()) {
            DEBUG_SAMPLER_API_RESULT("null - node not found")
            return 0;
        }
        
        uint64_t invocationCount = node->invocationCount();
        DEBUG_SAMPLER_API_RESULT(WebCore::String::number(invocationCount));
        return invocationCount;
    }
    
    bool WebSamplerImpl::getObjectMembers(uint64_t identifier, WebSamplerMembersList* webMembersList)
    {
        DEBUG_SAMPLER_API_BEGIN("getObjectMembers", identifier)

        JSC::JSLock lock(false);
        WTF::RefPtr<JSC::BaseSamplerNode> node(getSamplerNode(identifier));
        if (!node.get()) {
            DEBUG_SAMPLER_API_RESULT("null - node not found")
            return false;
        }
            
        JSC::ProfilerMembersList profilerMembersList(this);
        node->getMembers(&profilerMembersList);
        
        if (profilerMembersList.isEmpty()) {
            DEBUG_SAMPLER_API_RESULT("null - result is empty")
            return false;
        }
        
		webMembersList->m_pVTable->init(webMembersList, profilerMembersList.size());
		
#if ENABLE(DEBUG_SAMPLER_NODES)
        WebCore::String membersList;
#endif

        JSC::ProfilerMembersList::const_iterator iter = profilerMembersList.begin();
        JSC::ProfilerMembersList::const_iterator end = profilerMembersList.end();
        
        for( ; iter != end; iter++ ) {
            JSC::ProfilerMembersItem member = *iter;
#if ENABLE(DEBUG_SAMPLER_NODES)
            membersList.append(WebCore::ustringToString(member.name));
            membersList.append(" (");
            membersList.append(WebCore::String::number(member.identifier));
            membersList.append(") ");
#endif
            webMembersList->m_pVTable->append(webMembersList, WebCore::String(WebCore::ustringToString(member.name)).webString(), member.identifier);
        }
        
        DEBUG_SAMPLER_API_RESULT(membersList)
		return true;
    }
    
    WebSamplerImpl* WebSamplerImpl::getInstance()
    {
        static WTF::RefPtr<WebSamplerImpl> profiler;

        if(!profiler.get())
            profiler = new WebSamplerImpl();
            
        return profiler.get();
    }

    void WebSamplerImpl::recompileAllJSFunctionsSoon()
    {
        DEBUG_SAMPLER_LOG("recompileAllJSFunctionsSoon")
        m_recompileTimer.startOneShot(0);
    }

    void WebSamplerImpl::recompileAllJSFunctions(WebCore::Timer<WebSamplerImpl>*)
    {
        DEBUG_SAMPLER_LOG("recompileAllJSFunctions")
        JSC::JSLock lock(false);
        JSC::JSGlobalData* globalData = WebCore::JSDOMWindow::commonJSGlobalData();

        // If JavaScript is running, it's not safe to recompile, since we'll end
        // up throwing away code that is live on the stack.
        ASSERT(!globalData->dynamicGlobalObject);
        if (globalData->dynamicGlobalObject)
            return;

        WTF::Vector<JSC::ProtectedPtr<JSC::JSFunction> > functions;
        JSC::LiveObjectIterator heapEnd = globalData->heap.primaryHeapEnd();
        for (JSC::LiveObjectIterator it = globalData->heap.primaryHeapBegin(); it != heapEnd; ++it) {
            if ((*it)->inherits(&JSC::JSFunction::info)) {
                JSC::JSFunction* function = static_cast<JSC::JSFunction*>(*it);
                if (!function->isHostFunction())
                    functions.append(function);
            }
        }

        typedef WTF::HashMap<WTF::RefPtr<JSC::FunctionBodyNode>, WTF::RefPtr<JSC::FunctionBodyNode> > FunctionBodyMap;

        FunctionBodyMap functionBodies;

        size_t size = functions.size();
        for (size_t i = 0; i < size; ++i) {
            JSC::JSFunction* function = functions[i];

            // remove after fixing below
            (void) function;
            /*
            apollo integrate -> TODO
            JSC::FunctionBodyNode* oldBody = function->body();
            pair<FunctionBodyMap::iterator, bool> result = functionBodies.add(oldBody, 0);
            if (!result.second) {
                function->setBody(result.first->second.get());
                continue;
            }

            JSC::ExecState* exec = function->scope().globalObject()->JSGlobalObject::globalExec();
            const JSC::SourceCode& sourceCode = oldBody->source();

            WTF::RefPtr<JSC::FunctionBodyNode> newBody = globalData->parser->parse<JSC::FunctionBodyNode>(exec, 0, sourceCode);
            ASSERT(newBody);
            newBody->finishParsing(oldBody->copyParameters(), oldBody->parameterCount());
            // propagate old contextual name
            newBody->setContextualName(oldBody->contextualName());

            result.first->second = newBody;
            function->setBody(newBody.release());
            */
        }
    }
}

#endif

