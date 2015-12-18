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
#ifndef WebPopupWindowHelper_h
#define WebPopupWindowHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo {
    template <class ImplClass>
    class WebPopupWindowHelper : private WebPopupWindow {

    public:
    
        inline WebPopupWindow* getWebPopupWindow() { return this; }
 
        static inline ImplClass* getImpl( WebPopupWindow* const pWebPopupWindow )
        {
            //ASSERT( pWebPopupWindow );
            //ASSERT( pWebPopupWindow->m_pVTable == &s_VTable );
            return getThis( pWebPopupWindow );
        }

    protected:
        WebPopupWindowHelper();
        virtual ~WebPopupWindowHelper();
        
    private:
        static WebPopupWindowVTable const s_VTable;
        static ImplClass* getThis(WebPopupWindow* const pWebPopupWindow) { return static_cast<ImplClass*>(pWebPopupWindow); }
        static const ImplClass* getThis(const WebPopupWindow* const pWebPopupWindow) { return static_cast<const ImplClass*>(pWebPopupWindow); }

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAddOption( struct WebPopupWindow* pPopupWindow
                                                                        , const uint16_t* const pUTF16OptionText
                                                                        , unsigned long const numOptionTextCodeUnits
                                                                        , int const itemIndex
                                                                        , bool const isSelected
                                                                        , bool const isDisabled
                                                                        , bool const isChildOfGroup
                                                                        , float const naturalTextWidth
                                                                        );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAddGroupLabel( struct WebPopupWindow* pPopupWindow
                                                                            , const uint16_t* const pUTF16OptionText
                                                                            , unsigned long const numOptionTextCodeUnits
                                                                            , int const itemIndex
                                                                            , bool const isDisabled
                                                                            , float const naturalTextWidth
                                                                            );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAddSeparator( struct WebPopupWindow* pPopupWindow
                                                                           , int const itemIndex
                                                                           );
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sShow(struct WebPopupWindow* pPopupWindow, int const windowX, int const windowY, int const popupWindowWidth);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sHide(struct WebPopupWindow* pPopupWindow);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sUpdateSelectedItem(struct WebPopupWindow* pPopupWindow, int const itemIdex);
    };

    template <class ImplClass>
    WebPopupWindowVTable const WebPopupWindowHelper<ImplClass>::s_VTable = {
        sizeof(WebPopupWindowVTable),
        WebPopupWindowHelper<ImplClass>::sAddOption,
        WebPopupWindowHelper<ImplClass>::sAddGroupLabel,
        WebPopupWindowHelper<ImplClass>::sAddSeparator,
        WebPopupWindowHelper<ImplClass>::sShow,
        WebPopupWindowHelper<ImplClass>::sHide,
        WebPopupWindowHelper<ImplClass>::sUpdateSelectedItem,
    };

    template<class ImplClass>
    WebPopupWindowHelper<ImplClass>::WebPopupWindowHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
    WebPopupWindowHelper<ImplClass>::~WebPopupWindowHelper()
    {
        m_pVTable = 0;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowHelper<ImplClass>::sAddOption ( struct WebPopupWindow* pPopupWindow
                                                                          , const uint16_t* const pUTF16OptionText
                                                                          , unsigned long const numOptionTextCodeUnits
                                                                          , int const itemIndex
                                                                          , bool const isSelected
                                                                          , bool const isDisabled
                                                                          , bool const isChildOfGroup
                                                                          , float const naturalTextWidth
                                                                          )
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        if (pThis->isUnusable()) return;
        pThis->addOption( pUTF16OptionText, numOptionTextCodeUnits, itemIndex, isSelected, isDisabled, isChildOfGroup, naturalTextWidth );
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowHelper<ImplClass>::sAddGroupLabel ( struct WebPopupWindow* pPopupWindow
                                                                              , const uint16_t* const pUTF16OptionText
                                                                              , unsigned long const numOptionTextCodeUnits
                                                                              , int const itemIndex
                                                                              , bool const isDisabled
                                                                              , float const naturalTextWidth
                                                                              )
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        if (pThis->isUnusable()) return;
        pThis->addGroupLabel( pUTF16OptionText, numOptionTextCodeUnits, itemIndex, isDisabled, naturalTextWidth );
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowHelper<ImplClass>::sAddSeparator ( struct WebPopupWindow* pPopupWindow
                                                                             , int const itemIndex
                                                                             )
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        if (pThis->isUnusable()) return;
        pThis->addSeparator( itemIndex );
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowHelper<ImplClass>::sShow( struct WebPopupWindow* pPopupWindow
                                                                    , int const windowX
                                                                    , int const windowY
                                                                    , int const popupWindowWidth
                                                                    )
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        if (pThis->isUnusable()) return;
        pThis->show(windowX, windowY, popupWindowWidth);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowHelper<ImplClass>::sHide(struct WebPopupWindow* pPopupWindow)
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        if (pThis->isUnusable()) return;
        pThis->hide();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebPopupWindowHelper<ImplClass>::sUpdateSelectedItem(struct WebPopupWindow* pPopupWindow, int const itemIndex)
    {
        ImplClass* const pThis = getThis(pPopupWindow);
        if (pThis->isUnusable()) return;
        pThis->updateSelectedItem(itemIndex);
    }

}

#endif
