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
#ifndef WebFrameHelper_h
#define WebFrameHelper_h

#include <WebKitApollo/WebKit.h>
#include <stddef.h>

namespace WebKitApollo
{
    template <class ImplClass>
    class WebFrameHelper : private WebFrameApollo {

    public:
        inline WebFrameApollo* getWebFrame() { return this; }
    protected:
        WebFrameHelper();
        virtual ~WebFrameHelper();
    private:
        static WebFrameVTable const s_VTable;
        static ImplClass* getThis( WebFrameApollo* const pWebFrame ) { return static_cast< ImplClass*>( pWebFrame ); }
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sLoadHTMLString(struct WebFrameApollo* pWebFrame, const char* utf8HTMLString, size_t numHTMLStringBytes, const unsigned char* baseURL);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sLoadRequest(struct WebFrameApollo* pWebFrame, const WebResourceRequest* requestToLoad);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sEvalJavaScriptString(struct WebFrameApollo* pWebFrame, const uint16_t* utf16Text, unsigned long numUTF16CodeUnits);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sReload(struct WebFrameApollo* pWebFrame);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sStopLoading(struct WebFrameApollo* pWebFrame);
        static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 sGetGlobalObject(struct WebFrameApollo* pWebFrame);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sHistoryGo(struct WebFrameApollo* pWebFrame, int steps );
		static WEBKIT_APOLLO_PROTO1 unsigned WEBKIT_APOLLO_PROTO2 sGetHistoryLength(struct WebFrameApollo* pWebFrame);
		static WEBKIT_APOLLO_PROTO1 unsigned WEBKIT_APOLLO_PROTO2 sGetHistoryPosition(struct WebFrameApollo* pWebFrame);
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetHistoryPosition(struct WebFrameApollo* pWebFrame, unsigned pos );
		static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetHistoryAt(struct WebFrameApollo* pWebFrame, unsigned position, char** pUrl, char** pOriginalUrl, bool* isPost, uint16_t** pTitle );

    };

    template <class ImplClass>
    WebFrameVTable const WebFrameHelper<ImplClass>::s_VTable = {
        sizeof(WebFrameVTable),
        WebFrameHelper<ImplClass>::sLoadHTMLString,
        WebFrameHelper<ImplClass>::sLoadRequest,
		WebFrameHelper<ImplClass>::sEvalJavaScriptString,
        WebFrameHelper<ImplClass>::sReload,
        WebFrameHelper<ImplClass>::sStopLoading,
        WebFrameHelper<ImplClass>::sGetGlobalObject,
        WebFrameHelper<ImplClass>::sHistoryGo,
        WebFrameHelper<ImplClass>::sGetHistoryLength,
        WebFrameHelper<ImplClass>::sGetHistoryPosition,
        WebFrameHelper<ImplClass>::sSetHistoryPosition,
        WebFrameHelper<ImplClass>::sGetHistoryAt,
    };

    template <class ImplClass>
    WebFrameHelper<ImplClass>::WebFrameHelper()
    {
        m_pVTable = &s_VTable;
    }
    
    template <class ImplClass>
    WebFrameHelper<ImplClass>::~WebFrameHelper()
    {
        m_pVTable = 0;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sLoadHTMLString(struct WebFrameApollo* pWebFrame, const char* utf8HTMLString, size_t numHTMLStringBytes, const unsigned char* baseURL)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->loadHTMLString(utf8HTMLString, numHTMLStringBytes, baseURL);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sLoadRequest(struct WebFrameApollo* pWebFrame, const WebResourceRequest* requestToLoad)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->loadRequest(requestToLoad);
    }
	
	template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sEvalJavaScriptString(struct WebFrameApollo* pWebFrame, const uint16_t* utf16Text, unsigned long numUTF16CodeUnits)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->evalJavaScriptString(utf16Text, numUTF16CodeUnits);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sReload(struct WebFrameApollo* pWebFrame)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->reload();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sStopLoading(struct WebFrameApollo* pWebFrame)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->stopLoading();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    WebScriptProxyVariant* WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sGetGlobalObject(struct WebFrameApollo* pWebFrame)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        return pThis->getGlobalObject();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sHistoryGo(struct WebFrameApollo* pWebFrame, int steps )
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->historyGo( steps);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    unsigned WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sGetHistoryLength(struct WebFrameApollo* pWebFrame)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        return pThis->getHistoryLength();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    unsigned WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sGetHistoryPosition(struct WebFrameApollo* pWebFrame)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        return pThis->getHistoryPosition();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sSetHistoryPosition(struct WebFrameApollo* pWebFrame, unsigned position)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        return pThis->setHistoryPosition(position);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1 
    void WEBKIT_APOLLO_PROTO2 WebFrameHelper<ImplClass>::sGetHistoryAt(struct WebFrameApollo* pWebFrame,
	    unsigned position, char** pUrl, char** pOriginalUrl, bool* pIsPost, uint16_t** pTitle)
    {
        ImplClass* const pThis = getThis(pWebFrame);
        pThis->getHistoryAt(position, pUrl, pOriginalUrl, pIsPost, pTitle);
    }

}

#endif
