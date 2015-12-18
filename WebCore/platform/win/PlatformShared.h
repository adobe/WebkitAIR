#ifndef PLATFORM_SHARED_H
#define PLATFORM_SHARED_H

#include <mlang.h>
#include "win/COMPtr.h"

namespace Shared {
    typedef COMPtr<IMultiLanguage2> MultiLangCOMPtr;
    MultiLangCOMPtr getIMultiLanguage();
}


#endif /* PLATFORM_SHARED_H */