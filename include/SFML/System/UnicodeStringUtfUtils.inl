// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/UnicodeString.hpp"
#include "SFML/System/UnicodeStringUtfUtils.hpp" // NOLINT(misc-header-include-cycle)
#include "SFML/System/Utf.hpp"

#include "SFML/Base/BackInserter.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
UnicodeString UnicodeStringUtfUtils::fromUtf8(T begin, T end)
{
    UnicodeString string;
    Utf8::toUtf32(begin, end, base::BackInserter(*static_cast<std::u32string*>(string.getImplString())));
    return string;
}


////////////////////////////////////////////////////////////
template <typename T>
UnicodeString UnicodeStringUtfUtils::fromUtf16(T begin, T end)
{
    UnicodeString string;
    Utf16::toUtf32(begin, end, base::BackInserter(*static_cast<std::u32string*>(string.getImplString())));
    return string;
}


////////////////////////////////////////////////////////////
template <typename T>
UnicodeString UnicodeStringUtfUtils::fromUtf32(T begin, T end)
{
    UnicodeString string;
    static_cast<std::u32string*>(string.getImplString())->assign(begin, end);
    return string;
}

} // namespace sf
