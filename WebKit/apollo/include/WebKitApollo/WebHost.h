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
#ifndef WebHost_h
#define WebHost_h

#include <WebKitApollo/WebConfig.h>
#include <WebKitApollo/WebCursor.h>
#include <WebKitApollo/WebResourceHandle.h>
#include <WebKitApollo/WebNavigationPolicyAction.h>
#include <WebKitApollo/WebNavigationType.h>

struct WebHost;
struct WebBitmap;
struct WebError;
struct WebIntRect;
struct WebPopupWindow;
struct WebResourceHandle;
struct WebResourceHandleClient;
struct WebResourceRequest;
struct WebScriptProxyVariant;
struct WebString;
struct WebViewApollo;
struct WebString;

struct WebHostCreateWindowArgs
{
	unsigned long windowArgsSize;

	enum { USE_DEFAULT = 0x7fffffff };
	int x, y, width, height;

	bool menuBarVisible;
	bool statusBarVisible;
	bool toolBarVisible;
	bool locationBarVisible;
	bool scrollBarsVisible;
	bool resizable;
	bool fullscreen;
};

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostUpdateLocationFunction )( struct WebHost* pWebHost
                                                                                            , const unsigned char* urlBytes );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostLoadEndFunction )( struct WebHost* pWebHost
                                                                                  , unsigned char success );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostSetTitleFunction )( struct WebHost* pWebHost
                                                                                    , const uint16_t* pUTF16Title
																					, unsigned long numTitleCodeUnits );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostSetStatusTextFunction )( struct WebHost* pWebHost
																						, const uint16_t* pUTF16Status
																						, unsigned long numStatusCodeUnits );
typedef WEBKIT_APOLLO_PROTO1 WebString* (WEBKIT_APOLLO_PROTO2 *WebHostGetUserAgentTextFunction )( struct WebHost* pWebHost);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostUncaughtJSExceptionFunction )( struct WebHost* pWebHost
                                                                                              , struct WebScriptProxyVariant* pExceptionVariant );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostRunJSAlertFunction )(  struct WebHost* pWebHost,
                                                                                        const char* pUTF8Message );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebHostRunJSConfirmFunction )( struct WebHost* pWebHost
                                                                                                , const char* pUTF8Message );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebHostRunJSPromptFunction )( struct WebHost* pWebHost
                                                                                               , const char* pUTF8Prompt
                                                                                               , const char* pUTF8DefaultValue
                                                                                               , char** ppUTF8Result );
typedef WEBKIT_APOLLO_PROTO1 WebResourceHandle* ( WEBKIT_APOLLO_PROTO2 *WebHostLoadResourceFunction )( struct WebHost* pWebHost
                                                                                                     , struct WebResourceRequest* pRequest
                                                                                                     , struct WebResourceHandleClient* pClient );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostLoadResourceSynchronouslyFunction )( struct WebHost* pWebHost
                                                                                                    , struct WebResourceRequest* pRequest
                                                                                                    , struct WebResourceHandleClient* pClient );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostHandleOnLoadEventsFunction )( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostHandleOnDocumentCreatedFunction )( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 struct WebViewApollo* ( WEBKIT_APOLLO_PROTO2 *WebHostCreateNewWindowFunction )( struct WebHost* pWebHost
																									 , struct WebHostCreateWindowArgs* pWindowArgs );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostCloseWindowFunction )( struct WebHost* pWebHost );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostBeginModalFunction )( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostEndModalFunction )( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 uint16_t* ( WEBKIT_APOLLO_PROTO2 *WebHostOpenFileChooserFunction )( struct WebHost* pWebHost
                                                                                               , unsigned long* pFilePathLength );

typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebHostPDFCapabilityFunction )(	struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebHostLoadPDFPluginFunction )(	struct WebHost* pWebHost, void **pPDFPluginObject );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebHostUnloadPDFPluginFunction )(	struct WebHost* pWebHost, void *pdfPluginObject );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostHandlePDFErrorFunction )(	struct WebHost* pWebHost, int pdfErrorNum );

typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* ( WEBKIT_APOLLO_PROTO2 *WebHostGetEventObjectFunction ) (	struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* ( WEBKIT_APOLLO_PROTO2 *WebHostGetRootPackageFunction ) (	struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* ( WEBKIT_APOLLO_PROTO2 *WebHostGetWKMethodsFunction ) (	struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* ( WEBKIT_APOLLO_PROTO2 *WebHostMakeDoorFunction ) (	struct WebHost* pWebHost, struct WebScriptProxyVariant* pProxy );
typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariant* ( WEBKIT_APOLLO_PROTO2 *WebHostGetHtmlControlFunction ) (	struct WebHost* pWebHost );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostSetMouseCursorFunction ) (	struct WebHost* pWebHost, WebCursorType cursorType );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostSetMouseCaptureFunction ) ( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostReleaseMouseCaptureFunction ) ( struct WebHost* pWebHost );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostCompositionSelectionChangedFunction ) ( struct WebHost* pWebHost, int start, int end );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostCompositionAbandonedFunction ) ( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostSetInputMethodStateFunction ) ( struct WebHost* pWebHost, unsigned char enable );
typedef WEBKIT_APOLLO_PROTO1 bool ( WEBKIT_APOLLO_PROTO2 *WebHostGetInputMethodStateFunction ) ( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostActivateIMEForPluginFunction ) ( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostDeactivateIMEForPluginFunction ) ( struct WebHost* pWebHost );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostSelectionChangedFunction ) ( struct WebHost* pWebHost );

typedef WEBKIT_APOLLO_PROTO1 const uint16_t* ( WEBKIT_APOLLO_PROTO2 *WebHostGetAppResourcePathFunction )( struct WebHost* pWebHost, unsigned long* pAppResourcePathLength );
typedef WEBKIT_APOLLO_PROTO1 const uint16_t* ( WEBKIT_APOLLO_PROTO2 *WebHostGetAppStoragePathFunction )( struct WebHost* pWebHost, unsigned long* pAppStoragePathLength );
typedef WEBKIT_APOLLO_PROTO1 WebPopupWindow* ( WEBKIT_APOLLO_PROTO2 *WebHostCreatePopupWindow )( struct WebHost* pWebHost, struct WebPopupWindowClient* pWebPopupWindowClient, float computedFontSize );

typedef WEBKIT_APOLLO_PROTO1 WebBitmap* ( WEBKIT_APOLLO_PROTO2 *WebHostCreateBitmapFunction )( struct WebHost* pWebHost, unsigned long width, unsigned long height );
typedef WEBKIT_APOLLO_PROTO1 WebBitmap* ( WEBKIT_APOLLO_PROTO2 *WebHostCreateBitmapFromBitmapDataObjectFunction )( struct WebHost* pWebHost, WebScriptProxyVariant* bitmapDataVariant );

typedef WEBKIT_APOLLO_PROTO1 void* ( WEBKIT_APOLLO_PROTO2 *WebHostGetPlatformInterpreterContext )( struct WebHost* pWebHost );

typedef WEBKIT_APOLLO_PROTO1 WebNavigationPolicyAction ( WEBKIT_APOLLO_PROTO2 *WebHostDispatchDecidePolicyForNavigationActionFunction)( struct WebHost* pWebHost, WebString* url, WebNavigationTypeApollo navType, struct WebResourceRequest*);
typedef WEBKIT_APOLLO_PROTO1 WebNavigationPolicyAction ( WEBKIT_APOLLO_PROTO2 *WebHostDispatchDecidePolicyForNewWindowActionFunction)( struct WebHost* pWebHost, unsigned const char* urlBytes, unsigned long numURLBytes, WebNavigationTypeApollo navType, struct WebResourceRequest*, const uint16_t* frameNameUTF16, unsigned long numFrameNameUTF16CodeUnits);

typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebHostGetLanguage )( struct WebHost* pWebHost );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebHostCanShowPluginsFunction )( struct WebHost* webHost, bool* canShowPlugins, bool* canShowWindowedPlugins );

