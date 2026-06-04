#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Scn/ScnChar.hpp"    // IWYU pragma: export -- char overload (noD ADL on `char`)
#include "SFML/Base/Scn/ScnCore.hpp"    // IWYU pragma: export
#include "SFML/Base/Scn/ScnNumeric.hpp" // IWYU pragma: export -- int / float / bool overloads (no ADL on built-ins)

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename S, typename T>
concept HasScnArg = ScnSource<S> && requires(S& src, T& out) { static_cast<bool>(scnArg(src, out)); };

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Low-level entry point. Dispatches to `scnArg(src, out)` and
/// returns the parser's success bit verbatim.
///
/// On `false`, `out` and the source position are unspecified: a parser may
/// consume a partial token before discovering that it cannot produce a value.
/// Callers that need transactional behavior must keep their own copy of `out`
/// and use a source with explicit mark/restore support.
///
////////////////////////////////////////////////////////////
template <ScnSource S, typename T>
[[nodiscard]] constexpr bool scnInto(S& src, T& out)
{
    if constexpr (priv::HasScnArg<S, T>)
    {
        return static_cast<bool>(scnArg(src, out));
    }
    else
    {
        static_assert(false,
                      "No matching `scnArg` overload for this target type. Include the body header that "
                      "provides it (e.g. <SFML/Base/Scn/ScnNumeric.hpp> for int/float/bool, "
                      "<SFML/Base/Scn/ScnChar.hpp> for char, <SFML/Base/Scn/ScnString.hpp> for "
                      "base::String) -- or provide your own `scnArg(MySource&, MyType&)` in MyType's or "
                      "MySource's namespace.");

        return false;
    }
}


////////////////////////////////////////////////////////////
/// \brief High-level entry point. Default-constructs a `T`, calls
/// `scnArg(src, tmp)`, and returns either the parsed value or `nullOpt`.
///
/// Requires `T` to be default-constructible. Anything more exotic --
/// reference types, pinning, partial parsing -- should use `scnInto`.
///
////////////////////////////////////////////////////////////
template <typename T, ScnSource S>
[[nodiscard]] constexpr Optional<T> scn(S& src)
{
    T tmp;

    if (!scnInto(src, tmp))
        return nullOpt;

    return makeOptional(static_cast<T&&>(tmp));
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Public `Scn` entry point. Includes the core source concepts/helpers
/// plus the primitive body headers required for built-in targets with
/// no ADL namespace (`char`, ints, floats, bool).
///
/// User types extend the system via an ADL `scnArg(src, out) -> bool`
/// overload in MySource's or MyType's own namespace. `scnInto` checks
/// for that expression directly instead of adding a catch-all `scnArg`
/// overload to the overload set, so base-class customizations remain
/// viable.
///
/// Headers like `ScnString.hpp` are intentionally not transitively
/// included: the target type (`sf::base::String`) brings `sf::base` in
/// via ADL, so its overload is reachable as long as that body header is
/// included somewhere in the TU.
///
////////////////////////////////////////////////////////////
