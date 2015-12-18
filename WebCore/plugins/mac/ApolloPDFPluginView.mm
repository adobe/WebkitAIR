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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#import <WebKitApollo/WebWindow.h>
#import <WebKitApollo/WebHost.h>

#import <WebKit/WebScriptObject.h>
#import <WebKit/WebPluginViewFactory.h>

#include "ApolloPDFPluginView.h"
#include "WebPDFPluginProtocol.h"
#include "ResourceLoader.h"
#include "Document.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "HostWindow.h"
#include "FrameLoaderClientApollo.h"
#include "Timer.h"

#include <wtf/text/CString.h>

#include "ScriptController.h"
#include "runtime_root.h"
#include "objc_instance.h"
#include <runtime/JSLock.h>

#define LOG_PDFIMPL_CALLS		(0 && defined(DEBUG) && DEBUG)

#define DEBUG_RETAIN_RELEASE	(0 && defined(DEBUG) && DEBUG)

#define	DUMP_VIEWS				(0 && defined(DEBUG) && DEBUG)

#if defined(DEBUG) && DEBUG
static char sDBGRectStr[200];
char *SprintfNSRect( NSRect *rect );
char *SprintfNSRect( NSRect *rect )
{
    float wleft = rect->origin.x;
    float wtop = rect->origin.y;
    float wwidth = rect->size.width;
    float wheight = rect->size.height;
    sprintf( sDBGRectStr, "[ %.1f, %.1f / %.1f, %.1f ]",
            wleft, wtop, wleft + wwidth, wtop + wheight );
    return sDBGRectStr;
}
#endif

#if DUMP_VIEWS
inline void DumpViewTree( NSView *view, int level )
{
    
    static char sNesting[] = "                              "
    "                              "
    "                              "
    "                              "
    "                              "
    "                              ";
    
    const char *viewClass = object_getClassName( [ view class ] );
    NSRect viewFrame = [ view convertRect: [ view bounds ] toView: nil ];
    float left = viewFrame.origin.x;
    float top = viewFrame.origin.y;
    float width = viewFrame.size.width;
    float height = viewFrame.size.height;
    
    if ( level == 0 )
    {
        float wleft = viewFrame.origin.x;
        float wtop = viewFrame.origin.y;
        float wwidth = viewFrame.size.width;
        float wheight = viewFrame.size.height;
        NSLog( @"Dumping tree view for window 0x%08X [ %.1f, %.1f / %.1f, %.1f ]",
              [ view window ], wtop, wleft, wtop + wheight, wleft + wwidth);
    }
    
    NSString *str = [ NSString stringWithCString: sNesting length: (level * 2) ];
    NSLog( @"%@ 0x%08X %s [ %.1f, %.1f / %.1f, %.1f ", 
          str, view, viewClass, top, left, top + height, left + width, width, height );
    
    //	A different view: check subviews
    NSArray *subviews = [ view subviews ];
    NSEnumerator *viewNumerator = [ subviews objectEnumerator ];
    NSView *subview;
    while ( (subview = [ viewNumerator nextObject ]) != nil )
        DumpViewTree( subview, level + 1 );
}
#endif

#if DUMP_VIEWS
void DumpViewTree( NSWindow* window )
{
    NSView *contentView = [ window contentView ];
    DumpViewTree( contentView, 0 );
    NSLog( @"\n" );
}
#else
void DumpViewTree( NSWindow*);
void DumpViewTree( NSWindow*)
{
}
#endif


#if defined(DEBUG) && DEBUG
//	Class to keep track of the plugin superview calls for debugging purposes
@interface PluginSuperview : NSView {
}

- (void) setFrame: (NSRect) frameRect;

@end

@implementation PluginSuperview

- (void) setFrame: (NSRect) frameRect
{
    NSLog( @"PluginSuperview(0x%08lX) setFrame(0x%08lX) %s", (size_t)self, (size_t)[super self], SprintfNSRect(&frameRect) );
    [ super setFrame: frameRect ];
}

@end
#else
#define PluginSuperview 	NSView
#endif

//@protocol WebPlugInViewFactory <NSObject>
//	+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments;
//@end

@protocol WebPlugin <NSObject>

- (void) webPlugInInitialize;
- (void) webPlugInStart;
- (void) webPlugInStop;
- (void) webPlugInDestroy;
- (void) webPlugInSetIsSelected: (BOOL) isSelected;
- (id) objectForWebScript;

@end

//	Forward declarations.
@class OApolloPDFPluginViewBridge;

//	This is the webFrame bridge for the plugin: it implements the necessary WebFrame
//	methods in order to support POSTs to this frame and others.
@interface OPluginPDFWebFrameBridge : NSObject<ApolloWebKitWebFramePluginHost> {
	OApolloPDFPluginViewBridge *	mMacBridge;
	WebCore::Frame *		mTargetFrame;
}

//	Initialization/destruction
- (OPluginPDFWebFrameBridge *) initWithMacBridge: (OApolloPDFPluginViewBridge *) macBridge andTargetFrame: (WebCore::Frame *) targetFrame;
- (void) dealloc;

//	WebFrame Routines needed by AdobePDFView
- (void) errorLoadingAcrobat: (SInt32) error;
- (void) loadRequest: (NSURLRequest *) request;
- (NSObject<ApolloWebKitWebFramePluginHost> *) findFrameNamed: (NSString *) name;
- (id) dataSource;
- (id) webView;

//	This is actuallya WebPlugInContainer routine, we put it here for convenience
//	and AdobePDFViewer gets the WebFrame from it: self.
- (NSObject<ApolloWebKitWebFramePluginHost> *) webFrame;

@end

//This is the bridge object betweeen the Apollo Web Kit PDF mac subclass
//and AdobePDFViewer, the Acrobat WebKit plugin.  This object acts as
//the WebKit Plugin Host.
@interface OApolloPDFPluginViewBridge : NSObject {
	NSView<WebPlugInViewFactory,WebPlugin>*		mPluginView;
    WebCore::ApolloPDFPluginView*				mPluginPDFImpl;
	NSMutableDictionary*						mPluginArguments;
	NSView*										mPluginSuperview;
    NSWindow*                                   mApolloWindow;
#if IMP_EXTERNAL_RESOURCE_LOADER
    NSMutableArray *							mCurrentPluginClients;
#endif
	BOOL										mStarted;
	BOOL										mVisible;
    BOOL                                        mEnabled;
    BOOL                                        mPluginVisible;
    BOOL                                        mApolloSuperViewUsingOpenGL;
}

+ (NSBundle *) adobePDFViewerBundleForPDFImpl: (WebCore::ApolloPDFPluginView *) pluginPDFImpl;
- (OApolloPDFPluginViewBridge *) initWithPluginPDFImpl: (WebCore::ApolloPDFPluginView *) pluginPDFImpl;
- (NSView<WebPlugInViewFactory,WebPlugin>*) pluginView;
- (NSURL *) URLByExpandingCustomSchemesFor: (NSURL *) origURL;
- (NSURL *) URLByContractingCustomSchemesFor: (NSURL *) origURL;
- (void) start;
- (bool) addToApolloWindow;
- (void) stop;
- (void) setFrameToRect: (const WebCore::IntRect&) rect clippingBy: (const WebCore::IntRect&) clipRect;
- (void) setNeedsDisplay: (const WebCore::IntRect&) windowDirtyRect;
- (void) setVisibility: (BOOL) visibility;
- (void) setEnabled: (BOOL) enabled;
- (NSRect) pluginRectFromApolloWindowRect: (const WebCore::IntRect&) windowRect;
- (WebCore::ApolloPDFPluginView*) pluginPDFImpl;
- (void) dealloc;
@end

