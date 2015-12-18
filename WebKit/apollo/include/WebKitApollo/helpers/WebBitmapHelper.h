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
#ifndef WebBitmapHelper_h
#define WebBitmapHelper_h

#include <WebKitApollo/WebKit.h>
#include <stddef.h>

namespace WebKitApollo
{

	template <class ImplClass>
	class WebBitmapHelper : private WebBitmap {

	public:
		inline WebBitmap* getWebBitmap() { return this; }
    protected:
		WebBitmapHelper();
		virtual ~WebBitmapHelper();
	private:
		static WebBitmapVTable const s_VTable;
		static ImplClass* getThis( WebBitmap* pWebBitmap ) { return static_cast<ImplClass*>(pWebBitmap); }

		static WEBKIT_APOLLO_PROTO1 void* WEBKIT_APOLLO_PROTO2 sGetBitmapDataObject(struct WebBitmap* pBitmap);
		static WEBKIT_APOLLO_PROTO1 void* WEBKIT_APOLLO_PROTO2 sGetPixelData(struct WebBitmap* pBitmap);
		static WEBKIT_APOLLO_PROTO1 unsigned long WEBKIT_APOLLO_PROTO2 sGetWidth(struct WebBitmap* pBitmap);
		static WEBKIT_APOLLO_PROTO1 unsigned long WEBKIT_APOLLO_PROTO2 sGetHeight(struct WebBitmap* pBitmap);
		static WEBKIT_APOLLO_PROTO1 unsigned long WEBKIT_APOLLO_PROTO2 sGetStride(struct WebBitmap* pBitmap);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sCopyPixels(struct WebBitmap* pBitmap, struct WebBitmap* pSourceBitmap, const struct WebIntRect* sourceRect, const struct WebIntPoint* destPoint);
		static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 sGetBitmapVariant(struct WebBitmap* pBitmap);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDetach(struct WebBitmap* pBitmap);
	};

	template <class ImplClass>
	WebBitmapVTable const WebBitmapHelper<ImplClass>::s_VTable = {
		sizeof( WebBitmapVTable ),
		WebBitmapHelper<ImplClass>::sGetBitmapDataObject,
		WebBitmapHelper<ImplClass>::sGetPixelData,
		WebBitmapHelper<ImplClass>::sGetWidth,
		WebBitmapHelper<ImplClass>::sGetHeight,
		WebBitmapHelper<ImplClass>::sGetStride,
		WebBitmapHelper<ImplClass>::sCopyPixels,
		WebBitmapHelper<ImplClass>::sGetBitmapVariant,
		WebBitmapHelper<ImplClass>::sDetach
	};

    template<class ImplClass>
    WebBitmapHelper<ImplClass>::WebBitmapHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
	WebBitmapHelper<ImplClass>::~WebBitmapHelper()
	{
        m_pVTable = 0;
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void* WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sGetBitmapDataObject(WebBitmap* pBitmap)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
			return 0;
		return pThis->getBitmapDataObject();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void* WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sGetPixelData(WebBitmap* pBitmap)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
            return 0;
		return pThis->getPixelData();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	unsigned long WEBKIT_APOLLO_PROTO2 WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sGetWidth(WebBitmap* pBitmap)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
            return 0;
		return pThis->getWidth();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	unsigned long WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sGetHeight(WebBitmap* pBitmap)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
            return 0;
		return pThis->getHeight();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	unsigned long WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sGetStride(WebBitmap* pBitmap)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
            return 0;
		return pThis->getStride();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sCopyPixels(WebBitmap* pBitmap, WebBitmap *sourceBitmap, const WebIntRect *sourceRect, const WebIntPoint* destPoint)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
			return;
		pThis->copyPixels(sourceBitmap->m_pVTable->getBitmapDataObject(sourceBitmap), sourceRect, destPoint);
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sGetBitmapVariant(WebBitmap* pBitmap)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
			return NULL;
		return pThis->getBitmapVariant();
	}

	template <class ImplClass>
	WEBKIT_APOLLO_PROTO1
	void WEBKIT_APOLLO_PROTO2 WebBitmapHelper<ImplClass>::sDetach(WebBitmap* pBitmap)
	{
		ImplClass* const pThis = getThis(pBitmap);
		if (pThis->isUnusable())
			return;
		pThis->detach();
	}

};
#endif 
