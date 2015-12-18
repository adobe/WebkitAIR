#ifndef WebClipboard_h
#define WebClipboard_h

#include <AtomicString.h>
#include <DragActions.h>
#include <Shared.h>
#include <StringHash.h>
#include <kjs/object.h>
#include <kjs/protect.h>
#include <wtf/HashSet.h>
#include <ClipboardApollo.h>

namespace WebKitApollo {

class WebClipboard : public WebCore::ClipboardApolloHelper
{
public:
    WebClipboard(KJS::JSObject* clipboard, KJS::ExecState* exec);
    virtual ~WebClipboard();

    WTF::HashSet<WebCore::String> types();
    KJS::JSValue* getData(const WebCore::String& type, bool& success);
    bool setData(const WebCore::String& type, KJS::JSValue* data);
    void clearData(const WebCore::String& type);
    void clearAllData();

    WebCore::DragOperation dragOperationAllowed();
    void setDragOperation(WebCore::DragOperation operation);

    WebCore::String getDataString(const WebCore::String& type, bool& success);
    bool setData(const WebCore::String& type, const WebCore::String& data);

    KJS::JSObject* dataTransfer() const;
    KJS::ExecState* execState() const;

private:
    KJS::ProtectedPtr<KJS::JSObject> m_dataTransfer;
    KJS::ExecState* m_exec;

    void setDropEffect(const WebCore::String& dropEffect);
};

}

#endif // WebClipboard_h
