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
 
#ifndef WebScriptProxyHelper_h
#define WebScriptProxyHelper_h

#include <WebKitApollo/WebKit.h>

namespace WebKitApollo {
    template <class ImplClass>
    class WebScriptProxyVariantHelper : private WebScriptProxyVariant {
    public:
        inline WebScriptProxyVariant* getApolloScriptProxyVariant() { return this; }
        inline const WebScriptProxyVariant* getApolloScriptProxyVariant() const { return this; }
        

        static bool isSameImpl(const struct WebScriptProxyVariant* const pVariant);
        static ImplClass* downCast(struct WebScriptProxyVariant* const pOtherVariant);
        static const ImplClass* downCast(const struct WebScriptProxyVariant* const pOtherVariant);
    
    protected:
        WebScriptProxyVariantHelper();
        virtual ~WebScriptProxyVariantHelper();
        
    private:
        static WebScriptProxyVariantVTable const s_VTable;
        static ImplClass* getThis(WebScriptProxyVariant* const pVariant) { return static_cast<ImplClass*>(pVariant); }
        static const ImplClass* getThis(const WebScriptProxyVariant* const pVariant) { return static_cast<const ImplClass*>(pVariant); }
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sAcquire(const struct WebScriptProxyVariant* pVariant);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sRelease(const struct WebScriptProxyVariant* pVariant);
        static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariantType WEBKIT_APOLLO_PROTO2 sGetType( const struct WebScriptProxyVariant* pVariant);
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sGetBool(const struct WebScriptProxyVariant* pVariant);
        static WEBKIT_APOLLO_PROTO1 double WEBKIT_APOLLO_PROTO2 sGetNumber(const struct WebScriptProxyVariant* pVariant);
        static WEBKIT_APOLLO_PROTO1 unsigned long WEBKIT_APOLLO_PROTO2 sGetStringUTF16Length(const struct WebScriptProxyVariant* pVariant);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetStringUTF16(const struct WebScriptProxyVariant* pVariant, uint16_t* pDest);
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sHasProperty(const struct WebScriptProxyVariant* pVariant
                                                                                   , const uint16_t* pUTF16PropertyName
                                                                                   , unsigned long utf16PropertyNameLength
																				   , void* interpreterContext);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetProperty(struct WebScriptProxyVariant* pVariant
                                                                          , const uint16_t* pUTF16PropertyName
                                                                          , unsigned long utf16PropertyNameLength
																		  , void* interpreterContext
                                                                          , WebScriptProxyInvokeGetSetResult* pPropertyGetResult);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sSetProperty(struct WebScriptProxyVariant* pVariant
                                                                          , const uint16_t* pUTF16PropertyName
                                                                          , unsigned long utf16PropertyNameLength
                                                                          , WebScriptProxyVariant* pValue
																		  , void* interpreterContext
                                                                          , WebScriptProxyInvokeGetSetResult* pPropertySetResult);
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sDeleteProperty(struct WebScriptProxyVariant* pVariant
                                                                                      , const uint16_t* pUTF16PropertyName
                                                                                      , unsigned long utf16PropertyNameLength
																					  , void* interpreterContext
                                                                                      , struct WebScriptProxyVariant** ppDeleteException);
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sCanInvoke(const struct WebScriptProxyVariant* pVariant);
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sInvoke(const struct WebScriptProxyVariant* pVariant
                                                                     , struct WebScriptProxyVariant* pThisVariant
                                                                     , unsigned long numArguments
                                                                     , struct WebScriptProxyVariant * const * argumentArray
																	 , void* interpreterContext
                                                                     , struct WebScriptProxyInvokeGetSetResult* pInvokeResult);
        
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sEnumProperties(const struct WebScriptProxyVariant* pVariant
                                                                             , WebScriptProxyVariantEnumPropertiesProcFunction enumProc
                                                                             , struct WebScriptProxyVariantPropertiesEnumData* pClientData
																			 , void* interpreterContext);

        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sCanConstruct(const struct WebScriptProxyVariant* pVariant);

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sConstruct(const struct WebScriptProxyVariant* pVariant
                                                                        , unsigned long numArguments
                                                                        , struct WebScriptProxyVariant * const * argumentArray
																		, void* interpreterContext
                                                                        , struct WebScriptProxyInvokeGetSetResult* pConstructResult);

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sDefaultValue(const struct WebScriptProxyVariant* pVariant
                                                                           , struct WebScriptProxyInvokeGetSetResult* pDefaultValueResult
                                                                           , enum WebScriptProxyVariantType typeHint
																		   , void* interpreterContext);
        static WEBKIT_APOLLO_PROTO1 WebScriptProxyVariantObjectType WEBKIT_APOLLO_PROTO2 sGetObjectType(const struct WebScriptProxyVariant* pVariant);

        
        //default implementation provided no need for sub-class to implement getScriptEngineId
        static WEBKIT_APOLLO_PROTO1 const WebScriptProxyEngineId*  WEBKIT_APOLLO_PROTO2 sGetScriptEngineId(const struct WebScriptProxyVariant* pVariant);
        
