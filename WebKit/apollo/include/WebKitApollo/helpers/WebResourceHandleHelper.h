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
#ifndef WebResourceHandleHelper_h
#define WebResourceHandleHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo
{
	template <class ImplClass>
	class WebResourceHandleHelper : private WebResourceHandle
	{
	public:
		inline WebResourceHandle* getWebResourceHandle() { return this; }
	protected:
		WebResourceHandleHelper();
		virtual ~WebResourceHandleHelper();
	private:
		static WebResourceHandleVTable const s_VTable;
		static ImplClass* getThis( WebResourceHandle* const pResourceLoader) { return static_cast<ImplClass*>(pResourceLoader); }
		
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAcquire(struct WebResourceHandle* pResourceLoader);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sRelease(struct WebResourceHandle* pResourceLoader);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sCancel(struct WebResourceHandle* pResourceLoader);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sKill(struct WebResourceHandle* pResourceLoader);
        static WEBKIT_APOLLO_PROTO1 WebResourceLoaderObject* WEBKIT_APOLLO_PROTO2 sGetWebResourceLoaderObject(struct WebResourceHandle* pResourceLoader);
	};
	
	template <class ImplClass>
	WebResourceHandleVTable const WebResourceHandleHelper<ImplClass>::s_VTable = {
		sizeof(WebResourceHandleVTable),
		WebResourceHandleHelper<ImplClass>::sAcquire,
		WebResourceHandleHelper<ImplClass>::sRelease,
		WebResourceHandleHelper<ImplClass>::sCancel,
		WebResourceHandleHelper<ImplClass>::sKill,
        WebResourceHandleHelper<ImplClass>::sGetWebResourceLoaderObject
	};
	
	template <class ImplClass>
	WebResourceHandleHelper<ImplClass>::WebResourceHandleHelper()
	{
		m_pVTable = &s_VTable;
	}
	
	template <class ImplClass>
	WebResourceHandleHelper<ImplClass>::~WebResourceHandleHelper()
	{
        m_pVTable = 0;
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleHelper<ImplClass>::sAcquire(struct WebResourceHandle* pResourceLoader)
	{
		ImplClass* const pThis = getThis(pResourceLoader);
		pThis->acquire();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleHelper<ImplClass>::sRelease(struct WebResourceHandle* pResourceLoader)
	{
		ImplClass* const pThis = getThis(pResourceLoader);
		pThis->release();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleHelper<ImplClass>::sCancel(struct WebResourceHandle* pResourceLoader)
	{
		ImplClass* const pThis = getThis(pResourceLoader);
		pThis->cancel();
	}
	
	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebResourceHandleHelper<ImplClass>::sKill(struct WebResourceHandle* pResourceLoader)
	{
		ImplClass* const pThis = getThis(pResourceLoader);
		pThis->kill();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebResourceLoaderObject* WEBKIT_APOLLO_PROTO2 WebResourceHandleHelper<ImplClass>::sGetWebResourceLoaderObject(struct WebResourceHandle* pResourceLoader)
	{
		ImplClass* const pThis = getThis(pResourceLoader);
		return pThis->getWebResourceLoaderObject();
	}
}

#endif
