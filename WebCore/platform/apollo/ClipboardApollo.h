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
 
#ifndef ClipboardApollo_h
#define ClipboardApollo_h

#include "IntPoint.h"
#include "Clipboard.h"
#include "ClipboardAccessPolicy.h"
#include "CachedResourceClient.h"
#include "JSObject.h"
#include <wtf/OwnPtr.h>

namespace WebCore {

class ClipboardApolloHelper;

class ClipboardApollo : public Clipboard, public CachedResourceClient {
public:
    ClipboardApollo(bool forDragging, ClipboardApolloHelper*, ClipboardAccessPolicy, Frame*);

    virtual ~ClipboardApollo();

    virtual void clearData(const String &type);
    virtual void clearAllData();
    virtual String getData(const String& type, bool& success) const;
    virtual JSC::JSValue getDataValue(const String& type, bool& success) const;
    virtual WTF::PassRefPtr<Node> getDataNode(const String& type, bool& success) const;
    virtual bool setData(const String& type, const String& data);
    virtual bool setData(const String& type, JSC::JSValue data);
    virtual bool setData(const String& type, Node* data);

    virtual bool hasData();

    // extensions beyond IE's API
    virtual HashSet<String> types() const;
    virtual PassRefPtr<FileList> files() const;

    virtual void setDragImage(CachedImage*, const IntPoint&);
    virtual void setDragImageElement(Node*, const IntPoint&);

    virtual DragImageRef createDragImage(IntPoint& dragLoc) const;
    virtual void declareAndWriteDragImage(Element*, const KURL&, const String& title, Frame*);
    virtual void writeRange(Range*, Frame* frame);
    virtual void writeURL(const KURL&, const String&, Frame* frame);
    virtual void writePlainText(const String&, Frame*);

    JSC::JSObject* dataTransfer() const;

private:
    void setDragImage(CachedImage*, Node*, const IntPoint&);
    WebBitmap* dragWebBitmap(IntPoint &loc) const;

    WTF::OwnPtr<ClipboardApolloHelper> m_clipboard;
    Frame* m_frame;
};

}

#endif
