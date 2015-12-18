#ifndef APOLLO_WEBKIT_PDF_PLUGIN_PROTOCOL_H
#define APOLLO_WEBKIT_PDF_PLUGIN_PROTOCOL_H

//	Turn this on to have the plug-in host be able to supply a resource-loader
//	that does the fetches for us (but does not support byte-range fetching).
//	This will make us send PDFX_FILE_READ streams.  Right now we turn it off
//	because the only existing on (Apollo) doesn't work when Acrobat is in
//	a synchronous read loop.
//	In AdobePDFViewwre there is an identical macro and that needs to be turned
//	on as well.
#define	IMP_EXTERNAL_RESOURCE_LOADER		0

#if IMP_EXTERNAL_RESOURCE_LOADER

	//	The "ResouceLoader" methods implement the bridge between the Apollo
	//	Resource Loader and AdobePDFViewer, the Acrobat WebKit Plugin.  This
	//	object is passed into the entrypoint for AdobePDFViewer and it makes
	//	callbacks when it wants loading to start.  The protocol is when we
	//	get data from the Apollo Resource Loader and want to pass that data
	//	back to the plug-in.

	//	AdobePDFViewer needs to return an object that conforms to this protocol.
	@protocol ApolloWebKitPDFPluginObject

		//	TBD

	@end

	//	AdobePDFViewer needs to pass an object that conforms to this protocol
	//	when asking Apollo to load a URL.
	@protocol ApolloWebKitPDFPluginURLLoadingClient

		- (void) receivedRedirect: (NSURL *) URL;
		- (void) receivedResponseOfExpectedLength: (SInt64) expectedLength
							withHeaders: (NSDictionary *) headers
							andStatusCode: (int) statusCode
							andMIMEType: (NSString *) mimeType;
		- (void) receivedData: (NSData *) data;
		- (void) receivedAllData;

	@end

	//	Apollo WebKit Plugin Host for AdobePDFViewer needs to pass and object
	//	that conforms to this protocol.
	@protocol ApolloWebKitPDFPluginHost

		//	Called when AdobePDFViewer wants to load a URL with a GET.
		- (void) startLoadingURL: (NSURL *) URL
					forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client;
		
		//	Called when AdobePDFViewer wants to load a URL with a custom method and possibly data.
		- (void) startLoadingURL: (NSURL *) URL
					forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client
					withMethod: (NSString *) method
					andData: (NSData *) data;
		
		//	Called when AdobePDFViewer wants to stop load a URL.  Note that if there is more
		//	than one concurrent load of that URL (that is, startLoadingURL: was called more
		//	than once for the same URL), the results are undefined: a random one of those
		//	fetches will be stopped.
		- (void) stopLoadingURL: (NSURL *) URL
					forClient: (id<ApolloWebKitPDFPluginURLLoadingClient>) client;
		
	@end

#endif	//	IMP_EXTERNAL_RESOURCE_LOADER

#if defined(__OBJC__) && __OBJC__
//	AdobePDFViewer needs some WebFrame calls to do POST correctly; in Safari these
//	are implemented by the WebKit WebFrame class.
@protocol ApolloWebKitWebFramePluginHost

	- (void) loadRequest: (NSURLRequest *) request;
	- (id<ApolloWebKitWebFramePluginHost>) findFrameNamed: (NSString *) name;

@end
#endif /* __OBJC__ */

#endif	// APOLLO_WEBKIT_PDF_PLUGIN_PROTOCOL_H
