#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include <chrono>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Android, chrono-compatible, suspend-aware clock
///
/// Linux steady clock is represented by CLOCK_MONOTONIC.
/// However, this implementation does not work properly for
/// long-running clocks that work in the background when the
/// system is suspended.
///
/// SuspendAwareClock uses CLOCK_BOOTTIME which is identical
/// to CLOCK_MONOTONIC, except that it also includes any time
/// that the system is suspended.
///
/// Note: In most cases, CLOCK_MONOTONIC is a better choice.
/// Make sure this implementation is required for your use case.
///
////////////////////////////////////////////////////////////
struct SFML_SYSTEM_API SuspendAwareClock
{
    ////////////////////////////////////////////////////////////
    /// \brief Type traits and static members
    ///
    /// These type traits and static members meet the requirements
    /// of a Clock concept in the C++ Standard. More specifically,
    /// TrivialClock requirements are met. Thus, naming convention
    /// has been kept consistent to allow for extended use e.g.
    /// https://en.cppreference.com/w/cpp/chrono/is_clock
    ///
    ////////////////////////////////////////////////////////////
    using duration   = std::chrono::nanoseconds;
    using rep        = duration::rep;
    using period     = duration::period;
    using time_point = std::chrono::time_point<SuspendAwareClock, duration>;

    static constexpr bool is_steady = true; // NOLINT(readability-identifier-naming)

    static time_point now() noexcept;
};

} // namespace sf