//	Convenience function for string conversions.
static NSString *NSStringFromWebCoreString( const WebCore::String& wcString )
{
	return [ NSString stringWithCString: wcString.utf8().data() encoding: NSUTF8StringEncoding ];
}

//Copied from WebKit KURLMac.mm and KURLCFNet.cpp
NSURL* NSURLFromKURL( const WebCore::KURL& kurl );
NSURL* NSURLFromKURL( const WebCore::KURL& kurl )
{
    WTF::CString str = kurl.string().latin1();
    const UInt8 *bytes = (const UInt8 *)str.data();
    // NOTE: We use UTF-8 here since this encoding is used when computing strings when returning URL components
    // (e.g calls to NSURL -path). However, this function is not tolerant of illegal UTF-8 sequences, which
    // could either be a malformed string or bytes in a different encoding, like Shift-JIS, so we fall back
    // onto using ISO Latin-1 in those cases.
    CFURLRef result = CFURLCreateAbsoluteURLWithBytes(0, bytes, str.length(), kCFStringEncodingUTF8, 0, true);
    if (!result)
        result = CFURLCreateAbsoluteURLWithBytes(0, bytes, str.length(), kCFStringEncodingISOLatin1, 0, true);
    return [ (NSURL *) result autorelease ];
}

@implementation OApolloPDFPluginViewBridge

#if DEBUG_RETAIN_RELEASE
- (void) retain
{
    [ super retain ];
}

- (void) release
{
    [ super release ];
}
#endif

//	This returns TRUE if the plugin has been loaded and responds to the correct method.
//	Until it's loaded successfully the first time, it will attempt to laod it every
//	time a PDF widget is created, so the user doesn't have to Quit Apollo if they
//	have to change something in Acrobat to make it work.
+ (NSBundle *) adobePDFViewerBundleForPDFImpl: (WebCore::ApolloPDFPluginView *) pluginPDFImpl
{
	static NSBundle *sAdobePDFViewerBundle = nil;
	if ( sAdobePDFViewerBundle == nil )
	{
        WebHost* pWebHost = pluginPDFImpl->webHost();
        int pdfStatus = pWebHost->m_pVTable->loadPDFPlugin( pWebHost, reinterpret_cast<void **>(&sAdobePDFViewerBundle) );
		if ( pdfStatus != 0 )
		{
            pWebHost->m_pVTable->handlePDFError( pWebHost, pdfStatus );
		}
	}
    
	return sAdobePDFViewerBundle;
}

typedef enum {
	kPDFImplMacSchemeNotAppPath,
	kPDFImplMacSchemeAppResourcePath,
	kPDFImplMacSchemeAppStoragePath
} PDFImplMacSchemeType;

- (const uint16_t *) pathForSchemeType: (PDFImplMacSchemeType) schemeType returningLength: (unsigned long *) pathLength
{
	//	If this is an app or app-storageURL, we need to map it to the 
	//	real file URL.  We get UTF16 characters back.
	//	We only expand if it's app or app-storage and there is a path.
	*pathLength = 0;
	switch ( schemeType )
	{
		case kPDFImplMacSchemeAppResourcePath:
        {
            WebHost* pWebHost = mPluginPDFImpl->webHost();
			return pWebHost->m_pVTable->getAppResourcePath( pWebHost, pathLength );
        }
		case kPDFImplMacSchemeAppStoragePath:
        {
            WebHost* pWebHost = mPluginPDFImpl->webHost();
			return pWebHost->m_pVTable->getAppStoragePath( pWebHost, pathLength );
        }
		default:
			return NULL;
	}
}

static PDFImplMacSchemeType URLSchemeTypeAndPath( NSURL *appURL, NSString **appPath )
{
	PDFImplMacSchemeType schemeType = kPDFImplMacSchemeNotAppPath;
	*appPath = nil;
    
	NSString *URLScheme = [ [ appURL scheme ] lowercaseString ];
	if ( [ URLScheme isEqualToString: @"app" ] )
		schemeType = kPDFImplMacSchemeAppResourcePath;
	else if ( [ URLScheme isEqualToString: @"app-storage" ] )
		schemeType = kPDFImplMacSchemeAppStoragePath;
	else
		return schemeType;
    
	if ( [ [ appURL absoluteString ] length ] > ( [ URLScheme length ] + 2 ) )
	{
		NSString *urlPath = [ [ appURL absoluteString ] substringFromIndex: [ URLScheme length ] + 1 ];
		if ( [ urlPath hasPrefix: @"/" ] && ([ urlPath length ] > 1) )
		{
			urlPath = [ urlPath substringFromIndex: 1 ];
			if ( ![ urlPath hasPrefix: @"/" ] )
			{
				*appPath = urlPath;
				return schemeType;
			}
			else
			{
#if !defined(NDEBUG) || !NDEBUG
                NSLog( @"URL of %@, too many slashes, not mapping", [ appURL absoluteString ] );
#endif
			}
		}
		else
		{
#if !defined(NDEBUG) || !NDEBUG
            NSLog( @"URL of %@, no slashes, not mapping", [ appURL absoluteString ] );
#endif
		}
	}
	
	//	Didn't work out.
	return kPDFImplMacSchemeNotAppPath;
}


//	Starting with an app or app-storage URL, map it to a file URL.
//	We assume the original URL is percent-encoded with UTF8 encoding.
- (NSURL *) URLByExpandingCustomSchemesFor: (NSURL *) origURL
{
	NSString *urlPath = nil;
	PDFImplMacSchemeType schemeType = URLSchemeTypeAndPath( origURL, &urlPath );
	if ( schemeType == kPDFImplMacSchemeAppResourcePath || schemeType == kPDFImplMacSchemeAppStoragePath )
	{
		//	appPath returned is a singleton so we don't own it.
		unsigned long appPathLength = 0;
		uint16_t *appPathUTF16 = const_cast<uint16_t *>( [ self pathForSchemeType: schemeType returningLength: &appPathLength ] );
		if ( appPathUTF16 != NULL )
		{
			NSString *mappedFilePath = [ NSString stringWithCharacters: reinterpret_cast<const unichar *>(appPathUTF16) length: appPathLength ];
            
			if ( mappedFilePath != nil )
			{
				NSString *fullPath = [ NSString stringWithFormat: @"%@/%@", mappedFilePath, urlPath ];
				return [ NSURL fileURLWithPath: fullPath ];
			}
			else
			{
#if !defined(NDEBUG) || !NDEBUG
                NSLog( @"URL of %@, got nil mapped file path", [ origURL absoluteString ] );
#endif
			}
		}
		else
		{
#if !defined(NDEBUG) || !NDEBUG
            NSLog( @"URL of %@, got nil app path", [ origURL absoluteString ] );
#endif
		}
	}
    
	return origURL;
}

