#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/NonDeduced.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::base
{
template <typename...>
struct FmtString;
}


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Mixin providing `.appendFmt(fmt, args...)` for `AppendSink` types.
///
/// Inherit publicly to get the method:
///
///     class MyBuilder : public sf::base::FmtAppendMixin { ... };
///
/// The inheriting type must satisfy the `AppendSink` concept (have
/// `append(const char* data, SizeT n)`). The formatter's `FmtResult` is
/// discarded -- intended for sinks that grow on demand and cannot run
/// out of room (e.g. `String`, `Utf8String`). Sinks with a
/// `FmtResult`-returning `append` should call `fmtTo(*this, ...)`
/// directly and inspect the result.
///
/// \note The body of `appendFmt` lives in `<SFML/Base/Fmt/FmtAppendMixin.hpp>`.
/// Including only the forward header is enough to inherit, construct, and
/// pass instances around; the call site of `appendFmt` must include the
/// full header for the template body to be visible.
///
////////////////////////////////////////////////////////////
struct FmtAppendMixin
{
    template <typename... Args>
    constexpr void appendFmt(this auto&& self, typename NonDeduced<const FmtString<Args...>>::type fmtStr, const Args&... args);

    template <typename T>
    void appendArg(this auto&& self, const T& value);
};

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Forward-declaration of `FmtAppendMixin`. Inheriting from the mixin only
/// requires this header; *calling* `appendFmt` additionally requires
/// `<SFML/Base/Fmt/FmtAppendMixin.hpp>` to bring in the function-template
/// definition (which in turn pulls `<SFML/Base/Fmt/Fmt.hpp>`).
///
/// Splitting the two lets foundational types like `sf::base::String` opt
/// into the `.appendFmt(...)` syntax without forcing every consumer of
/// `String.hpp` to transitively include the whole Fmt machinery.
///
////////////////////////////////////////////////////////////