        static WEBKIT_APOLLO_PROTO1 unsigned char WEBKIT_APOLLO_PROTO2 sIsDeadVariant(const struct WebScriptProxyVariant* pVariant);

        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sGetObjectTypeName(const struct WebScriptProxyVariant* pVariant
                                                                                , uint16_t *pUTF16TypeName
                                                                                , unsigned long* pTypeNameLength);
        
    };
    
    template <class ImplClass>
    WebScriptProxyVariantVTable const WebScriptProxyVariantHelper<ImplClass>::s_VTable = {
        sizeof(WebScriptProxyVariantVTable),
        WebScriptProxyVariantHelper<ImplClass>::sAcquire,
        WebScriptProxyVariantHelper<ImplClass>::sRelease,
        WebScriptProxyVariantHelper<ImplClass>::sGetType,
        WebScriptProxyVariantHelper<ImplClass>::sGetBool,
        WebScriptProxyVariantHelper<ImplClass>::sGetNumber,
        WebScriptProxyVariantHelper<ImplClass>::sGetStringUTF16Length,
        WebScriptProxyVariantHelper<ImplClass>::sGetStringUTF16,
        WebScriptProxyVariantHelper<ImplClass>::sHasProperty,
        WebScriptProxyVariantHelper<ImplClass>::sGetProperty,
        WebScriptProxyVariantHelper<ImplClass>::sSetProperty,
        WebScriptProxyVariantHelper<ImplClass>::sDeleteProperty,
        WebScriptProxyVariantHelper<ImplClass>::sCanInvoke,
        WebScriptProxyVariantHelper<ImplClass>::sInvoke,
        WebScriptProxyVariantHelper<ImplClass>::sGetScriptEngineId,
        WebScriptProxyVariantHelper<ImplClass>::sEnumProperties,
        WebScriptProxyVariantHelper<ImplClass>::sCanConstruct,
        WebScriptProxyVariantHelper<ImplClass>::sConstruct,
        WebScriptProxyVariantHelper<ImplClass>::sDefaultValue,
        WebScriptProxyVariantHelper<ImplClass>::sGetObjectType,
        WebScriptProxyVariantHelper<ImplClass>::sIsDeadVariant,
        WebScriptProxyVariantHelper<ImplClass>::sGetObjectTypeName
    };

    template<class ImplClass>
    WebScriptProxyVariantHelper<ImplClass>::WebScriptProxyVariantHelper()
    {
        m_pVTable = &s_VTable;
    }

