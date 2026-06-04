#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace zb
{
////////////////////////////////////////////////////////////
/// \brief Result of a Fmt operation.
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] FmtResult : unsigned char
{
    Ok,       //!< Formatting/writing succeeded
    Overflow, //!< Destination buffer was too small; retrying with more room may succeed
    Failed    //!< Formatter could not produce output; retrying with more room will not help
};


////////////////////////////////////////////////////////////
#define ZB_FMT_TRY(...)                                        \
    do                                                                \
    {                                                                 \
        const ::zb::FmtResult zancleFmtTryResult = (__VA_ARGS__); \
                                                                      \
        if (zancleFmtTryResult != ::zb::FmtResult::Ok)            \
            return zancleFmtTryResult;                                  \
                                                                      \
    } while (false)

} // namespace zb
