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
#ifndef WEB_PLUGIN_PDF_IMPL_MAC_H
#define WEB_PLUGIN_PDF_IMPL_MAC_H

#include "WebPDFPluginProtocol.h"
#include "Widget.h"
#include "IntRect.h"
#include "KURL.h"
#include <wtf/Vector.h>
#include "HTTPHeaderMap.h"
#include "ResourceHandleClient.h"

struct WebWindow;
struct WebHost;

namespace JSC {
    namespace Bindings {
        class Instance;
    }
}

namespace WebCore
{
    class Element;
    class Frame;
    class ResourceHandle;
    
	class ApolloPDFPluginView : public Widget, public ResourceHandleClient
        {
        public:
            ApolloPDFPluginView( Frame* frame, Element *element,
                             const KURL& url,
                             const Vector<String>& paramNames, 
                             const Vector<String>& paramValues,
                             const String& mimeType, WebWindow* pWebWindow, WebHost * pWebHost );
            virtual ~ApolloPDFPluginView();
            
            static WTF::PassRefPtr<ApolloPDFPluginView> create(Frame* parentFrame, 
                                               const IntSize&, Element*, 
                                               const KURL&, 
                                               const Vector<String>& paramNames, 
                                               const Vector<String>& paramValues, 
                                               const String& mimeType, 
                                               bool loadManually, WebWindow* pWebWindow, WebHost * pWebHost);
            
        public:
            bool load();
            //virtual void updateWindowedPlugin(bool showWindowedPlugins);
            virtual void paint( WebCore::GraphicsContext* pGraphicsContext, const WebCore::IntRect& dirtyRect );
            virtual void show();
            virtual void hide();

            const WebCore::KURL& getUrl() const { return mURL; };
            const WebCore::String& getMimeType() const { return mMimeType; };

            void loadNSURLRequestInFrame( void *request, WebCore::Frame *targetFrame );
            void wrapperLoadNSURLRequestInFrame( void *request, WebCore::Frame *targetFrame );

            bool allowURLRequest( const KURL sourceURL, const KURL requestedURL, Frame *targetFrame );
            bool allowURLRequest( const KURL requestedURL, Frame *target );
            
            WebHost* webHost(){ return mpWebHost; }
            WebWindow* webWindow() {return mpWebWindow; }
            
            Frame* frame(){ return mFrame; }
            
            virtual void invalidateRect(const IntRect&);
            virtual void setFrameRect(const IntRect&);
            
            virtual void updatePluginWindow(bool canShowPlugins, bool canShowWindowedPlugins);
            
            virtual bool isApolloPDFPluginView() const { return true; }

            WTF::PassRefPtr<JSC::Bindings::Instance> bindingInstance();
            
        protected:
            virtual void frameGeometryHasChanged( const WebCore::IntRect &oldRect, const WebCore::IntRect& windowRect, const WebCore::IntRect& windowClipRect );
            
            void loadURLForClient( const KURL& urlToLoad, void *client );
            void loadURLForClient(	const KURL& urlToLoad, const char *method,
                                  const char *data, unsigned int dataLength,
                                  void *client,
                                  const HTTPHeaderMap& headerMap,
                                  Frame *frame = NULL );
            
#if IMP_EXTERNAL_RESOURCE_LOADER
		protected:
			//Overrides of WebCore::ResourceLoaderClient.  This will thunk back to the 
			void clientReceivedRedirect(void *client, WebCore::ResourceLoader*, const WebCore::KURL&);
			void clientReceivedResponse(void *client, WebCore::ResourceLoader*, WebCore::PlatformResponse);
			void clientReceivedData(void *client, WebCore::ResourceLoader*, const char*, int);
			void clientReceivedAllData(void *client, WebCore::ResourceLoader*);
			void clientReceivedAllData(void *client, WebCore::ResourceLoader*, WebCore::PlatformData);
            
		public:
			void pluginLoadURLRequest( WebCore::KURL& url, void *pluginClient );
#endif
            
        private:
            void *		mObjCBridge;
            bool		mEverGotNonzeroSize;
            
            Frame* mFrame;
            
            KURL		mURL;
            String		mMimeType;
            Vector<String> mParamNames;
            Vector<String> mParamValues;
            
            WebWindow* mpWebWindow;
            WebHost * mpWebHost;
        };
}

#endif
