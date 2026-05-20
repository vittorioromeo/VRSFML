#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/FmtSink.hpp"
#include "SFML/Base/Fmt/FmtSpec.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
// String-like: anything with byte `.data()` and `.size()`
template <typename T>
[[nodiscard, gnu::always_inline]] inline constexpr FmtResult fmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec) noexcept
    requires requires {
        static_cast<const char*>(arg.data());
        static_cast<SizeT>(arg.size());
    }
{
    SFML_BASE_ASSERT(spec.precision < 0 && spec.type == '\0');
    return sink.append(static_cast<const char*>(arg.data()), static_cast<SizeT>(arg.size()));
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for string-like types: anything exposing byte
/// `.data()` and `.size()` (e.g. `base::String`, `base::StringView`).
////////////////////////////////////////////////////////////
