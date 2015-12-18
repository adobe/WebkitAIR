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
#include "Node.h"

#if PLATFORM(APOLLO)

#if ENABLE(DOM_SAMPLING)
#include "sampler/ProfilerMembers.h"
#include "sampler/SamplerApollo.h"
#include "sampler/SamplerNodes.h"
#include "EventTargetNode.h"
#include "EventListener.h"
#include "JSEventListener.h"

namespace JSC {
    
    template <> 
    class SamplerDataNode<WebCore::Node>: public BaseSamplerNode
	{
	public:
        SamplerDataNode<WebCore::Node>(WebCore::Node* ptr, size_t size)
            : BaseSamplerNode(ptr)
            , m_size(size)
        {
        }
        
        inline WebCore::Node* impl() const { return static_cast<WebCore::Node*>(ptr()); }
        
        virtual size_t objectSize(bool /*inConstructor*/) const { return m_size; }
        
        virtual UString objectType() const 
        {
            return WebCore::String("[DOM]") + impl()->nodeName();
        }
        
        virtual void getMembers(ProfilerMembersList* members) const;
        
    private:
        size_t m_size;
	};

void SamplerDataNode<WebCore::Node>::getMembers(ProfilerMembersList* members) const
{
    impl()->getMembersForProfiler(members);
    
    WebCore::Node* child = impl()->firstChild();
    int childNumber = 0;
    while (child)
    {
        UString memberName;
        
        if (WebCore::NamedAttrMap* childAttrs = child->attributes()) {
            WebCore::Attribute* idItem = childAttrs->getAttributeItem(WebCore::HTMLNames::idAttr);
            if (idItem && !idItem->isNull())
                memberName = idItem->value();
        }
        
        if (memberName.isEmpty())
            memberName = UString::from(childNumber);
        
        members->append(memberName, child);
        
        childNumber++;                
        child = child->nextSibling();
    }
    
    // check if we have any listeners 
    if (impl()->isEventTargetNode()) {
        const WebCore::RegisteredEventListenerVector& listeners = WebCore::EventTargetNodeCast(impl())->eventListeners();

        if (!listeners.isEmpty()) {
        
            size_t size = listeners.size();
            
            for (size_t i = 0; i < size; ++i) {
                WebCore::RegisteredEventListener* registeredListener = listeners.at(i).get();
                if (registeredListener->removed())
                        continue;
                
                WebCore::EventListener* listener = registeredListener->listener();
                
                if (listener->isJSAbstractEventListener()) {
                    WebCore::JSAbstractEventListener* jsAbstractEventListener = static_cast<WebCore::JSAbstractEventListener*>(listener);
                    
                    members->append(registeredListener->eventType(), jsAbstractEventListener->listenerObj());
                }
            }
            
        }
        
    }
    
}

} // namespace JSC
#endif //ENABLE(DOM_SAMPLING)

namespace WebCore {

#if ENABLE(DOM_SAMPLING)
void* Node::operator new(size_t size)
{
    void* buffer = new char[size];

    if (JSC::SamplerApollo* sampler = *JSC::SamplerApollo::enabledSamplerReference())
        sampler->registerNode(adoptRef(new JSC::SamplerDataNode<Node>(static_cast<Node*>(buffer), size)));

    return buffer;
}

void Node::operator delete(void* ptr)
{
    delete [] (char*)ptr;
}

void Node::destroy()
{  
#ifndef NDEBUG
    m_deletedFromDestroy = true;
#endif
    
    if (JSC::SamplerApollo* sampler = *JSC::SamplerApollo::enabledSamplerReference())
        sampler->unregisterNodeHelper(this);
        
    delete this;
}

void Node::getMembersForProfiler(JSC::ProfilerMembersList*)
{
}
#endif //ENABLE(DOM_SAMPLING)

}

#endif //PLATFORM(APOLLO)

