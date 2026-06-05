#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Fmt/FmtResult.hpp"
#include "Zancle/Fmt/FmtSink.hpp"
#include "Zancle/Fmt/FmtSpec.hpp"
#include "Zancle/Base/SizeT.hpp"


namespace za
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
        ZA_ASSERT(false && "invalid string format spec");
        return FmtResult::Failed;
    }

    return sink.append(static_cast<const char*>(arg.data()), static_cast<SizeT>(arg.size()));
}

} // namespace za


////////////////////////////////////////////////////////////
/// \file
/// Built-in `fmtArg` for string-like types: anything exposing byte
/// `.data()` and `.size()` (e.g. `za::String`, `za::StringView`).
///
////////////////////////////////////////////////////////////
