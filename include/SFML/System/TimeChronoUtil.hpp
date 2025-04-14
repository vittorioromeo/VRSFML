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
/// \brief Interoperability utilities between `sf::Time` and `<chrono>`
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API TimeChronoUtil
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a `Time` from a `std::chrono::duration`
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
    /// \return Time in microseconds
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr std::chrono::microseconds toDuration(const Time time)
    {
        return std::chrono::microseconds(time.asMicroseconds());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a custom `std::chrono::duration`
    ///
    /// \return Duration in microseconds
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
/// Interoperability utilities between `sf::Time` and `<chrono>`
///
/// \see sf::Time, sf::Clock
///
////////////////////////////////////////////////////////////
