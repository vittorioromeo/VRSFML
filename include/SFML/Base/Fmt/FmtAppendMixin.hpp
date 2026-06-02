#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Fmt/Fmt.hpp"               // IWYU pragma: export -- consumers of `.appendFmt` want Fmt itself too
#include "SFML/Base/Fmt/FmtAppendMixinFwd.hpp" // IWYU pragma: export
#include "SFML/Base/NonDeduced.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Args>
constexpr void FmtAppendMixin::appendFmt(this auto&&                                         self,
                                         typename NonDeduced<const FmtString<Args...>>::type fmtStr,
                                         const Args&... args)
{
    // The `FmtResult` is discarded by design: this mixin is for sinks
    // that grow on demand and cannot run out of room (e.g. `String`,
    // `Utf8String`). Sinks with a `FmtResult`-returning `append` should
    // call `fmtTo(*this, ...)` directly and inspect the result.
    (void)fmtTo(self, fmtStr, args...);
}


////////////////////////////////////////////////////////////
template <typename T>
void FmtAppendMixin::appendArg(this auto&& self, const T& value)
{
    (void)fmtArgTo(self, value);
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
