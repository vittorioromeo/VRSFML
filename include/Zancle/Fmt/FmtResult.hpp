#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za
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
#define ZA_FMT_TRY(...)                                           \
    do                                                            \
    {                                                             \
        const ::za::FmtResult zancleFmtTryResult = (__VA_ARGS__); \
                                                                  \
        if (zancleFmtTryResult != ::za::FmtResult::Ok)            \
            return zancleFmtTryResult;                            \
                                                                  \
    } while (false)

} // namespace za