// the following type is passed as a callback to enterPlayer
typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2 *WebHostEnterPlayerCallback)( void* arg );
// enters the player and calls the callback by passing the "arg" to it
typedef WEBKIT_APOLLO_PROTO1 bool (WEBKIT_APOLLO_PROTO2 *WebHostEnterPlayer)( struct WebHost* webHost, WebHostEnterPlayerCallback callback, void* arg );

typedef WEBKIT_APOLLO_PROTO1 bool ( WEBKIT_APOLLO_PROTO2 *WebHostIsPlatformAPIVersion )( struct WebHost* webHost, uint32_t airVersionToCheck);

struct WebHostVTable {
    unsigned long m_vTableSize;
    WebHostUpdateLocationFunction updateLocation;
    WebHostLoadEndFunction loadEnd;
    WebHostSetTitleFunction setTitle;
    WebHostSetStatusTextFunction setStatusText;
	WebHostGetUserAgentTextFunction getUserAgentText;   // returns alias to user-agent text; does not need to be freed
    WebHostUncaughtJSExceptionFunction uncaughtJSException;
    WebHostRunJSAlertFunction runJSAlert;
    WebHostRunJSConfirmFunction runJSConfirm;
    WebHostRunJSPromptFunction runJSPrompt;
    WebHostLoadResourceFunction loadResource;
    WebHostLoadResourceSynchronouslyFunction loadResourceSynchronously;
    WebHostHandleOnLoadEventsFunction handleOnLoadEvents;
    WebHostHandleOnDocumentCreatedFunction handleOnDocumentCreated;
    WebHostCreateNewWindowFunction createNewWindow;
    WebHostCloseWindowFunction closeWindow;
    WebHostBeginModalFunction beginModal;
    WebHostEndModalFunction endModal;
	WebHostPDFCapabilityFunction pdfCapability;
	WebHostLoadPDFPluginFunction loadPDFPlugin;
	WebHostUnloadPDFPluginFunction unloadPDFPlugin;
	WebHostHandlePDFErrorFunction handlePDFError;
    WebHostOpenFileChooserFunction openFileChooser;
    WebHostGetEventObjectFunction getEventObject;
    WebHostGetRootPackageFunction getRootPackage;
    WebHostGetWKMethodsFunction getWKMethods;
    WebHostMakeDoorFunction makeDoor;
    WebHostGetHtmlControlFunction getHtmlControl;
    WebHostSetMouseCursorFunction setMouseCursor;
    WebHostSetMouseCaptureFunction setMouseCapture;
    WebHostReleaseMouseCaptureFunction releaseMouseCapture;
    WebHostCompositionSelectionChangedFunction compositionSelectionChanged;
    WebHostCompositionAbandonedFunction compositionAbandoned;
    WebHostSetInputMethodStateFunction setInputMethodState;
	WebHostGetInputMethodStateFunction getInputMethodState;
    WebHostActivateIMEForPluginFunction activateIMEForPlugin;
    WebHostDeactivateIMEForPluginFunction deactivateIMEForPlugin;
    WebHostSelectionChangedFunction selectionChanged;
	WebHostGetAppResourcePathFunction getAppResourcePath;
	WebHostGetAppStoragePathFunction getAppStoragePath;
	WebHostCreatePopupWindow createPopupWindow;
	WebHostCreateBitmapFunction createBitmap;
	WebHostCreateBitmapFromBitmapDataObjectFunction createBitmapFromBitmapDataObject;
    WebHostGetPlatformInterpreterContext getPlatformInterpreterContext;
    WebHostDispatchDecidePolicyForNavigationActionFunction dispatchDecidePolicyForNavigationAction;
    WebHostDispatchDecidePolicyForNewWindowActionFunction dispatchDecidePolicyForNewWindowAction;
    WebHostGetLanguage getLanguage;
    WebHostCanShowPluginsFunction canShowPlugins;
	WebHostEnterPlayer enterPlayer;
	WebHostIsPlatformAPIVersion isPlatformAPIVersion;
};

struct WebHost {
    const struct WebHostVTable* m_pVTable;
};

#endif
