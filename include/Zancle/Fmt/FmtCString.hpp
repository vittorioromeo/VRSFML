#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Fmt/FmtResult.hpp"
#include "Zancle/Fmt/FmtSink.hpp"
#include "Zancle/Fmt/FmtSpec.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/AssertAndAssume.hpp"
#include "Zancle/Base/Strlen.hpp"


namespace za
{
////////////////////////////////////////////////////////////
// Null-terminated C string
[[nodiscard, gnu::always_inline]] inline constexpr FmtResult fmtArg(FmtSink&                        sink,
                                                                    const char* const               arg,
                                                                    [[maybe_unused]] const FmtSpec& spec) noexcept
{
    ZA_ASSERT_AND_ASSUME(arg != nullptr);

    if (spec.precision >= 0 || spec.type != '\0') [[unlikely]]
    {
        ZA_ASSERT(false && "invalid C string format spec");
        return FmtResult::Failed;
    }

    return sink.append(arg, ZA_STRLEN(arg));
}

} // namespace za


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for null-terminated C strings (`const char*`).
///
////////////////////////////////////////////////////////////
