
#include "config.h"
#include "unicode/Collator.h"

#if USE(WIN_OS_UNICODE) && !UCONFIG_NO_COLLATION

#include "Assertions.h"
#include "Threading.h"
#include <wtf/HashMap.h>
#include <string.h>
#include <windows.h>
#include "win/PlatformShared.h"
#include "win/BString.h"

namespace WTF {

Collator::Collator(const char* locale)
    : m_haveLCID(false)
    , m_locale(locale ? strdup(locale) : 0)
    , m_lowerFirst(false)
{
}

PassOwnPtr<Collator> Collator::userDefault()
{
    return new Collator(0);
}

Collator::~Collator()
{
    free(m_locale);
}

void Collator::setOrderLowerFirst(bool lowerFirst)
{
    m_lowerFirst = lowerFirst;
}

namespace {
    static const DWORD caseSensitiveSortKeyFlags = LCMAP_SORTKEY; 
    static const DWORD caseInSensitiveSortKeyFlags = LCMAP_SORTKEY | NORM_IGNORECASE;
    static inline void createSortKey(LCID const lcid, bool const ignoreCase, const UChar* const s, size_t const len, Vector<unsigned char>& dest)
    {
        DWORD const sortKeyFlags = ignoreCase ? caseInSensitiveSortKeyFlags : caseSensitiveSortKeyFlags;
        int const sortKeyByteLen = LCMapStringW(lcid, sortKeyFlags, s, len, 0, 0);
        dest.grow(sortKeyByteLen);
        LCMapStringW(lcid, sortKeyFlags, s, len, reinterpret_cast<wchar_t*>(dest.data()), sortKeyByteLen);
    }

    static inline LCID getLCIDForLocaleName(char* localeName)
    {
        ASSERT(localeName);
        if (!localeName)
            return LOCALE_USER_DEFAULT;

        if (*localeName == '\0')
            return LOCALE_NEUTRAL;

        Shared::MultiLangCOMPtr multiLang = Shared::getIMultiLanguage();
        if (!multiLang)
            return LOCALE_NEUTRAL;

        LCID localeId = LOCALE_NEUTRAL;
        WebCore::String str(localeName);
        WebCore::BString bstrLocalName(str);
        if (multiLang->GetLcidFromRfc1766(&localeId, bstrLocalName) != S_OK)
            return LOCALE_NEUTRAL;

        return localeId;
    }

    static inline Collator::Result compareByteVectors(const Vector<unsigned char>& lhs, const Vector<unsigned char>& rhs)
    {
        int const memcmpResult = memcmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));
        if (memcmpResult < 0) {
            return Collator::Less;
        }
        else if (memcmpResult > 0) {
            return Collator::Greater;
        }
        else {
            ASSERT(memcmpResult == 0);
            if (lhs.size() < rhs.size())
                return Collator::Less;
            else if (lhs.size() > rhs.size())
                return Collator::Greater;
            else
                return Collator::Equal;
        }
    }
}

Collator::Result Collator::collate(const UChar* lhs, size_t lhsLength, const UChar* rhs, size_t rhsLength) const
{
    if (lhsLength == 0)
        if (rhsLength == 0)
            return Collator::Equal;
        else
            return Collator::Less;
    else if (rhsLength == 0)
        return Collator::Greater;

    if (!m_haveLCID) {
        m_collationLocale = getLCIDForLocaleName(m_locale);
        m_haveLCID = true;
    }

    Vector<unsigned char> lhsSortKey;
    createSortKey(m_collationLocale, true, lhs, lhsLength, lhsSortKey);

    Vector<unsigned char> rhsSortKey;
    createSortKey(m_collationLocale, true, rhs, rhsLength, rhsSortKey);

    Collator::Result const caseInsesitiveResult = compareByteVectors(lhsSortKey, rhsSortKey);
    if (caseInsesitiveResult == Collator::Equal) {
        createSortKey(m_haveLCID, false, lhs, lhsLength, lhsSortKey);
        createSortKey(m_haveLCID, false, rhs, rhsLength, rhsSortKey);
        Collator::Result const caseSensitiveResult = compareByteVectors(lhsSortKey, rhsSortKey);
        if (!m_lowerFirst)
            return caseSensitiveResult;
        if (caseSensitiveResult == Collator::Less)
            return Collator::Greater;
        else if (caseSensitiveResult == Collator::Greater)
            return Collator::Less;
        return Collator::Equal;
    }
    return caseInsesitiveResult;
}

}

#endif
