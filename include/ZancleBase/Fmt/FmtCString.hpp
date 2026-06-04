#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/AssertAndAssume.hpp"
#include "ZancleBase/Builtin/Strlen.hpp"
#include "ZancleBase/Fmt/FmtResult.hpp"
#include "ZancleBase/Fmt/FmtSink.hpp"
#include "ZancleBase/Fmt/FmtSpec.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
// Null-terminated C string
[[nodiscard, gnu::always_inline]] inline constexpr FmtResult fmtArg(FmtSink&                        sink,
                                                                    const char* const               arg,
                                                                    [[maybe_unused]] const FmtSpec& spec) noexcept
{
    ZB_ASSERT_AND_ASSUME(arg != nullptr);

    if (spec.precision >= 0 || spec.type != '\0') [[unlikely]]
    {
        ZB_ASSERT(false && "invalid C string format spec");
        return FmtResult::Failed;
    }

    return sink.append(arg, ZB_STRLEN(arg));
}

} // namespace zb


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for null-terminated C strings (`const char*`).
///
////////////////////////////////////////////////////////////
