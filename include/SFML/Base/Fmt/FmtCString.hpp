#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Fmt/FmtResult.hpp"
#include "SFML/Base/Fmt/FmtSink.hpp"
#include "SFML/Base/Fmt/FmtSpec.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
// Null-terminated C string
[[nodiscard, gnu::always_inline]] inline constexpr FmtResult fmtArg(FmtSink&                        sink,
                                                                    const char* const               arg,
                                                                    [[maybe_unused]] const FmtSpec& spec) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(arg != nullptr);

    if (spec.precision >= 0 || spec.type != '\0') [[unlikely]]
    {
        SFML_BASE_ASSERT(false && "invalid C string format spec");
        return FmtResult::Failed;
    }

    return sink.append(arg, SFML_BASE_STRLEN(arg));
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for null-terminated C strings (`const char*`).
///
////////////////////////////////////////////////////////////
