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
#ifndef WebKit_h
#define WebKit_h

#include <WebKitApollo/WebConfig.h>

#include <WebKitApollo/WebHost.h>
#include <WebKitApollo/WebBitmap.h>
#include <WebKitApollo/WebView.h>
#include <WebKitApollo/WebFrame.h>
#include <WebKitApollo/WebError.h>
#include <WebKitApollo/WebPoint.h>
#include <WebKitApollo/WebPopupWindow.h>
#include <WebKitApollo/WebPopupWindowClient.h>
#include <WebKitApollo/WebRect.h>
#include <WebKitApollo/WebWindow.h>
#include <WebKitApollo/WebResourceRequest.h>
#include <WebKitApollo/WebResourceResponse.h>
#include <WebKitApollo/WebResourceHandleClient.h>
#include <WebKitApollo/WebResourceHandle.h>
#include <WebKitApollo/WebScriptProxy.h>
#include <WebKitApollo/WebString.h>
#include <WebKitApollo/WebDebugListener.h>
#include <WebKitApollo/WebDebugState.h>
#include <WebKitApollo/BreakIteratorType.h>
#include <WebKitApollo/WebSamplerMembersList.h>
#include <WebKitApollo/WebSampler.h>
#include <WebKitApollo/WebSamplerListener.h>

struct WebFileStream;

// mapChar operations
enum CharMapping
{
	CharMapping_LowerCase,
	CharMapping_UpperCase,
	CharMapping_FoldCase,
	CharMapping_TitleCase,
	CharMapping_MirrorChar,
	CharMapping_DigitValue,
	CharMapping_BidirectionalClass,
	CharMapping_GeneralCatagory,
	CharMapping_Script,
	CharMapping_TypographicClusterBreakClass,
	CharMapping_LineBreakClass,
	CharMapping_WordBreakClass,
	CharMapping_SentenceBreakClass,
	CharMapping_NonSpacingMark,
	CharMapping_JoiningType,
	CharMapping_FontOrCompatDecompositionType
};



// mapString operations
enum StringMapping
{
	StringMapping_LowerCase,
	StringMapping_UpperCase,
	StringMapping_FoldCase,
};

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebFreeBytesFunction )(void*);
typedef WEBKIT_APOLLO_PROTO1 void* ( WEBKIT_APOLLO_PROTO2 *WebAllocBytesFunction )(unsigned);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSharedTimerSetFireTimeFunction )(int);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSharedTimerFiredFunction )();
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSharedTimerSetFiredFunctionFunction )(WebSharedTimerFiredFunction);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSharedTimerStopFunction )();
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSharedTimerStopFunction )();
typedef WEBKIT_APOLLO_PROTO1 unsigned long ( WEBKIT_APOLLO_PROTO2 *WebSharedScreenGetDepth )(WebWindow *window);
typedef WEBKIT_APOLLO_PROTO1 unsigned long ( WEBKIT_APOLLO_PROTO2 *WebSharedScreenGetDepthPerComponent )( WebWindow *window );
typedef WEBKIT_APOLLO_PROTO1 bool ( WEBKIT_APOLLO_PROTO2 *WebSharedScreenIsMonochrome )( WebWindow *window );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSharedScreenGetRect )( WebWindow *window, struct WebFloatRect* pRect );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSharedScreenGetUsableRect )( WebWindow *window, struct WebFloatRect* pRect );
typedef WEBKIT_APOLLO_PROTO1 unsigned char* ( WEBKIT_APOLLO_PROTO2 *WebGetFlashPlayerPluginPathUTF8Bytes )( unsigned long* numPathBytes );
typedef WEBKIT_APOLLO_PROTO1 const unsigned char* ( WEBKIT_APOLLO_PROTO2 *WebGetUserAgentForFlashNPP )( unsigned long* numUTF8Bytes );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebSetJavaScriptCookies )( const WebString* url, const WebString* policyURL, const WebString* value );
typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebGetJavaScriptCookies )( const WebString* url );
typedef WEBKIT_APOLLO_PROTO1 char* ( WEBKIT_APOLLO_PROTO2 *GetMIMETypeFromExtension )( const char* extension );
typedef WEBKIT_APOLLO_PROTO1 const char* ( WEBKIT_APOLLO_PROTO2 *GetLocalizedText )( const char* stringIdentifier );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebGetFileSize )( const WebString* path, long long* result );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebGetFileModTime )( const WebString* path, WebTime* result );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebGetFileExists )( const WebString* path );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebDeleteFile )( const WebString* path );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebDeleteEmptyDirectory )( const WebString* path );
typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebAppendComponentToPath )( const WebString* path, const WebString* component );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebMakeAllDirectories )( const WebString* path );
typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebPathGetFileName )( const WebString* path );
typedef WEBKIT_APOLLO_PROTO1 WebString* ( WEBKIT_APOLLO_PROTO2 *WebOpenTempFile )( const WebString* prefix, WebFileStream** handle );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebCloseFile )( WebFileStream* handle );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *WebWriteToFile )( WebFileStream* handle, const char* data, int length );
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebUnloadModule )(WebModule module);