- (NSURL *) URLByContractingCustomSchemesFor: (NSURL *) origURL
{
	NSString *urlPath = nil;
	PDFImplMacSchemeType schemeType = URLSchemeTypeAndPath( origURL, &urlPath );
	switch ( schemeType )
	{
		case kPDFImplMacSchemeAppResourcePath:
			return [ NSURL URLWithString: [ NSString stringWithFormat: @"app:/%@", urlPath ] ];
            break;
            
		case kPDFImplMacSchemeAppStoragePath:
			return [ NSURL URLWithString: [ NSString stringWithFormat: @"app-storage:/%@", urlPath ] ];
            break;
            
		default:
			return origURL;
	}
	
}

- (OApolloPDFPluginViewBridge *) initWithPluginPDFImpl: (WebCore::ApolloPDFPluginView *) pluginPDFImpl
{
	self = [ super init ];
	if ( self != nil )
	{
		mPluginView = nil;
		mPluginPDFImpl = pluginPDFImpl;
		mPluginArguments = nil;
		mPluginSuperview = nil;
        mApolloWindow = nil;
		mStarted = NO;
#if IMP_EXTERNAL_RESOURCE_LOADER
        mCurrentPluginClients = nil;
#endif
		mVisible = NO;
        mPluginVisible = NO;
        mEnabled = NO;
        
        mApolloSuperViewUsingOpenGL = NO;
        
		NSBundle *pluginBundle = [ OApolloPDFPluginViewBridge adobePDFViewerBundleForPDFImpl: pluginPDFImpl ];
		if ( pluginBundle != nil )
		{
			Class viewFactory = [ pluginBundle principalClass ];
            
			//"arguments" argument, dictionary of attributes.  AdobePDFViewer is expecting from WebKit
			//   Key                         value                 Use (* == must exist)
			// "WebPlugInContainerKey"      WebFrame           getting datasource (avoiding double-loading)
			//                                                 loading non-PDF weblinks
			//
			// "WebPlugInBaseURLKey"        NSString           base URL if URL below is relative
			//                                            
			// "WebPlugInAttributesKey"     NSDictionary       object/embed parameters
			//                     "type"            NSString         mimeType
			//                    *"src" or "data"   NSString         URL source
            
			//AdobePDFViewer is expecting from ApolloWebKit (or non-normal WebKit)
			// "AcrobatAttributesKey"     NSDictionary       acrobat-specific parameters
			//                     "Environment"     NSString         kind of webkit ("ApolloWebKit")
			//                     "ResourceLoader"  id               class to use instead of NSURLConnection
			//                     "WebFrame"        id<WebFrameCalls>    class to use instead of webFrame
			//	... this is passed into Acrobat as a CFDictionary, so Apollo can send things through AdobePDFViewer
			
			mPluginArguments = [ [ NSMutableDictionary dictionaryWithCapacity: 3 ] retain ];
			NSMutableDictionary *attributes = [ NSMutableDictionary dictionaryWithCapacity: 2 ];
            
			NSURL *url = NSURLFromKURL( mPluginPDFImpl->getUrl() );
			NSURL *mappedURL = [ self URLByExpandingCustomSchemesFor: url ];
			if ( mappedURL == nil )	//paranoia
				mappedURL = [ NSURL URLWithString: @"about:blank" ];
			[ attributes setObject: [ mappedURL absoluteString ] forKey:  @"src" ];
            
            WebCore::String mimeType = mPluginPDFImpl->getMimeType(); 
            if(mimeType.isEmpty())
                mimeType = "application/pdf";
			[ attributes setObject: [ NSString stringWithCString: mimeType.utf8().data() encoding: NSUTF8StringEncoding ] forKey:  @"type" ];
			[ mPluginArguments setObject: attributes forKey: @"WebPlugInAttributesKey" ];
			
			//Special argument for AdobePDFViewer so it knows it's running in ApolloWebKit
			//this is not in System WebKit.
			NSMutableDictionary *acroAttributes = [ NSMutableDictionary dictionaryWithCapacity: 2 ];
			[ acroAttributes setObject: @"ApolloWebKit" forKey: @"Environment" ];
			[ acroAttributes setObject: self forKey: @"ApolloWebKitPDFPluginHost" ];
			
			//	Get the URL of the container (the frame) and if it's different, send it in.
			WTF::RefPtr<WebCore::Frame> frame = pluginPDFImpl->frame();
			NSURL *frameURL = NSURLFromKURL( frame->loader()->url() );
			
			//	If the frame has a different URL, then it's our container
			NSURL *mappedFrameURL = [ self URLByExpandingCustomSchemesFor: frameURL ];
			if ( ! [ [ [ mappedFrameURL absoluteString ] lowercaseString ] isEqualToString: [ [ mappedURL absoluteString ] lowercaseString ] ] )
				[ acroAttributes setObject: mappedFrameURL forKey: @"HostContainer" ];
            
			//	AdobePDFViewer has to know what kind of request was made for this response, since it
			//	will have to duplicate the request.  It does this through a WebDataSource object and
			//	gets a request object to see what it is.  Create one from the one we're given; if
			//	we're just doing a GET, we don't have to make this, only if it's a POST.
			//	We assume that no POST will be app or app-storage so we don't use mappedURLs.
			const WebCore::ResourceRequest& resourceRequest = frame->loader()->originalRequest();
			if ( resourceRequest.httpMethod() == "POST" )
			{
				NSMutableURLRequest *nsRequest = [ NSMutableURLRequest requestWithURL: NSURLFromKURL( resourceRequest.url() ) ];
				[ nsRequest setHTTPMethod: @"POST" ];
				Vector<char> postData;
				resourceRequest.httpBody()->flatten(postData);
				if ( postData.size() > 0 )
				{
					NSData *nsData = [ NSData dataWithBytes: postData.data() length: postData.size() ];
					[ nsRequest setHTTPBody: nsData ];
				}
				
				WebCore::HTTPHeaderMap headerMap = resourceRequest.httpHeaderFields();
				unsigned long numHeaders = headerMap.size();
				NSMutableDictionary *hdrDict = [ NSMutableDictionary dictionaryWithCapacity: numHeaders ];
                
				for(WebCore::HTTPHeaderMap::iterator i = headerMap.begin(); i != headerMap.end(); ++i)
				{
					const WebCore::String name = i->first;
					const WebCore::String value = i->second;
					if ( name.length() > 0 && value.length() > 0 )
						[ hdrDict setValue: NSStringFromWebCoreString( value ) forKey: NSStringFromWebCoreString( name ) ];
				}
				
				[ nsRequest setAllHTTPHeaderFields: hdrDict ];
				[ acroAttributes setObject: nsRequest forKey: @"HostRequest" ];
			}
            
			//	We have a slightly different class for javascript undefined objects (it's actually
			//	exactly the same code but the obj-c class is renamed to not conflict with the
			//	system Webkit used by Acrobat... but when it returns the 'undefined' value it
			//	has to be an exact match with ours, so we pass it in).
			
			[ acroAttributes setObject: [ WebUndefined undefined ] forKey: @"WebUndefined" ];
            
			//	This concludes the arguments inside AcrobatAttributes
			[ mPluginArguments setObject: acroAttributes forKey: @"AcrobatAttributesKey" ];
            
			//	For POSTing, we need a webframe host that supports some WebFrame methods.
			OPluginPDFWebFrameBridge *webFrameBridge = [ [ [ OPluginPDFWebFrameBridge alloc ] initWithMacBridge: self
                                                                                                 andTargetFrame: frame.get() ]
                                                        autorelease ];
			[ mPluginArguments setObject: webFrameBridge forKey: @"WebPlugInContainerKey" ];
			
			{
				JSC::JSLock::DropAllLocks dropAllLocks(false);
				mPluginView = (NSView<WebPlugInViewFactory,WebPlugin> *)[ [ viewFactory plugInViewWithArguments: mPluginArguments ] retain ];
				[ mPluginView webPlugInInitialize ];
			}
		}
	}
	
	return self;
}

