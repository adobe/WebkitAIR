/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef CachedResourceHandle_h
#define CachedResourceHandle_h

#include "CachedResource.h"

namespace WebCore {

    class CachedResourceHandleBase {
    public:
        ~CachedResourceHandleBase() { 
            if (m_resource) 
                m_resource->unregisterHandle(this); 
#if ENABLE(APOLLO_CLEAR_CACHE)
            m_resource = NULL;
#endif
        }
        CachedResource* get() const { return m_resource; }
        
        bool operator!() const { return !m_resource; }
        
        // This conversion operator allows implicit conversion to bool but not to other integer types.
        // Parenthesis is needed for winscw compiler to resolve class qualifier in this case.
        typedef CachedResource* (CachedResourceHandleBase::*UnspecifiedBoolType);
        operator UnspecifiedBoolType() const { return m_resource ? &CachedResourceHandleBase::m_resource : 0; }

    protected:
#if ENABLE(APOLLO_CLEAR_CACHE)
        CachedResourceHandleBase(bool isWeakReference) 
			: m_resource(0)
			, m_isWeakReference(isWeakReference)
		{
		}
#else
        CachedResourceHandleBase() : m_resource(0) {}
#endif
        
#if ENABLE(APOLLO_CLEAR_CACHE)
		CachedResourceHandleBase(CachedResource* res, bool isWeakReference) 
			: m_isWeakReference(isWeakReference) 
#else   
        CachedResourceHandleBase(CachedResource* res)
#endif
		{ 
			m_resource = res; 
			if (m_resource) 
				m_resource->registerHandle(this); 
		}
        
		CachedResourceHandleBase(const CachedResourceHandleBase& o)
			: m_resource(o.m_resource)
#if ENABLE(APOLLO_CLEAR_CACHE)
			, m_isWeakReference(o.m_isWeakReference) 
#endif
		{ 
			if (m_resource) 
				m_resource->registerHandle(this); 
		}

        void setResource(CachedResource*);
		
#if ENABLE(APOLLO_CLEAR_CACHE)
		bool isWeakReference() const { return m_isWeakReference; }
#endif
	
	protected:
#if ENABLE(APOLLO_CLEAR_CACHE)
		void setIsWeakReference(bool weakReference) { m_isWeakReference = weakReference; }
#endif
	
    private:
        CachedResourceHandleBase& operator=(const CachedResourceHandleBase&) { return *this; } 
        
        friend class CachedResource;
		
		CachedResource* m_resource;
		
#if ENABLE(APOLLO_CLEAR_CACHE)
		bool m_isWeakReference;
#endif
    };
        
#if ENABLE(APOLLO_CLEAR_CACHE)
	// While investigating the bug [#2647006: Removing elements from DOM does not release memory],
	// we have seen that even after resources were evicted from cache, their dtor was not called.
	// The document loader was keeping a last reference to them. In order to release the resources,
	// we instructed the document loader to keep a weak reference to these resources and such,
	// when the last client of the resource goes away, the resource can be freed since the reference
	// from document loader is no longer taken into account.
	// There is also a WebKit bug on this: https://bugs.webkit.org/show_bug.cgi?id=31253.
    template <class R, bool IS_WEAK_REFERENCE = false> class CachedResourceHandle : public CachedResourceHandleBase {
    public: 
        CachedResourceHandle() : CachedResourceHandleBase(IS_WEAK_REFERENCE) { }
        CachedResourceHandle(R* res) : CachedResourceHandleBase(res, IS_WEAK_REFERENCE) { }
        CachedResourceHandle(const CachedResourceHandle<R>& o) : CachedResourceHandleBase(o) { }

        R* get() const { return reinterpret_cast<R*>(CachedResourceHandleBase::get()); }
        R* operator->() const { return get(); }
               
        CachedResourceHandle& operator=(R* res) { ASSERT(!isWeakReference()); setResource(res); return *this; } 
        CachedResourceHandle& operator=(const CachedResourceHandle& o) { setIsWeakReference(o.isWeakReference()); setResource(o.get()); return *this; }
        bool operator==(const CachedResourceHandleBase& o) const { return get() == o.get(); }
        bool operator!=(const CachedResourceHandleBase& o) const { return get() != o.get(); }
    };
#else
    template <class R> class CachedResourceHandle : public CachedResourceHandleBase {
    public: 
        CachedResourceHandle() { }
        CachedResourceHandle(R* res);
        CachedResourceHandle(const CachedResourceHandle<R>& o) : CachedResourceHandleBase(o) { }

        R* get() const { return reinterpret_cast<R*>(CachedResourceHandleBase::get()); }
        R* operator->() const { return get(); }
               
        CachedResourceHandle& operator=(R* res) { setResource(res); return *this; } 
        CachedResourceHandle& operator=(const CachedResourceHandle& o) { setResource(o.get()); return *this; }
        bool operator==(const CachedResourceHandleBase& o) const { return get() == o.get(); }
        bool operator!=(const CachedResourceHandleBase& o) const { return get() != o.get(); }
    };

    // Don't inline for winscw compiler to prevent the compiler aggressively resolving
    // the base class of R* when CachedResourceHandler<T>(R*) is inlined.  The bug is
    // reported at: https://xdabug001.ext.nokia.com/bugzilla/show_bug.cgi?id=9812.
    template <class R>
#if !COMPILER(WINSCW)
    inline
#endif
    CachedResourceHandle<R>::CachedResourceHandle(R* res) : CachedResourceHandleBase(res) 
    {
    }
#endif

    template <class R, class RR> bool operator==(const CachedResourceHandle<R>& h, const RR* res) 
    { 
        return h.get() == res; 
    }
    template <class R, class RR> bool operator==(const RR* res, const CachedResourceHandle<R>& h) 
    { 
        return h.get() == res; 
    }
    template <class R, class RR> bool operator!=(const CachedResourceHandle<R>& h, const RR* res) 
    { 
        return h.get() != res; 
    }
    template <class R, class RR> bool operator!=(const RR* res, const CachedResourceHandle<R>& h) 
    { 
        return h.get() != res; 
    }
}

#endif
