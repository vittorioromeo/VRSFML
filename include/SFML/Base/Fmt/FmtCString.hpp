#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Fmt/FmtSink.hpp"
#include "SFML/Base/Fmt/FmtSpec.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
// Null-terminated C string
[[gnu::always_inline]] inline constexpr void fmtArg(FmtSink& sink, const char* const arg, const FmtSpec&) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(arg != nullptr);
    sink.append(arg, SFML_BASE_STRLEN(arg));
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for null-terminated C strings (`const char*`).
////////////////////////////////////////////////////////////
