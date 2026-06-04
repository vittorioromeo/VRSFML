#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp" // IWYU pragma: export -- callers virtually always want numeric support
#include "ZancleBase/NonDeduced.hpp"
#include "ZancleBase/String.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] String fmtToString(typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args)
{
    String out;
    (void)fmtTo(out, fmtStr, args...);
    return out;
}

} // namespace zb


////////////////////////////////////////////////////////////
/// \file Opt-in extension: `fmtToString(fmt, args...) -> zb::String`.
///
/// Lives in a separate header so the lightweight `<ZancleBase/Fmt/Fmt.hpp>`
/// stays free of `<ZancleBase/String.hpp>`. Include this when you need the
/// String-returning convenience; otherwise prefer `fmtTo` / `fmtIntoBuffer`.
///
////////////////////////////////////////////////////////////