- (NSView<WebPlugInViewFactory,WebPlugin> *) pluginView
{
	return mPluginView;
}

- (void) start
{
}

- (bool) addToApolloWindow
{
    if(!mPluginPDFImpl->parent())
        return false;
    
    NSWindow* window = mPluginPDFImpl->parent()->hostWindow()->platformPageClient();
    
    if(mApolloWindow == window)
        return false;
        
    if([window isVisible] == NO)
        return false;
        
    if(mApolloWindow != nil && mPluginSuperview != nil)
    {
        //remove from old window
        if([mPluginSuperview superview]!=nil)
            [mPluginSuperview removeFromSuperview];
    }
    
    if(window != nil)
    {
        NSView *contentView = [ window contentView ];
        DumpViewTree( window );
        

        // Determine whether the Apollo content view is attached to an OpenGL context
        SEL usingOpenGLSelector = NSSelectorFromString(@"getOpenGLMode");
        if([ contentView respondsToSelector:usingOpenGLSelector ]) {
            // A lot of rigmarole just to dynamically call a selector that returns a
            // native type instead of an object (id)
            NSInvocation *invocation = [ NSInvocation invocationWithMethodSignature:
                                        [ [contentView class] instanceMethodSignatureForSelector:usingOpenGLSelector] ];
            [ invocation setSelector:usingOpenGLSelector ];
            [ invocation setTarget:contentView ] ;
            [ invocation invoke ];
            [ invocation getReturnValue:&mApolloSuperViewUsingOpenGL ];
        }
        
        
        // This check is needed for the case where the PDF view is being moved
        // to a new window (as fullscreen uses a different window).
        if (mPluginSuperview == nil)
        {
            //	We set the plug-in view's size manually; the superview is used to clip
            //	the plug-in view within the HTML content, so it shouldn't auto-resize
            //	its subview.
            mPluginSuperview = [ [ PluginSuperview alloc ] init ];
            [ mPluginSuperview setAutoresizesSubviews: NO ];
            [ mPluginSuperview setAutoresizingMask: NSViewMaxXMargin|NSViewMinYMargin ];
        }
        [ contentView addSubview: mPluginSuperview ];
    }
    
    mApolloWindow = window;
    
    return true;
}

- (void) stop
{
	//	We are *completely* done.  Make sure we're gone from everywhere
	[ self setEnabled: NO ];
	{
		JSC::JSLock::DropAllLocks dropAllLocks(false);
		[ mPluginView webPlugInDestroy ];
	}
	[ mPluginView release ];
	mPluginView = nil;
    
	[ mPluginSuperview release ];
	mPluginSuperview = nil;
    
#if IMP_EXTERNAL_RESOURCE_LOADER
    [ mCurrentPluginClients release ];
    mCurrentPluginClients = nil;
#endif
    
	[ mPluginArguments release ];
	mPluginArguments = nil;
}

//	We have to translate the frame to Cocoa coordinates, which is a pain:
//	 * adl uses Carbon coordinates, which is (0,0) at the top-left of the main screen and y goes down.
//	 * Cocoa coordinates are (0,0) at the bottom-left of the main screen and y goes up.
//	For frames in NSViews, the coords are relative to the superview's bounds, so the location
//	and height of the main screen is irrelevant. (for any NSView, 'frame' is in the coords
//	of the superview, and 'bounds' is its own coord system that subviews' frames are in. By
//	setting an NSView's frame, you are placing it in its superview; by setting an NSView's
//	bounds, you are moving its subviews around).
//	The incoming rect is relative to the window; this may be harder to figure out?
//  (Appended comment by rosantos 2010/11/09) APE's NSView uses flipped coordinates so no
//  translation is needed.
- (NSRect) pluginRectFromApolloWindowRect: (const WebCore::IntRect&) windowRect
{
		
	NSRect newFrame;
	newFrame.origin.x = windowRect.x();
	newFrame.size.width = windowRect.width();
	newFrame.size.height = windowRect.height();

	if ( NO == [ [ mPluginSuperview superview ] isFlipped ] )
		newFrame.origin.y = [ [ mPluginSuperview superview ] bounds ].size.height - (windowRect.y() + windowRect.height());
	else
		newFrame.origin.y = windowRect.y();

	return newFrame;
}

- (void) setFrameToRect: (const WebCore::IntRect&) rect clippingBy: (const WebCore::IntRect&) clipRect
{
    WebCore::IntRect newClipRect = rect;
    newClipRect.intersect(clipRect);
    
	//	We use the superView to do the clipping
	NSRect newClip = [ self pluginRectFromApolloWindowRect: newClipRect ];
    
#if LOG_PDFIMPL_CALLS
    NSLog( @"pluginMacBridge(0x%08X) setFrame(0x%08X) %s", self, mPluginSuperview, SprintfNSRect(&newClip) );
#endif
	[ mPluginSuperview setFrame: newClip ];
    
    //	The routine returns the value according to the mPluginSuperView frame, not bounds,
	//	so when we set the pluginView's frame we want to account for the offset of the
	//	superview's bounds.
	NSRect newFrame = [ self pluginRectFromApolloWindowRect: rect ];
	newFrame = NSOffsetRect( newFrame, -newClip.origin.x, -newClip.origin.y );
    
#if LOG_PDFIMPL_CALLS
    NSLog( @"pluginMacBridge(0x%08X) setFrame(0x%08X) %s", self, mPluginView, SprintfNSRect(&newFrame) );
#endif
    
	[ mPluginView setFrame: newFrame ];
}

- (void) setNeedsDisplay: (const WebCore::IntRect&) windowDirtyRect
{
	NSRect pluginDirtyRect = [ self pluginRectFromApolloWindowRect: windowDirtyRect ];
	[ mPluginView setNeedsDisplayInRect: pluginDirtyRect ];
}

- (void) setVisibility: (BOOL) visibility
{
    mVisible = visibility;
}

