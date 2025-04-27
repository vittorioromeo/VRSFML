#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/System/Time.hpp"

#include <chrono>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utilities for converting between `sf::Time` and `<chrono>` durations.
///
/// This class provides static functions to facilitate seamless conversion
/// between SFML's `sf::Time` and the standard C++ `<chrono>` library's duration types.
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API TimeChronoUtil
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a `Time` from a `std::chrono::duration`
    ///
    /// Converts any `std::chrono::duration` type into an equivalent `sf::Time`.
    ///
    /// \param duration The `std::chrono::duration` to convert.
    ///
    /// \return The equivalent `sf::Time` value.
    ///
    ////////////////////////////////////////////////////////////
    template <typename Rep, typename Period>
    [[nodiscard]] static constexpr Time fromDuration(const std::chrono::duration<Rep, Period>& duration)
    {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a `std::chrono::microseconds` duration
    ///
    /// \param time The `sf::Time` to convert.
    ///
    /// \return The equivalent duration in `std::chrono::microseconds`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr std::chrono::microseconds toDuration(const Time time)
    {
        return std::chrono::microseconds(time.asMicroseconds());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a custom `std::chrono::duration`
    ///
    /// \param time The `sf::Time` to convert.
    ///
    /// \return The equivalent duration represented by the specified `std::chrono::duration` template instantiation.
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
