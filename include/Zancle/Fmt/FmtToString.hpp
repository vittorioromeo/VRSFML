#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp" // IWYU pragma: export -- callers virtually always want numeric support

#include "Zancle/String/String.hpp"

#include "Zancle/Base/NonDeduced.hpp"


namespace za
{
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] String fmtToString(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    String out;
    (void)fmtTo(out, fmtStr, args...);
    return out;
}

} // namespace za


////////////////////////////////////////////////////////////
/// \file
///
/// \brief Opt-in extension: `fmtToString(fmt, args...) -> za::String`.
///
/// Lives in a separate header so the lightweight `<Zancle/Fmt/Fmt.hpp>`
/// stays free of `<Zancle/String/String.hpp>`. Include this when you need the
/// String-returning convenience; otherwise prefer `fmtTo` / `fmtIntoBuffer`.
///
////////////////////////////////////////////////////////////