- (void) setEnabled: (BOOL)enabled
{
    mEnabled = enabled;
    
    if(!mPluginView)
        return;

    if(mEnabled)
        [self addToApolloWindow];
    
    const bool showPlugin = (mEnabled && mVisible);
    
	if ( showPlugin != mPluginVisible )
	{
		//	We may never have been able to load it.
		if ( mPluginSuperview != nil )
		{
			if ( showPlugin && mApolloWindow != nil)
			{                
				if ( [ mPluginView superview ] == nil )
				{
                    [mPluginSuperview setHidden: NO];
                    
                    if ( mApolloSuperViewUsingOpenGL ) {
                        // A bit of hack in order to get the PDF plug-in to display at all
                        // when the Apollo contentView is attached to an OpenGL context
                        [ mPluginView setWantsLayer:YES ];
                    }

					[ mPluginSuperview addSubview: mPluginView ];

					{
						JSC::JSLock::DropAllLocks dropAllLocks(false);
						[ mPluginView webPlugInStart ];
					}

                    mPluginVisible = showPlugin;
				}
			}
			else
			{
				if( [ mPluginView superview ] != nil )
				{
					{
						JSC::JSLock::DropAllLocks dropAllLocks(false);
						[ mPluginView webPlugInStop ];
					}
					
					[ mPluginView removeFromSuperview ];
                    mPluginVisible = showPlugin;
                    
                    [mPluginSuperview setHidden: YES];
                    
                    if ( mApolloSuperViewUsingOpenGL ) {
                        // A bit of hack in order to get the PDF plug-in to display at all
                        // when the Apollo contentView is attached to an OpenGL context
                        [ mPluginView setWantsLayer:NO ];
                    }
				}
			}
		}
	}
}

- (WebCore::ApolloPDFPluginView*) pluginPDFImpl
{
	return mPluginPDFImpl;
}

- (void) dealloc
{
	[ self stop ];
	[ super dealloc ];
}

@end

@implementation OPluginPDFWebFrameBridge

- (OPluginPDFWebFrameBridge *) initWithMacBridge: (OApolloPDFPluginViewBridge *) macBridge andTargetFrame: (WebCore::Frame *) targetFrame
{
	self = [ super init ];
	if ( self != nil )
	{
		mMacBridge = [ macBridge retain ];
		mTargetFrame = targetFrame;
	}
	return self;
}


- (void) errorLoadingAcrobat: (SInt32) error
{
#if LOG_PDFIMPL_CALLS
    NSLog (@"OApolloPDFPluginViewBridge(%08x) : errorLoadingAcrobat %d", self, error );
#else
    //keep the comp happy
    (void)error;
#endif
}

- (void) loadRequest: (NSURLRequest *) request
{
#if LOG_PDFIMPL_CALLS
    NSLog (@"OApolloPDFPluginViewBridge(%08x) : loadRequest %@", self, [ request URL ] );
#endif
    
	//	We passed Acrobat a mapped URL (app or app-storage) so we need
	//	to do the same mapping in reverse.
	WebCore::KURL urlRequested( [ mMacBridge URLByContractingCustomSchemesFor: [ request URL ] ] );
	WebCore::KURL urlSource = [ mMacBridge pluginPDFImpl ]->getUrl();
	if ( [ mMacBridge pluginPDFImpl ]->allowURLRequest( urlSource, urlRequested, mTargetFrame ))
	{
		{
			if ( mTargetFrame != NULL )
				[ mMacBridge pluginPDFImpl ]->wrapperLoadNSURLRequestInFrame( request, mTargetFrame );
			else
			{
				//	TBD: Find out if we can open windows and do so; otherwise load it here.
				[ mMacBridge pluginPDFImpl ]->wrapperLoadNSURLRequestInFrame( request, NULL );
                
#if 0	/* Security hole: don't launch Safari */
                FSRef safariRef;
                NSURL *safariURL;
                if ( noErr == LSGetApplicationForURL( (CFURLRef) [ NSURL URLWithString: @"http://www.adobe.com" ], kLSRolesAll, &safariRef, (CFURLRef *) &safariURL ) )
                {
                    LSLaunchURLSpec launchURLSpec;
                    BlockZero(&launchURLSpec, sizeof(launchURLSpec));
                    launchURLSpec.appURL = (CFURLRef) safariURL;	// LSFindApplicationForInfo retains safariUrl, so we release it below
                    CFURLRef destURL = (CFURLRef) [ request URL ];
                    launchURLSpec.itemURLs = CFArrayCreate(kCFAllocatorDefault, (const void **) &destURL, 1, &kCFTypeArrayCallBacks);
                    (void) LSOpenFromURLSpec( &launchURLSpec, NULL );
                    CFRelease( safariURL );
                }
#endif
			}
		}
	}
}

- (NSObject<ApolloWebKitWebFramePluginHost> *) findFrameNamed: (NSString *) name
{
#if LOG_PDFIMPL_CALLS
    NSLog (@"OApolloPDFPluginViewBridge(%08x) : findFrameNamed %@", self, name );
#endif
	
	const char *utf8Name = [ name UTF8String ];
	ASSERT(utf8Name);
	if ( utf8Name == NULL )
		return nil;		// badly-formed frame name
    
	//	'self' is still attached to the macbridge for this plugin, but the
	//	target frame needs to be different.  We will get a loadRequest later
	//	and we want to make sure it goes to the right frame.
	WebCore::Frame *foundFrame = mTargetFrame->tree()->find( utf8Name );
	if ( foundFrame == mTargetFrame )
		return self;
	else if (	foundFrame == NULL
             &&	![ name isEqualToString: @"_blank" ]
             &&	![ name isEqualToString: @"_new" ] )
	{
		//	Nope, not a reserved name and we can't find it.  Drop it on the floor.
		return nil;
	}
	else
	{
		//	AdobePDFViewer does not expect to own this and will retain it if it wants.
		//	If the frame is nil, we'll know to open the default window.  Note that will
		//	only work for GETs.
		OPluginPDFWebFrameBridge *webFrameBridge = [ [ [ OPluginPDFWebFrameBridge alloc ]
                                                      initWithMacBridge: mMacBridge
                                                      andTargetFrame: foundFrame ]
                                                    autorelease ];
		return webFrameBridge;
	}
}

- (id) dataSource
{
	return nil;
}

- (id) webView
{
	return nil;
}


//	This is called by AdobePDFViewer to get the webFrame it's in because we are
//	passed ins the WebPluginContainerKey... so we are doing double duty as both
//	the "WebPluginContainer" and the "WebFrame".  
- (NSObject<ApolloWebKitWebFramePluginHost> *) webFrame
{
	return self;
}

- (void) dealloc
{
	[ mMacBridge release ];
	mTargetFrame = nil;
	[ super dealloc ];
}



@end

#if IMP_EXTERNAL_RESOURCE_LOADER

//	AdobePDFViewer calls us with these when it's ready to load, and when
//	Acrobat needs to communicate.
@interface OApolloPDFPluginViewBridge (ApolloWebKitPDFPluginHost)

- (void) startLoadingURL: (NSURL *) URL
               forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client;
