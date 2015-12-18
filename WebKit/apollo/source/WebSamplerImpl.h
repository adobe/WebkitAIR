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
   
#ifndef WebSamplerImpl_h
#define WebSamplerImpl_h

#if ENABLE(APOLLO_PROFILER)

#include <WebKitApollo/helpers/WebSamplerHelper.h>
#include "wtf/RefCounted.h"
#include "wtf/ListHashSet.h"
#include "sampler/SamplerApollo.h"
#include "Timer.h"


namespace WebKitApollo {

    class WebViewImpl;
        
    class WebSamplerImpl: public WTF::RefCounted<WebSamplerImpl>
                         , public WebSamplerHelper<WebSamplerImpl>
                         , public JSC::SamplerApollo
    {
    public:
        WebSamplerImpl();
        ~WebSamplerImpl();
        
        
        virtual void samplerDidEnterFunction(uint64_t identifier, int32_t lineno);
        virtual void samplerDidExitFunction(uint64_t identifier);

        virtual void atStatement(int line);

        virtual uint64_t samplerDidAllocate(JSC::BaseSamplerNode* node);
        virtual bool samplerWillDeallocate(JSC::BaseSamplerNode* node);
        
        // begin WebProfiler methods
        void startSampling();
        void stopSampling();
		void pauseSampling();

        WebString* getObjectType(uint64_t identifier);
        size_t getObjectSize(uint64_t identifier);
		bool getObjectMembers(uint64_t identifier, WebSamplerMembersList* webMembersList);
		
		uint64_t getObjectCallCount(uint64_t identifier);
		WebString* getObjectFilename(uint64_t identifier);
		WebString* getObjectName(uint64_t identifier);
        // end WebProfiler methods
        
        static WebSamplerImpl* getInstance();
        
        JSC::SamplerApollo* getSampler() { return this; }
        
    private:
        void recompileAllJSFunctionsSoon();
        void recompileAllJSFunctions(WebCore::Timer<WebSamplerImpl>*);
        uint64_t getObjectIdentifier(JSC::JSValue* obj);
        
        WebCore::Timer<WebSamplerImpl> m_recompileTimer;
    
        WebSamplerListener* m_webSamplerListener;
    };

}

#endif

#endif // WebSamplerImpl_h
