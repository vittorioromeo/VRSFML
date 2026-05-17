#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/MiniFmt.hpp"
#include "SFML/Base/MiniFmtNumeric.hpp" // IWYU pragma: export -- format() callers virtually always want numeric support
#include "SFML/Base/String.hpp"


////////////////////////////////////////////////////////////
/// \brief Opt-in extension: `format(fmt, args...) -> base::String`.
///
/// Lives in a separate header so the lightweight `<SFML/Base/MiniFmt.hpp>`
/// stays free of `<SFML/Base/String.hpp>`. Include this when you need the
/// String-returning convenience; otherwise prefer `formatTo` / `formatIntoBuffer`.
////////////////////////////////////////////////////////////


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] String format(typename NonDeduced<const FormatString<Args...>>::type fmt, const Args&... args)
{
    String out;
    formatTo(out, fmt, args...);
    return out;
}

} // namespace sf::base
