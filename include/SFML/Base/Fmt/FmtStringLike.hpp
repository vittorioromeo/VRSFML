#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Fmt/FmtSink.hpp"
#include "SFML/Base/Fmt/FmtSpec.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
// String-like: anything with `.data()` and `.size()`
template <typename T>
[[gnu::always_inline]] inline constexpr void fmtArg(FmtSink& sink, const T& arg, const FmtSpec&) noexcept
    requires requires {
        arg.data();
        arg.size();
    }
{
    sink.append(arg.data(), static_cast<SizeT>(arg.size()));
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for string-like types: anything exposing
/// `.data()` and `.size()` (e.g. `base::String`, `base::StringView`).
////////////////////////////////////////////////////////////
