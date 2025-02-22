#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/String.hpp"
#include "SFML/System/StringUtfUtils.hpp" // NOLINT(misc-header-include-cycle)
#include "SFML/System/Utf.hpp"

#include "SFML/Base/Algorithm.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
String StringUtfUtils::fromUtf8(T begin, T end)
{
    String string;
    Utf8::toUtf32(begin, end, base::BackInserter(*static_cast<std::u32string*>(string.getImplString())));
    return string;
}


////////////////////////////////////////////////////////////
template <typename T>
String StringUtfUtils::fromUtf16(T begin, T end)
{
    String string;
    Utf16::toUtf32(begin, end, base::BackInserter(*static_cast<std::u32string*>(string.getImplString())));
    return string;
}


////////////////////////////////////////////////////////////
template <typename T>
String StringUtfUtils::fromUtf32(T begin, T end)
{
    String string;
    static_cast<std::u32string*>(string.getImplString())->assign(begin, end);
    return string;
}

} // namespace sf
