#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Scn/ScnCore.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `scnArg` for a single `char`.
///
/// **Does NOT skip leading whitespace** -- matches scnlib behavior. If
/// you want skip-then-read, do `scnSkipWhitespace(src)` yourself, or
/// parse into a `String` (which will pull the next whitespace-delimited
/// token).
///
/// Returns `false` at EOF; leaves `out` untouched.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] constexpr bool scnArg(ScnSource auto& src, char& out)
{
    auto c = src.peek();

    if (!c)
        return false;

    out = *c;
    src.consume();

    return true;
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Built-in `scnArg` for `char`. Header-only; intentionally tiny -- a
/// single byte read is cheaper than every codegen alternative.
///
////////////////////////////////////////////////////////////