    template <class ImplClass>
	WebScriptProxyVariantHelper<ImplClass>::~WebScriptProxyVariantHelper()
	{
        m_pVTable = 0;
	}

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sAcquire(const struct WebScriptProxyVariant* pVariant )
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->acquire();
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sRelease(const struct WebScriptProxyVariant* pVariant )
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->release();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    WebScriptProxyVariantType WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetType(const struct WebScriptProxyVariant* pVariant )
    {
        const ImplClass* const pThis = getThis(pVariant);
        return pThis->getType();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetBool(const struct WebScriptProxyVariant* pVariant )
    {
        const ImplClass* const pThis = getThis(pVariant);
        bool const bRet = pThis->getBool();
        unsigned char const ret = bRet ? 1 : 0;
        return ret;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    double WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetNumber(const struct WebScriptProxyVariant* pVariant )
    {
        const ImplClass* const pThis = getThis(pVariant);
        return pThis->getNumber();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned long WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetStringUTF16Length(const struct WebScriptProxyVariant* pVariant )
    {
        const ImplClass* const pThis = getThis(pVariant);
        return pThis->getStringUTF16Length();
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetStringUTF16(const struct WebScriptProxyVariant* pVariant, uint16_t* pDest )
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->getStringUTF16(pDest);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sHasProperty(const struct WebScriptProxyVariant* pVariant
                                                                                            , const uint16_t* pUTF16PropertyName
                                                                                            , unsigned long utf16PropertyNameLength
																							, void* interpreterContext)
    {
        const ImplClass* const pThis = getThis(pVariant);
        bool const bRet = pThis->hasProperty(pUTF16PropertyName, utf16PropertyNameLength, interpreterContext);
        unsigned char const ret = bRet ? 1 : 0;
        return ret;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetProperty(struct WebScriptProxyVariant* pVariant
                                                                                   , const uint16_t* pUTF16PropertyName
                                                                                   , unsigned long utf16PropertyNameLength
																				   , void* interpreterContext
                                                                                   , WebScriptProxyInvokeGetSetResult* pPropertyGetResult)
    {
        ImplClass* const pThis = getThis(pVariant);
        pThis->getProperty(pUTF16PropertyName, utf16PropertyNameLength, interpreterContext, pPropertyGetResult);
    }
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sSetProperty(struct WebScriptProxyVariant* pVariant
                                                                                   , const uint16_t* pUTF16PropertyName
                                                                                   , unsigned long utf16PropertyNameLength
                                                                                   , WebScriptProxyVariant* pValue
																				   , void* interpreterContext
                                                                                   , WebScriptProxyInvokeGetSetResult* pPropertySetResult)
    {
        ImplClass* const pThis = getThis(pVariant);
        pThis->setProperty(pUTF16PropertyName, utf16PropertyNameLength, pValue, interpreterContext, pPropertySetResult);
    }
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sDeleteProperty(struct WebScriptProxyVariant* pVariant
                                                                                               , const uint16_t* pUTF16PropertyName
                                                                                               , unsigned long utf16PropertyNameLength
																							   , void* interpreterContext
                                                                                               , struct WebScriptProxyVariant** ppDeleteException)
    {
        ImplClass* const pThis = getThis(pVariant);
        bool const bRet = pThis->deleteProperty(pUTF16PropertyName, utf16PropertyNameLength, interpreterContext, ppDeleteException);
        unsigned char const ret = bRet ? 1 : 0;
        return ret;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sCanInvoke(const struct WebScriptProxyVariant* pVariant)
    {
        const ImplClass* const pThis = getThis(pVariant);
        bool const bRet = pThis->canInvoke();
        unsigned char const ret = bRet ? 1 : 0;
        return ret;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sInvoke(const struct WebScriptProxyVariant* pVariant
                                                                              , struct WebScriptProxyVariant* pThisVariant
                                                                              , unsigned long numArguments
                                                                              , struct WebScriptProxyVariant * const * argumentArray
																			  , void* interpreterContext
                                                                              , struct WebScriptProxyInvokeGetSetResult* pInvokeResult)
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->invoke(pThisVariant, numArguments, argumentArray, interpreterContext, pInvokeResult);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    const WebScriptProxyEngineId* WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetScriptEngineId(const struct WebScriptProxyVariant* pVariant)
    {
        return reinterpret_cast<const WebScriptProxyEngineId*>(pVariant->m_pVTable);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sEnumProperties(const struct WebScriptProxyVariant* pVariant
                                                                                      , WebScriptProxyVariantEnumPropertiesProcFunction enumProc
                                                                                      , struct WebScriptProxyVariantPropertiesEnumData* pClientData
																					  , void* interpreterContext)
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->enumProperties(enumProc, pClientData, interpreterContext);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sCanConstruct(const struct WebScriptProxyVariant* pVariant)
    {
        const ImplClass* pThis = getThis(pVariant);
        bool const bRet = pThis->canConstruct();
        unsigned char const ret = bRet ? 1 : 0;
        return ret;
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sConstruct(const struct WebScriptProxyVariant* pVariant
                                                                                 , unsigned long numArguments
                                                                                 , struct WebScriptProxyVariant * const * argumentArray
																				 , void* interpreterContext
                                                                                 , struct WebScriptProxyInvokeGetSetResult* pConstructResult)
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->construct(numArguments, argumentArray, interpreterContext, pConstructResult);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sDefaultValue(const struct WebScriptProxyVariant* pVariant
                                                                                    , struct WebScriptProxyInvokeGetSetResult* pDefaultValueResult
                                                                                    , enum WebScriptProxyVariantType typeHint
																				    , void* interpreterContext)
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->defaultValue(pDefaultValueResult, typeHint, interpreterContext);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    WebScriptProxyVariantObjectType WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetObjectType(const struct WebScriptProxyVariant* pVariant)
    {
        const ImplClass* const pThis = getThis(pVariant);
        WebScriptProxyVariantObjectType const result = pThis->getObjectType();
        return result;
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    bool WebScriptProxyVariantHelper<ImplClass>::isSameImpl(const struct WebScriptProxyVariant* const pVariant)
    {
        return reinterpret_cast<const WebScriptProxyEngineId*>(&s_VTable) == pVariant->m_pVTable->getScriptEngineId(pVariant);
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    ImplClass* WebScriptProxyVariantHelper<ImplClass>::downCast(struct WebScriptProxyVariant* const pVariant)
    {
        return getThis(pVariant);
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    const ImplClass* WebScriptProxyVariantHelper<ImplClass>::downCast(const struct WebScriptProxyVariant* const pVariant)
    {
        return getThis(pVariant);
    }

    template <class ImplClass>
    class PropertyEnumHelper
    {
    protected:
        PropertyEnumHelper();
        void enumProperties(const struct WebScriptProxyVariant* const pVariant, void* interpreterContext);
    private:
        static WEBKIT_APOLLO_PROTO1 void WEBKIT_APOLLO_PROTO2 sEnumProc(struct WebScriptProxyVariantPropertiesEnumData* pClientData
                                                                       , const uint16_t* pUTF16PropertyName
                                                                       , unsigned long utf16PropertyNameLength);
    };

    template <class ImplClass>
    PropertyEnumHelper<ImplClass>::PropertyEnumHelper()
    {
    }

    template <class ImplClass>
    void PropertyEnumHelper<ImplClass>::enumProperties(const struct WebScriptProxyVariant* const pVariant, void* interpreterContext)
    {
        struct WebScriptProxyVariantPropertiesEnumData* const pOpaqueThis =
            reinterpret_cast< struct WebScriptProxyVariantPropertiesEnumData* >(static_cast<ImplClass*>(this));
        pVariant->m_pVTable->enumProperties(pVariant, sEnumProc, pOpaqueThis, interpreterContext);
    }

    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 PropertyEnumHelper<ImplClass>::sEnumProc(struct WebScriptProxyVariantPropertiesEnumData* pClientData, const uint16_t* pUTF16PropertyName, unsigned long utf16PropertyNameLength)
    {
        ImplClass* const pThis =
            reinterpret_cast<ImplClass*>(pClientData);
        pThis->enumProc(pUTF16PropertyName, utf16PropertyNameLength);
    }
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    unsigned char WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sIsDeadVariant(const struct WebScriptProxyVariant* pVariant)
    {
    	const ImplClass* pThis = getThis(pVariant);
        bool const bRet = pThis->isDeadVariant();
        unsigned char const ret = bRet ? 1 : 0;
        return ret;
    } 
    
    template <class ImplClass>
    WEBKIT_APOLLO_PROTO1
    void WEBKIT_APOLLO_PROTO2 WebScriptProxyVariantHelper<ImplClass>::sGetObjectTypeName(const struct WebScriptProxyVariant* pVariant
                                                                                        , uint16_t* pUTF16TypeName
                                                                                        , unsigned long* pTypeNameLength)
    {
        const ImplClass* const pThis = getThis(pVariant);
        pThis->getObjectTypeName(pUTF16TypeName, pTypeNameLength);
    }   
}

#endif