- (void) startLoadingURL: (NSURL *) URL
               forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client
              withMethod: (NSString *) method
                 andData: (NSData *) data;
- (void) stopLoadingURL: (NSURL *) URL
              forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client;

@end

@interface OApolloPDFPluginViewBridge (ResourceLoading)
/*
 - (void) client: (id) client receivedRedirect: (NSURL *) URL;
 - (void) client: (id) client receivedResponse: (NSURLResponse *) response;
 - (void) client: (id) client receivedData: (const char *) data;
 - (void) clientReceivedAllData: (id) client;
 */

- (void) client: (void *) client receivedRedirect: (const WebCore::KURL&) URL;
- (void) client: (void *) client receivedResponse: (WebCore::PlatformResponse) response fromLoader: (WebCore::ResourceLoader *) loader;
- (void) client: (void *) client receivedData: (const char*) data length: (int) length;
- (void) clientReceivedAllData: (void *) client;

@end

@implementation OApolloPDFPluginViewBridge (ApolloWebKitPDFPluginHost)

- (void) startLoadingURL: (NSURL *) URL
               forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client
{
    [ self startLoadingURL: URL forClient: client withMethod: @"GET" andData: nil ];
}


- (void) startLoadingURL: (NSURL *) URL
               forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client
              withMethod: (NSString *) method
                 andData: (NSData *) data
{
    if ( mCurrentPluginClients == nil )
        mCurrentPluginClients = [ [ NSMutableArray alloc ] initWithCapacity: 5 ];
    [ mCurrentPluginClients addObject: client ];
    WebCore::KURL url( URL );
    mPluginPDFImpl->pluginLoadURLRequest( url, static_cast<void *>(client) );
}


- (void) stopLoadingURL: (NSURL *) URL
              forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client
{
    //	We may get things out-of-order; make sure we still have this guy.
    if ( [ mCurrentPluginClients containsObject: (id) client ] )
        [ mCurrentPluginClients removeObject: (id) client ];
}

@end

@implementation OApolloPDFPluginViewBridge (ResourceLoading)

//	We received a redirect: tell the client about that.  We don't have any way
//	of preventing this; presumably the Apollo environment has handled security.
- (void) client: (void *) client receivedRedirect: (const WebCore::KURL&) URL
{
    if ( [ mCurrentPluginClients containsObject: (id) client ] )
    {
        if ( [ (id) client respondsToSelector: @selector(receivedRedirect:) ] )
        {
            [ (id<ApolloWebKitPDFPluginURLLoadingClient>) client receivedRedirect: NSURLFromKURL( URL ) ];
        }
    }
}

//	We received a response; tell the client about it.  There is no object that
//	encapsulates headers and status and the request (NSMutableURLResponse would
//	be great but it doesn't exist),
- (void) client: (void *) client receivedResponse: (WebCore::PlatformResponse) response fromLoader: (WebCore::ResourceLoader *) loader
{
    if ( [ mCurrentPluginClients containsObject: (id) client ] )
    {
        if ( [ (id) client respondsToSelector: @selector(receivedResponseOfExpectedLength:withHeaders:andStatusCode:andMIMEType:) ] )
        {
            SInt64 contentLength = mPluginPDFImpl->resourceLoaderContentLength( loader );
            NSMutableDictionary *headers = [ NSMutableDictionary dictionaryWithCapacity: 10 ];
            [ headers setObject: [ NSNumber numberWithInt: contentLength ] forKey: @"Content-Length" ];
            WebCore::String contentType = loader->queryMetaData("HTTP_CONTENT-TYPE");
            NSString *nsContentType = [ NSStringFromWebCoreString( contentType ) stringByTrimmingCharactersInSet: [ NSCharacterSet whitespaceCharacterSet ] ];
            if ( nsContentType != NULL )
                [ headers setObject: nsContentType forKey: @"Content-Type" ];
            [ (id<ApolloWebKitPDFPluginURLLoadingClient>) client receivedResponseOfExpectedLength: contentLength
                                                                                      withHeaders: headers andStatusCode: response->m_httpStatusCode
                                                                                      andMIMEType: nsContentType ];
        }
    }
}

- (void) client: (void *) client receivedData: (const char*) data length: (int) length
{
    if ( [ mCurrentPluginClients containsObject: (id) client ] )
    {
        if ( [ (id) client respondsToSelector: @selector(receivedData:) ] )
        {
            [ (id<ApolloWebKitPDFPluginURLLoadingClient>) client receivedData: [ NSData dataWithBytes: data length: length ] ];
        }
    }
}

- (void) clientReceivedAllData: (void *) client
{
    if ( [ mCurrentPluginClients containsObject: (id) client ] )
    {
        if ( [ (id) client respondsToSelector: @selector(receivedAllData) ] )
        {
            [ (id<ApolloWebKitPDFPluginURLLoadingClient>) client receivedAllData ];
        }
        
        [ mCurrentPluginClients removeObject: (id) client ];
    }
}

@end

#endif


//Mac implementation of PluginPDFImpl
namespace WebCore
{

	class ApolloPDFStopCallback {
	public:
		static ApolloPDFStopCallback* create(OApolloPDFPluginViewBridge* objCBridge)
		{
			return new ApolloPDFStopCallback(objCBridge);
		}
		
	private:
		void onTimerFired(WebCore::Timer<ApolloPDFStopCallback>*);
		
		ApolloPDFStopCallback(OApolloPDFPluginViewBridge* objCBridge);
				
		OApolloPDFPluginViewBridge* m_objCBridge;
		WebCore::Timer<ApolloPDFStopCallback> m_stopPDF;
	};
	
	ApolloPDFStopCallback::ApolloPDFStopCallback(OApolloPDFPluginViewBridge* objCBridge)
		: m_objCBridge(objCBridge)
		, m_stopPDF(this, &ApolloPDFStopCallback::onTimerFired)
	{
		m_stopPDF.startOneShot(0);
	}
	
	void ApolloPDFStopCallback::onTimerFired(WebCore::Timer<ApolloPDFStopCallback>*)
	{
		//	Even though 'dealloc' calls 'stop', there is a circular retain
		//	reference on it so we have to stop it to get everything else
		//	to release the mObjCBridge first.
		[ m_objCBridge stop ];
		[ m_objCBridge release ];

		delete this;
	}

	ApolloPDFPluginView::ApolloPDFPluginView(WebCore::Frame* frame, WebCore::Element* const /*element*/,
                                       const WebCore::KURL& url,
                                       const Vector<WebCore::String>& paramNames, 
                                       const Vector<WebCore::String>& paramValues,
                                       const WebCore::String& /*mimeType*/,
                                       WebWindow* pWebWindow,
                                       WebHost* pWebHost)
    :	mObjCBridge( nil )
    ,	mEverGotNonzeroSize( false )
    ,   mFrame(frame)
    ,   mURL(url)    
    ,   mParamNames(paramNames)
    ,   mParamValues(paramValues)    
    ,   mpWebWindow(pWebWindow)
    ,   mpWebHost(pWebHost)
	{
        mFrame->loader()->client()->addPlugin(this);
        
		load();
	}

