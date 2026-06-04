#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: export -- callers virtually always want numeric support
#include "SFML/Base/NonDeduced.hpp"
#include "SFML/Base/String.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] String fmtToString(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    String out;
    (void)fmtTo(out, fmtStr, args...);
    return out;
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file Opt-in extension: `fmtToString(fmt, args...) -> base::String`.
///
/// Lives in a separate header so the lightweight `<SFML/Base/Fmt/Fmt.hpp>`
/// stays free of `<SFML/Base/String.hpp>`. Include this when you need the
/// String-returning convenience; otherwise prefer `fmtTo` / `fmtIntoBuffer`.
///
////////////////////////////////////////////////////////////
