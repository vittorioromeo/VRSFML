#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/System/Time.hpp>

#include <chrono>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Interoperability utilities between `sf::Time` and `<chrono>`
///
////////////////////////////////////////////////////////////
class TimeChronoUtil
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a `Time` from a `std::chrono::duration`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Rep, typename Period>
    [[nodiscard]] static constexpr Time fromDuration(const std::chrono::duration<Rep, Period>& duration);

    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a `std::chrono::microseconds` duration
    ///
    /// \return Time in microseconds
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr std::chrono::microseconds toDuration(Time time);

    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a custom `std::chrono::duration`
    ///
    /// \return Duration in microseconds
    ///
    ////////////////////////////////////////////////////////////
    template <typename Rep, typename Period>
    [[nodiscard]] static constexpr std::chrono::duration<Rep, Period> toCustomDuration(Time time);
};

} // namespace sf

#include <SFML/System/TimeChronoUtil.inl>


////////////////////////////////////////////////////////////
/// \class sf::TimeChronoUtil
/// \ingroup system
///
/// Interoperability utilities between `sf::Time` and `<chrono>`
///
/// \see sf::Time, sf::Clock
///
////////////////////////////////////////////////////////////
