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
#ifndef WebFocusManager_h
#define WebFocusManager_h

namespace WebCore
{
	class IWidgetApolloImpl;
}

namespace WebKitApollo
{
	class IFocusManager {
	public:
		virtual void setFocus( WebCore::IWidgetApolloImpl* const pWidgetImplToGetFocus ) = 0;
		virtual void clearFocus( ) = 0;
		virtual bool hasFocus( const WebCore::IWidgetApolloImpl* const pWidgetImpl ) const = 0;

	protected:
		//Only subclasses should instantiate
		IFocusManager();

		//Only sublcasses should be able to destroy a focus manager.
		virtual ~IFocusManager();
	private:
		IFocusManager& operator=( const IFocusManager& );
		IFocusManager( const IFocusManager& );
	};

	class IMouseCaptureManager {
	public:
		virtual void setMouseCapture( WebCore::IWidgetApolloImpl* const pWidgetImplToGetCapture ) = 0;
		virtual void releaseMouseCapture( ) = 0;
		virtual bool hasMouseCapture( const WebCore::IWidgetApolloImpl* const pWidgetImpl ) const = 0;

	protected:
		//Only subclasses should instantiate
		IMouseCaptureManager();

		//Only sublcasses should be able to destroy a mouse capture manager.
		virtual ~IMouseCaptureManager();
	private:
		IMouseCaptureManager& operator=( const IMouseCaptureManager& );
		IMouseCaptureManager( const IMouseCaptureManager& );
	};
}
#endif
