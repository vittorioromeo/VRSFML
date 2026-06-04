#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Fmt/FmtResult.hpp"
#include "ZancleBase/Fmt/FmtSink.hpp"
#include "ZancleBase/Fmt/FmtSpec.hpp"
#include "ZancleBase/SizeT.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
// String-like: anything with byte `.data()` and `.size()`
template <typename T>
[[nodiscard, gnu::always_inline]] inline constexpr FmtResult fmtArg(FmtSink&                        sink,
                                                                    const T&                        arg,
                                                                    [[maybe_unused]] const FmtSpec& spec) noexcept
    requires requires {
        static_cast<const char*>(arg.data());
        static_cast<SizeT>(arg.size());
    }
{
    if (spec.precision >= 0 || spec.type != '\0') [[unlikely]]
    {
        ZB_ASSERT(false && "invalid string format spec");
        return FmtResult::Failed;
    }

    return sink.append(static_cast<const char*>(arg.data()), static_cast<SizeT>(arg.size()));
}

} // namespace zb


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for string-like types: anything exposing byte
/// `.data()` and `.size()` (e.g. `base::String`, `base::StringView`).
///
////////////////////////////////////////////////////////////
