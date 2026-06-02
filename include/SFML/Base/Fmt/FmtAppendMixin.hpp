#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Fmt/Fmt.hpp"               // IWYU pragma: export -- consumers of `.appendFmt` want Fmt itself too
#include "SFML/Base/Fmt/FmtAppendMixinFwd.hpp" // IWYU pragma: export
#include "SFML/Base/Fmt/FmtResult.hpp"
#include "SFML/Base/NonDeduced.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Args>
constexpr bool FmtAppendMixin::appendFmt(this auto&&                                         self,
                                         typename NonDeduced<const FmtString<Args...>>::type fmtStr,
                                         const Args&... args)
{
    return fmtTo(self, fmtStr, args...) == FmtResult::Ok;
}


////////////////////////////////////////////////////////////
template <typename T>
bool FmtAppendMixin::appendArg(this auto&& self, const T& value)
{
    return fmtArgTo(self, value) == FmtResult::Ok;
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Out-of-class definition of `FmtAppendMixin::appendFmt`. Include this
/// header to call `.appendFmt(...)` on a type that inherits from the
/// mixin. The forward header (included transitively from `String.hpp`,
/// `Utf8String.hpp`, etc.) is enough for inheritance alone.
///
////////////////////////////////////////////////////////////
