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
#include "DragData.h"

#include "ClipboardApollo.h"
#include "ClipboardApolloHelper.h"
#include "ClipboardAccessPolicy.h"
#include "Document.h"
#include "DocumentFragment.h"
#include "MIMETypeRegistry.h"
#include "runtime/JSObject.h"
#include "JSDOMBinding.h"
#include "FrameLoaderClientApollo.h"
#include "JSLock.h"

namespace WebCore {

bool DragData::canSmartReplace() const
{
    return false;
}

bool DragData::containsColor() const
{
    return false;
}

bool DragData::containsPlainText() const
{
    HashSet<String> availableTypes(m_platformDragData->types());
    return availableTypes.contains(ClipboardApolloHelper::TEXT_TYPE);
}

String DragData::asPlainText() const
{
    JSC::JSLock lock(false);
    bool success;
    JSC::JSValue data = m_platformDragData->getData(ClipboardApolloHelper::TEXT_TYPE, success);
    if (success && data.isString())
        return String(ustringToString(data.toString(m_platformDragData->execState())));

    Vector<String> filenames;
    asFilenames(filenames);
    if (!filenames.isEmpty()) {
        String result;
        for (unsigned int i=0; i<filenames.size(); i++)
            result.append(filenames[i] + "\n");
        return result;
    }

    String url(asURL(NULL));
    if (!url.isEmpty())
        return url;

    return String();
}

Color DragData::asColor() const
{
    return makeRGBA(0, 0, 0, 0);
}

WTF::PassRefPtr<Clipboard> DragData::createClipboard(ClipboardAccessPolicy policy, Frame* frame) const
{
    ASSERT(frame);
    return adoptRef(new ClipboardApollo(true, new ClipboardApolloHelper(*m_platformDragData), policy, frame));
}

bool DragData::containsCompatibleContent() const
{
    HashSet<String> availableTypes(m_platformDragData->types());
    return availableTypes.contains(ClipboardApolloHelper::TEXT_TYPE)
        || availableTypes.contains(ClipboardApolloHelper::HTML_TYPE)
        || availableTypes.contains(ClipboardApolloHelper::URI_LIST_TYPE)
        || availableTypes.contains(ClipboardApolloHelper::BITMAP_TYPE)
        || availableTypes.contains(ClipboardApolloHelper::FILE_LIST_TYPE);
}

bool DragData::containsURL() const
{
    HashSet<String> availableTypes(m_platformDragData->types());
    return availableTypes.contains(ClipboardApolloHelper::URI_LIST_TYPE);
}

String DragData::asURL(String* /*title*/) const
{
    JSC::JSLock lock(false);
    bool success;
    JSC::JSValue data = m_platformDragData->getData(ClipboardApolloHelper::URI_LIST_TYPE, success);
    if (success && data.isString())
        return String(ustringToString(data.toString(m_platformDragData->execState())));
    return String();
}

PassRefPtr<DocumentFragment> DragData::asFragment(Document* /*doc*/) const
{
    return NULL;
}

void DragData::asFilenames(Vector<String>& result) const
{
    bool success;
    JSC::JSValue data = m_platformDragData->getData(ClipboardApolloHelper::FILE_LIST_TYPE, success);
    JSC::ExecState *exec = m_platformDragData->execState();
    if (success && data.isObject()) {
        JSC::JSObject* filenameArray = data.toObject(exec);
        uint32_t length = filenameArray->get(exec, JSC::Identifier(exec, "length")).toUInt32(exec);
        for (uint32_t i=0; i<length; i++) {
            JSC::JSValue fileValue = filenameArray->get(exec, i);
            if (fileValue.isObject()) {
                JSC::JSObject* file = fileValue.toObject(exec);
                JSC::JSValue pathValue = file->get(exec, JSC::Identifier(exec, "nativePath"));
                if (pathValue.isString()) {
                    String path = ustringToString(pathValue.toString(exec));
                    result.append(path);
                }
            }
        }
    }
    if (exec->hadException())
        exec->clearException();
}

bool DragData::containsFiles() const
{
    HashSet<String> availableTypes(m_platformDragData->types());
    return availableTypes.contains(ClipboardApolloHelper::FILE_LIST_TYPE);
}

}
