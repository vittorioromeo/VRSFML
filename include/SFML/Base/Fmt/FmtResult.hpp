#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Result of a Fmt operation.
////////////////////////////////////////////////////////////
enum class [[nodiscard]] FmtResult
{
    ok,       //!< Formatting/writing succeeded
    overflow, //!< Destination buffer was too small; retrying with more room may succeed
    failed    //!< Formatter could not produce output; retrying with more room will not help
};


////////////////////////////////////////////////////////////
#define SFML_BASE_FMT_TRY(...)                                        \
    do                                                                \
    {                                                                 \
        const ::sf::base::FmtResult sfmlFmtTryResult = (__VA_ARGS__); \
        if (sfmlFmtTryResult != ::sf::base::FmtResult::ok)            \
            return sfmlFmtTryResult;                                  \
    } while (false)

} // namespace sf::base