	ApolloPDFPluginView::~ApolloPDFPluginView()
	{
		mFrame->loader()->client()->removePlugin(this);
		
		ApolloPDFStopCallback::create((OApolloPDFPluginViewBridge *) mObjCBridge);
	}
    
    WTF::PassRefPtr<ApolloPDFPluginView> ApolloPDFPluginView::create(Frame* parentFrame, 
                                       const IntSize&, Element* element, 
                                       const KURL& url, 
                                       const Vector<String>& paramNames, 
                                       const Vector<String>& paramValues, 
                                       const String& mimeType, 
                                       bool /*loadManually*/,
                                       WebWindow* pWebWindow,
                                       WebHost* pWebHost
    )
    {
        return adoptRef(new ApolloPDFPluginView(parentFrame, element, url, paramNames, paramValues, mimeType, pWebWindow, pWebHost));
    }
    
    void ApolloPDFPluginView::invalidateRect(const IntRect&)
    {
    }
    
	//Load the plug-in: return true if it succeeded
	bool ApolloPDFPluginView::load()
	{
		//Load the plug-in with the objective-c thing.
		if ( mObjCBridge == nil )
			mObjCBridge = (void *) [ [ OApolloPDFPluginViewBridge alloc ] initWithPluginPDFImpl: this ];
		return ( [ (OApolloPDFPluginViewBridge *) mObjCBridge pluginView ] != nil );
	}
    
    void ApolloPDFPluginView::setFrameRect(const IntRect& newRect)
    {
        WebCore::IntRect oldRect = frameRect();
        Widget::setFrameRect(newRect);
        
        if(parent())
        {
            WebCore::IntRect windowRect( convertToApolloWindowRect( parent()->contentsToWindow(newRect) ) );
            WebCore::IntRect windowClipRect( parent()->contentsToWindow(newRect) );
            windowClipRect.intersect(parent()->windowClipRect(true));
            windowClipRect = convertToApolloWindowRect( windowClipRect );
            
            frameGeometryHasChanged(oldRect, windowRect, windowClipRect);
        }
    }
	
	void ApolloPDFPluginView::frameGeometryHasChanged( const WebCore::IntRect &/*oldRect*/, const WebCore::IntRect& windowRect, const WebCore::IntRect& windowClipRect )
	{
		WebCore::IntRect rect = frameRect();
        
#if LOG_PDFIMPL_CALLS
        NSLog( @"ApolloPDFPluginView::changeFrameGeometry [ %5d %5d ] [ %5d %5d ]", rect.x(), rect.y(), rect.width(), rect.height() );
#endif
		
		//	If this is the first nonzero size we get, attach the view to the superview,
		//	resize the view and start it.
		if ( !mEverGotNonzeroSize && !rect.isEmpty() )
		{
			mEverGotNonzeroSize = true;
			[ (OApolloPDFPluginViewBridge *) mObjCBridge start ];
		}
		
		//	We don't tell the plugin to start until we get a nonzero size; once we get
		//	one, though, we tell it resize even if it's zero (so resizing to nonzero,
		//	then to zero, will work).
		if ( mEverGotNonzeroSize || !windowRect.isEmpty() )
			[ (OApolloPDFPluginViewBridge *) mObjCBridge setFrameToRect: windowRect clippingBy: windowClipRect ];
	}
    
    WTF::PassRefPtr<JSC::Bindings::Instance> ApolloPDFPluginView::bindingInstance()
    {
        id obj = nil;
        {
            JSC::JSLock::DropAllLocks dropAllLocks(false);
            obj = [ [ (OApolloPDFPluginViewBridge *) mObjCBridge pluginView ] objectForWebScript ];
        }
        WTF::RefPtr<JSC::Bindings::RootObject> root = mFrame->script()->createRootObject(this);
        WTF::RefPtr<JSC::Bindings::Instance> instance = JSC::Bindings::ObjcInstance::create(obj, root.release());
        return instance.release();
    }
    
	void ApolloPDFPluginView::paint( WebCore::GraphicsContext* /*pGraphicsContext*/, const WebCore::IntRect& dirtyRect )
	{
#if LOG_PDFIMPL_CALLS
        NSLog( @"ApolloPDFPluginView::paint [ %d %d ] [ %d %d ]", dirtyRect.x(), dirtyRect.y(), dirtyRect.width(), dirtyRect.height() );
#endif
		
		//Paint rect doesn't clip to our rect: do so now.
		WebCore::IntRect windowDirtyRect = dirtyRect;
		windowDirtyRect.intersect( frameRect() );
		
		//We do paint differently: we have to tell it to update, but not synchronously.
		windowDirtyRect.setLocation( parent()->contentsToWindow( windowDirtyRect.location() ) );
		
		[ (OApolloPDFPluginViewBridge *) mObjCBridge setNeedsDisplay: windowDirtyRect ];
	}
    
    void ApolloPDFPluginView::show()
    {
        // show doesn't actually enable the plug-in, only updateWindowedPlugins can do that
        [ (OApolloPDFPluginViewBridge *) mObjCBridge setVisibility: YES ];
    }
    
    void ApolloPDFPluginView::hide()
    {
        [ (OApolloPDFPluginViewBridge *) mObjCBridge setVisibility: NO ];
    }
	
    void ApolloPDFPluginView::updatePluginWindow(bool canShowPlugins, bool canShowWindowedPlugins) { 
        if(mObjCBridge == nil)
	        return;
        
        [ (OApolloPDFPluginViewBridge *) mObjCBridge setEnabled: canShowPlugins && canShowWindowedPlugins ];
        
        //	All events, including paint, may indicate that we have to
		//	move/resize.  This is because our drawing model (nested NSViews)
		//	doesn't match the Apollo drawing model (one NSView).  Setting
		//	us to the same geometry will make this call do the window-
		//	relative math and all the clipping stuff, and call back
		//	frameGeometryHasChanged if it really has.
		setFrameRect( frameRect() );
    }
    
	void ApolloPDFPluginView::loadNSURLRequestInFrame( void *request, WebCore::Frame *targetFrame )
	{
		//	void * so C++ classes that include this class don't have to know about NSURLRequest class.
		NSURLRequest *nsurlRequest = reinterpret_cast<NSURLRequest *>(request);
        
		//	Get the loadURL parameters from the obj-c request object
		WebCore::KURL url( [ nsurlRequest URL ] );
		const char *method = [ [ nsurlRequest HTTPMethod ] UTF8String ];
		const char *data = static_cast<const char *>([ [ nsurlRequest HTTPBody ] bytes ]);
		unsigned int dataLength = [ [ nsurlRequest HTTPBody ] length ];
		
		//	Make the headers into key:value pairs separated by "\r\n"
		WebCore::HTTPHeaderMap urlHeaders;
		NSDictionary *reqHeaders = [ nsurlRequest allHTTPHeaderFields ];
		NSEnumerator *keyEnum = [ reqHeaders keyEnumerator ];
		NSString *key;
		NSCharacterSet *illegalCharSet = [ NSCharacterSet characterSetWithCharactersInString: @"\r\n" ];
		while ( ( key = (NSString *) [ keyEnum nextObject ] ) != nil )
		{
			//	We will add the usual headers, only find the custom ones.
			if ( NSOrderedSame != [ key caseInsensitiveCompare: @"Content-length" ] )
			{
				NSString *value = [ reqHeaders valueForKey: key ];
				
				//	Security: no \r or \n in the key or the value.
				if (	( NSNotFound == [ key rangeOfCharacterFromSet: illegalCharSet ].location ) &&
                    ( NSNotFound == [ value rangeOfCharacterFromSet: illegalCharSet ].location ) )
					urlHeaders.add([ key UTF8String ], [ value UTF8String ]);
			}
		}
        
		//	Load it: this may go to another frame.  AdobePDFViewer plug-in will itself handle
		//	any loads that go back to it; our method gets called only when it's expecting
		//	html to come back so the frame itself should load it.
		loadURLForClient( url, method, data, dataLength, nil, urlHeaders, targetFrame );
	}
    