typedef WEBKIT_APOLLO_PROTO1 WebDebugListener* ( WEBKIT_APOLLO_PROTO2 *WebGetDebugListener )();
typedef WEBKIT_APOLLO_PROTO1 bool ( WEBKIT_APOLLO_PROTO2 *WebIsDebuggingEnabled )();
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebLogWarning )( WebString* msg );

typedef WEBKIT_APOLLO_PROTO1 void* ( WEBKIT_APOLLO_PROTO2 *NewBreakIterator )( BreakIteratorType type, const uint16_t*, int length );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *DeleteBreakIterator )( void* tb );

typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *TextBreakFirst )( void* tb );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *TextBreakLast )( void* tb );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *TextBreakPrev )( void* tb );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *TextBreakNext )( void* tb );

typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *TextBreakCurrent )( void* tb );
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *TextBreakPreceding )( void* tb, int pos);
typedef WEBKIT_APOLLO_PROTO1 int ( WEBKIT_APOLLO_PROTO2 *TextBreakFollowing )( void* tb, int pos);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *TextBreakSetText )(  void* tb, const uint16_t*, int length );
typedef WEBKIT_APOLLO_PROTO1 bool ( WEBKIT_APOLLO_PROTO2 *IsTextBreak )( void* tb, int pos);

typedef WEBKIT_APOLLO_PROTO1 uint32_t ( WEBKIT_APOLLO_PROTO2 *ToUpper )( uint32_t c );
typedef WEBKIT_APOLLO_PROTO1 uint32_t ( WEBKIT_APOLLO_PROTO2 *ToLower )( uint32_t c );
typedef WEBKIT_APOLLO_PROTO1 uint32_t ( WEBKIT_APOLLO_PROTO2 *MapChar )( CharMapping, uint32_t c );
typedef WEBKIT_APOLLO_PROTO1 int32_t ( WEBKIT_APOLLO_PROTO2 *MapString )( StringMapping mapping, uint16_t *dest, int32_t destCapacity, uint16_t *src, int32_t srcLength, int32_t* error );

// The CRT rand funtion is not very random on windows, and the rand_s function does not work on win2k.
// In Apollo, the Runtime has a good working random number generator so we'll just thunk back to the runtime to get
// random numbers.

struct WebRandomNumberGeneratorState;

// hard code the size, we can use compile time asserts in code that knows what is in WebRandomNumberGeneratorState
// to make sure this hard coded value is big enough.
static const unsigned long WebRandomNumberGeneratorStateSize = 12;

typedef WEBKIT_APOLLO_PROTO1 void (WEBKIT_APOLLO_PROTO2 *WebInitRandomNumberGeneratorFunction)(WebRandomNumberGeneratorState*);
typedef WEBKIT_APOLLO_PROTO1 double (WEBKIT_APOLLO_PROTO2 *WebGetRandomNumberFunction)(WebRandomNumberGeneratorState*);


typedef WEBKIT_APOLLO_PROTO1 bool (WEBKIT_APOLLO_PROTO2 *WebGetCopyKeyState)();
typedef WEBKIT_APOLLO_PROTO1 bool (WEBKIT_APOLLO_PROTO2 *WebGetCapsLockState)();

static const unsigned int KEY_SHIFT_MASK = 0x00000001;
static const unsigned int KEY_CTRL_MASK = 0x00000002;
static const unsigned int KEY_ALT_MASK = 0x00000004;
static const unsigned int KEY_META_MASK = 0x00000008;
typedef WEBKIT_APOLLO_PROTO1 unsigned int (WEBKIT_APOLLO_PROTO2 *WebGetKeyModifiersState)();

