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
#include <runtime/JSLock.h>
#include <runtime/JSString.h>
#include "ClipboardApolloHelper.h"

#include "Clipboard.h"

#include <JSDOMBinding.h>
#include <JSLock.h>
#include <JSObject.h>
#include <JSString.h>
#include "Error.h"

namespace WebCore {

const String ClipboardApolloHelper::TEXT_TYPE("text/plain");
const String ClipboardApolloHelper::HTML_TYPE("text/html");
const String ClipboardApolloHelper::URI_LIST_TYPE("text/uri-list");
const String ClipboardApolloHelper::BITMAP_TYPE("image/x-vnd.adobe.air.bitmap");
const String ClipboardApolloHelper::FILE_LIST_TYPE("application/x-vnd.adobe.air.file-list");

ClipboardApolloHelper::ClipboardApolloHelper(JSC::JSObject* dataTransfer, JSC::ExecState* execState)
    : m_dataTransfer(dataTransfer)
    , m_exec(execState)
{
}

ClipboardApolloHelper::~ClipboardApolloHelper()
{
}

WTF::HashSet<String> ClipboardApolloHelper::types()
{
    JSC::JSLock lock(false);

    JSC::JSObject* types = m_dataTransfer->get(m_exec, JSC::Identifier(m_exec, "types")).toObject(m_exec);
    uint32_t length = types->get(m_exec, JSC::Identifier(m_exec, "length")).toUInt32(m_exec);
    WTF::HashSet<String> result;

    for (uint32_t i=0; i<length; i++) {
        String s(ustringToString(types->get(m_exec, i).toString(m_exec)));
        result.add(s);
    }

    if (m_exec->hadException())
        m_exec->clearException();

    return result;
}

JSC::JSValue ClipboardApolloHelper::getData(const String& type, bool& success)
{
    JSC::JSLock lock(false);

	JSC::JSObject* getDataFunction = m_dataTransfer->get(m_exec, JSC::Identifier(m_exec, "getData")).toObject(m_exec);
    JSC::CallData callData;
    JSC::CallType callType = getDataFunction->getCallData(callData);
    if (callType == JSC::CallTypeNone) {
        //FIXME: RHU - should we throw the error?
        JSC::JSValue errorData = throwError(m_exec, JSC::TypeError);
        success = !m_exec->hadException();
        m_exec->clearException();
        return errorData;
    }
    JSC::MarkedArgumentBuffer args;
    args.append(JSC::jsString(m_exec, stringToUString(type)));
	
    JSC::JSValue data = call(m_exec, getDataFunction, callType, callData, m_dataTransfer, args);
	
    success = !m_exec->hadException();
    m_exec->clearException();

    return data;
}

bool ClipboardApolloHelper::setData(const String& type, JSC::JSValue data)
{
    JSC::JSLock lock(false);
	JSC::JSObject* setDataFunction = m_dataTransfer->get(m_exec, JSC::Identifier(m_exec, "setData")).toObject(m_exec);
    JSC::CallData callData;
    JSC::CallType callType = setDataFunction->getCallData(callData);
    if (callType == JSC::CallTypeNone) {
        //FIXME: RHU - throw an error?
        return false;
    }
    JSC::MarkedArgumentBuffer args;
    args.append(JSC::jsString(m_exec, stringToUString(type)));
    args.append(data);
    call(m_exec, setDataFunction, callType, callData, m_dataTransfer, args);
	
    bool success = !m_exec->hadException();
    m_exec->clearException();

    return success;
}

void ClipboardApolloHelper::clearData(const String& type)
{
    JSC::JSLock lock(false);
	
	JSC::JSObject* clearDataFunction = m_dataTransfer->get(m_exec, JSC::Identifier(m_exec, "clearData")).toObject(m_exec);
    JSC::CallData callData;
    JSC::CallType callType = clearDataFunction->getCallData(callData);
    if (callType == JSC::CallTypeNone) {
        //FIXME: RHU - throw an error?
        return;
    }
    JSC::MarkedArgumentBuffer args;
    args.append(JSC::jsString(m_exec, stringToUString(type)));
    call(m_exec, clearDataFunction, callType, callData, m_dataTransfer, args);
    if (m_exec->hadException())
        m_exec->clearException();
}

void ClipboardApolloHelper::clearAllData()
{
    JSC::JSLock lock(false);

	JSC::JSObject* clearAllDataFunction = m_dataTransfer->get(m_exec, JSC::Identifier(m_exec, "clearAllData")).toObject(m_exec);
    JSC::CallData callData;
    JSC::CallType callType = clearAllDataFunction->getCallData(callData);
    if (callType == JSC::CallTypeNone) {
        //FIXME: RHU - throw an error?
        return;
    }
    JSC::ArgList args;
    call(m_exec, clearAllDataFunction, callType, callData, m_dataTransfer, args);
    if (m_exec->hadException())
        m_exec->clearException();
}

DragOperation ClipboardApolloHelper::dragOperationAllowed()
{
    JSC::JSValue effectAllowedValue = m_dataTransfer->get(m_exec, JSC::Identifier(m_exec, "effectAllowed"));
    if (!effectAllowedValue.isString())
        return DragOperationNone;
    String effectAllowedString(ustringToString(effectAllowedValue.toString(m_exec)));
    return Clipboard::dragOpFromIEOp(effectAllowedString);
}

void ClipboardApolloHelper::setDropEffect(const String& dropEffect)
{
    JSC::JSLock lock(false);
	JSC::PutPropertySlot slot;
    m_dataTransfer->put(m_exec, JSC::Identifier(m_exec, stringToUString("dropEffect")), JSC::jsString(m_exec, stringToUString(dropEffect)), slot);
    if (m_exec->hadException())
        m_exec->clearException();
}

void ClipboardApolloHelper::setDragOperation(DragOperation operation)
{
    setDropEffect(Clipboard::IEOpFromDragOp(operation));
}

void ClipboardApolloHelper::setPropagationStopped(bool stopped)
{
    JSC::JSLock lock(false);
	JSC::PutPropertySlot slot;
    m_dataTransfer->put(m_exec, JSC::Identifier(m_exec, "propagationStopped"), JSC::jsBoolean(stopped), slot);
    if (m_exec->hadException())
        m_exec->clearException();
}

String ClipboardApolloHelper::getDataString(const String& type, bool& success)
{
    JSC::JSValue data = getData(type, success);
    if (!success)
        return String();

    if (!data.isString()) {
        success = false;
        return String();
    }

    JSC::JSLock lock(false);

    String result(ustringToString(data.toString(m_exec)));
    success = !m_exec->hadException();
    m_exec->clearException();
    return result;
}

bool ClipboardApolloHelper::setData(const String& type, const String& data)
{
    JSC::JSLock lock(false);
    return setData(type, JSC::jsString(m_exec, stringToUString(data)));
}

JSC::JSObject* ClipboardApolloHelper::dataTransfer() const
{
    return m_dataTransfer;
}

JSC::ExecState* ClipboardApolloHelper::execState() const
{
    return m_exec;
}

}
