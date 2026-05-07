#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/StdChrono.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utilities for converting between `sf::Time` and `<chrono>` durations
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API TimeChronoUtil
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Convert any `std::chrono::duration` to the equivalent `sf::Time`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Rep, typename Period>
    [[nodiscard]] static constexpr Time fromDuration(const std::chrono::duration<Rep, Period>& duration)
    {
        return microseconds(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a `std::chrono::microseconds` duration
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr std::chrono::microseconds toDuration(const Time time)
    {
        return std::chrono::microseconds(time.asMicroseconds());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a custom `std::chrono::duration<Rep, Period>`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Rep, typename Period>
    [[nodiscard]] static constexpr std::chrono::duration<Rep, Period> toCustomDuration(const Time time)
    {
        return std::chrono::microseconds(time.asMicroseconds());
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TimeChronoUtil
/// \ingroup system
///
/// The `sf::TimeChronoUtil` class provides static helper functions to simplify
/// the conversion between SFML's time representation (`sf::Time`) and the
/// standard C++ time library (`<chrono>`). This allows for easier integration
/// with other libraries or code that uses `std::chrono`.
///
/// \see sf::Time, sf::Clock
///
////////////////////////////////////////////////////////////