    struct PDFCallbackArguments {
        ApolloPDFPluginView* pluginView;
        void *request;
        WebCore::Frame *targetFrame;
    };
    
    void PDFCallback(void* arg);
    void PDFCallback(void* arg)
    {
        PDFCallbackArguments* pdfArgs = static_cast<PDFCallbackArguments*>(arg);
        pdfArgs->pluginView->loadNSURLRequestInFrame(pdfArgs->request, pdfArgs->targetFrame);
    }
    
    void ApolloPDFPluginView::wrapperLoadNSURLRequestInFrame( void *request, WebCore::Frame *targetFrame )
	{
        WebHost* webHost = FrameLoaderClientApollo::clientApollo(mFrame)->webHost();
        PDFCallbackArguments arg = {this, request, targetFrame};
        webHost->m_pVTable->enterPlayer(webHost, PDFCallback, &arg);
    }
    
    bool ApolloPDFPluginView::allowURLRequest( const WebCore::KURL sourceURL, const WebCore::KURL requestedURL, WebCore::Frame */*targetFrame*/ )
    {
        //    Use the frame's security.
        const WTF::RefPtr<WebCore::Frame>& webCoreFrame = frame();
        WebCore::FrameLoader *frameLoader = webCoreFrame->loader();
        WebCore::FrameLoaderClientApollo *frameLoaderClient = reinterpret_cast<WebCore::FrameLoaderClientApollo *>( frameLoader->client() );
        return frameLoaderClient->canLoad( requestedURL, sourceURL );
    }
    
    bool ApolloPDFPluginView::allowURLRequest( const WebCore::KURL requestedURL, WebCore::Frame *targetFrame )
    {
        return allowURLRequest( getUrl(), requestedURL, targetFrame );
    }
    
    void ApolloPDFPluginView::loadURLForClient( const WebCore::KURL& urlToLoad, void *client )
    {
        WebCore::HTTPHeaderMap headerMap;
        
        loadURLForClient( urlToLoad, "GET", NULL, 0, client, headerMap );
    }
    
    void ApolloPDFPluginView::loadURLForClient(  const WebCore::KURL& urlToLoad, const char *method,
                                      const char *data, unsigned int dataLength,
                                      void*,
                                      const WebCore::HTTPHeaderMap& headerMap,
                                      WebCore::Frame *frame )
    {
        //    In cerain error cases, including where there is no "data" property in
        //    the pdf OBJECT element, we're asked to load an empty URL.  Short-circuit
        //    that here.
        if ( urlToLoad.isEmpty() )
            return;
        
        WebCore::ResourceRequest resourceRequest( urlToLoad );
        
        // XXX - is replacement correct?
        WebCore::String sMethod(method);
        resourceRequest.setHTTPMethod( sMethod );
        //resourceRequest.setDoPost( method != NULL && 0 == strcmp( method, "POST" ) );
        
        //    Add the POST body if there is any
        ASSERT((!data && !dataLength) || (data && dataLength));
        
        if (data && dataLength)
        {
            // XXX - correct?
            resourceRequest.setHTTPBody(WebCore::FormData::create(data, dataLength));
            //resourceRequest.postData.appendData( data, dataLength );
        }
        
        // XXX - correct?
        resourceRequest.addHTTPHeaderFields( headerMap );
        resourceRequest.setHTTPUserAgent(  mFrame->loader()->userAgent(urlToLoad) );
        

        //    We can specifically state that the frame itself needs to be loading this instead
        //    of our plug-in. This happens in two cases: when a different frame is targeted,
        //    in which case frame != mWebFrameOwner->getFrame(), or when PDF has made a request
        //    that it believes will return non-PDF/FDF content.
        //mihnea integrate -> lockHistory = false
        if(!frame)
            frame = mFrame;
        
        frame->loader()->load( resourceRequest, false );
    }
    
    
#if IMP_EXTERNAL_RESOURCE_LOADER
    
    void ApolloPDFPluginView::pluginLoadURLRequest( WebCore::KURL& url, void *pluginClient )
    {
        loadURLForClient( url, pluginClient );
    }
    
    void ApolloPDFPluginView::clientReceivedRedirect(void *client, WebCore::ResourceLoader*, const WebCore::KURL& url)
    {
#if LOG_PDFIMPL_LOADER_CALLBACKS
        NSLog( @"ApolloPDFPluginView::clientReceivedRedirect" );
#endif
        [ (OApolloPDFPluginViewBridge *) mObjCBridge client: client receivedRedirect: url ];
    }
    
    void ApolloPDFPluginView::clientReceivedResponse(void *client, WebCore::ResourceLoader *resourceLoader, WebCore::PlatformResponse response)
    {
#if LOG_PDFIMPL_LOADER_CALLBACKS
        NSLog( @"ApolloPDFPluginView::clientReceivedResponse" );
#endif
        [ (OApolloPDFPluginViewBridge *) mObjCBridge client: client receivedResponse: response fromLoader: resourceLoader ];
    }
    
    void ApolloPDFPluginView::clientReceivedData(void *client, WebCore::ResourceLoader*, const char* data, int length)
    {
#if LOG_PDFIMPL_LOADER_CALLBACKS
        NSLog( @"ApolloPDFPluginView::clientReceivedData" );
#endif
        [ (OApolloPDFPluginViewBridge *) mObjCBridge client: client receivedData: data length: length ];
    }
    
    void ApolloPDFPluginView::clientReceivedAllData(void *client, WebCore::ResourceLoader*)
    {
#if LOG_PDFIMPL_LOADER_CALLBACKS
        NSLog( @"ApolloPDFPluginView::clientReceivedAllData 1" );
#endif
        [ (OApolloPDFPluginViewBridge *) mObjCBridge clientReceivedAllData: client ];
    }
    
    void ApolloPDFPluginView::clientReceivedAllData(void *client, WebCore::ResourceLoader*, WebCore::PlatformData)
    {
#if LOG_PDFIMPL_LOADER_CALLBACKS
        NSLog( @"ApolloPDFPluginView::clientReceivedAllData 2" );
#endif
        [ (OApolloPDFPluginViewBridge *) mObjCBridge clientReceivedAllData: client ];
    }
#endif
}