typedef WEBKIT_APOLLO_PROTO1 WebString* (WEBKIT_APOLLO_PROTO2 *GetPDFPluginPathWebString)();

typedef WEBKIT_APOLLO_PROTO1 WebSamplerListener* (WEBKIT_APOLLO_PROTO2 *GetWebSamplerListener)();


struct WebKitAPIHostFunctions
{
    unsigned long m_size;
    WebFreeBytesFunction freeBytes;
    WebAllocBytesFunction allocBytes;
    WebSharedTimerSetFireTimeFunction setSharedTimerFireTime;
    WebSharedTimerSetFiredFunctionFunction setSharedTimerFiredFunction;
    WebSharedTimerStopFunction stopSharedTimer;
    WebSharedScreenGetDepth screenGetDepth;
    WebSharedScreenGetDepthPerComponent screenGetDepthPerComponent;
    WebSharedScreenIsMonochrome screenIsMonochrome;
    WebSharedScreenGetRect screenGetRect;
    WebSharedScreenGetUsableRect screenGetUsableRect;
    WebGetFlashPlayerPluginPathUTF8Bytes getFlashPlayerPluginPathUTF8Bytes;
    WebGetUserAgentForFlashNPP getUserAgentForFlashNPP;
    WebInitRandomNumberGeneratorFunction initRandomNumberGenerator;
    WebGetRandomNumberFunction getRandomNumber;
    WebSetJavaScriptCookies setJavaScriptCookies;
    WebGetJavaScriptCookies getJavaScriptCookies;
    GetMIMETypeFromExtension getMIMETypeFromExtension;
    GetLocalizedText getLocalizedText;

    WebGetFileSize getFileSize;
    WebGetFileModTime getFileModTime;
    WebGetFileExists getFileExists;
    WebDeleteFile deleteFile;
    WebDeleteEmptyDirectory deleteEmptyDirectory;
    WebAppendComponentToPath addComponentToPath;
    WebMakeAllDirectories makeAllDirectories;
    WebPathGetFileName pathGetFileName;
    WebOpenTempFile openTempFile;
    WebCloseFile closeFile;
    WebWriteToFile writeToFile;
    WebUnloadModule unloadModule;

    WebGetDebugListener getWebDebugListener;
    WebIsDebuggingEnabled isDebuggingEnabled;
    WebLogWarning logWarning;
    
    WebGetCapsLockState getCapsLockState;
	WebGetKeyModifiersState getKeyModifiersState;
    WebGetCopyKeyState getCopyKeyState;

    NewBreakIterator newBreakIterator;
    DeleteBreakIterator deleteBreakIterator;

    TextBreakFirst textBreakFirst;
	TextBreakLast textBreakLast;
	TextBreakPrev textBreakPrev;
    TextBreakNext textBreakNext;
    TextBreakCurrent textBreakCurrent;
    TextBreakPreceding textBreakPreceding;
    TextBreakFollowing textBreakFollowing;
	TextBreakSetText textBreakSetText;
    IsTextBreak isTextBreak;

    MapChar mapChar;
	MapString mapString;

    GetPDFPluginPathWebString getPDFPluginPathWebString;
    
    GetWebSamplerListener getWebSamplerListener;
};





typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebUpdateDirtyDocumentsFunction )();
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebGarbageCollectNow )();
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebDestroy )();


struct WebKitAPI
{
    unsigned long m_size;
    WebViewCreateFunction webViewCreate;
    WebUpdateDirtyDocumentsFunction updateDirtyDocuments;
    WebGarbageCollectNow webGarbageCollectNow;
    WebDestroy webDestroy;
};                                                                                              
                                                                                                    
typedef WEBKIT_APOLLO_PROTO1 const WebKitAPI* ( WEBKIT_APOLLO_PROTO2 *WebKitGetAPIFunction )( const WebKitAPIHostFunctions* pHostFunctions, WebError* pErrorInfo );

static const char WebKitGetAPIFunctionName[] = "WebKitGetAPI";
static const wchar_t WebKitGetAPIFunctionNameW[] = L"WebKitGetAPI";


enum WEBKIT_APOLLO_APIVersion {
    WEBKIT_APOLLO_AIR_1_5,
    WEBKIT_APOLLO_AIR_2_0,
    WEBKIT_APOLLO_AIR_LAST
};

#endif

