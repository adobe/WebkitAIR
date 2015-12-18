#include "WebClipboard.h"
#include <Clipboard.h>

namespace WebKitApollo {

WebClipboard::WebClipboard(KJS::JSObject* dataTransfer, KJS::ExecState* execState)
    : m_dataTransfer(dataTransfer)
    , m_exec(execState)
{
}

WebClipboard::~WebClipboard()
{
}

WTF::HashSet<WebCore::String> WebClipboard::types()
{
    KJS::JSLock lock;

    KJS::JSObject* types = m_dataTransfer->get(m_exec, KJS::Identifier("types"))->toObject(m_exec);
    uint32_t length = types->get(m_exec, KJS::Identifier("length"))->toUInt32(m_exec);
    WTF::HashSet<WebCore::String> result;

    for (uint32_t i=0; i<length; i++) {
        const WebCore::String s(types->get(m_exec, i)->toString(m_exec));
        result.add(s);
    }

    if (m_exec->hadException())
        m_exec->clearException();

    return result;
}

KJS::JSValue* WebClipboard::getData(const WebCore::String& type, bool& success)
{
    KJS::JSLock lock;

    KJS::JSObject* getDataFunction = m_dataTransfer->get(m_exec, KJS::Identifier("getData"))->toObject(m_exec);
    KJS::List args;
    args.append(KJS::jsString(type));
    KJS::JSValue* data = getDataFunction->callAsFunction(m_exec, m_dataTransfer, args);

    success = !m_exec->hadException();
    m_exec->clearException();

    return data;
}

bool WebClipboard::setData(const WebCore::String& type, KJS::JSValue* data)
{
    KJS::JSLock lock;
    KJS::JSObject* setDataFunction = m_dataTransfer->get(m_exec, KJS::Identifier("setData"))->toObject(m_exec);
    KJS::List args;
    args.append(KJS::jsString(type));
    args.append(data);
    setDataFunction->callAsFunction(m_exec, m_dataTransfer, args);

    bool success = !m_exec->hadException();
    m_exec->clearException();

    return success;
}

void WebClipboard::clearData(const WebCore::String& type)
{
    KJS::JSLock lock;

    KJS::JSObject* clearDataFunction = m_dataTransfer->get(m_exec, KJS::Identifier("clearData"))->toObject(m_exec);
    KJS::List args;
    args.append(KJS::jsString(type));
    clearDataFunction->callAsFunction(m_exec, m_dataTransfer, args);
    if (m_exec->hadException())
        m_exec->clearException();
}

void WebClipboard::clearAllData()
{
    KJS::JSLock lock;

    KJS::JSObject* clearAllDataFunction = m_dataTransfer->get(m_exec, KJS::Identifier("clearAllData"))->toObject(m_exec);
    KJS::List args;
    clearAllDataFunction->callAsFunction(m_exec, m_dataTransfer, args);
    if (m_exec->hadException())
        m_exec->clearException();
}

WebCore::DragOperation WebClipboard::dragOperationAllowed()
{
    KJS::JSValue* effectAllowedValue = m_dataTransfer->get(m_exec, KJS::Identifier("effectAllowed"));
    if (!effectAllowedValue->isString())
        return WebCore::DragOperationNone;
    WebCore::String effectAllowedString(effectAllowedValue->toString(m_exec));
    return WebCore::Clipboard::dragOpFromIEOp(effectAllowedString);
}

void WebClipboard::setDropEffect(const WebCore::String& dropEffect)
{
    KJS::JSLock lock;
    m_dataTransfer->put(m_exec, KJS::Identifier("dropEffect"), KJS::jsString(dropEffect));
    if (m_exec->hadException())
        m_exec->clearException();
}

void WebClipboard::setDragOperation(WebCore::DragOperation operation)
{
    setDropEffect(WebCore::Clipboard::IEOpFromDragOp(operation));
}

WebCore::String WebClipboard::getDataString(const WebCore::String& type, bool& success)
{
    KJS::JSValue* data = getData(type, success);
    if (!success)
        return WebCore::String();

    if (!data->isString()) {
        success = false;
        return WebCore::String();
    }

    KJS::JSLock lock;

    WebCore::String result(data->toString(m_exec));
    success = !m_exec->hadException();
    m_exec->clearException();
    return result;
}

bool WebClipboard::setData(const WebCore::String& type, const WebCore::String& data)
{
    KJS::JSLock lock;
    return setData(type, KJS::jsString(data));
}

KJS::JSObject* WebClipboard::dataTransfer() const
{
    return m_dataTransfer;
}

KJS::ExecState* WebClipboard::execState() const
{
    return m_exec;
}

}
