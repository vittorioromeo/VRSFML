#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Export.hpp"

#include "Zancle/System/Time.hpp"

#include "ZancleBase/StdChrono.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Utilities for converting between `za::Time` and `<chrono>` durations
///
////////////////////////////////////////////////////////////
class ZA_SYSTEM_API TimeChronoUtil
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Convert any `std::chrono::duration` to the equivalent `za::Time`
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

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::TimeChronoUtil
/// \ingroup system
///
/// The `za::TimeChronoUtil` class provides static helper functions to simplify
/// the conversion between Zancle's time representation (`za::Time`) and the
/// standard C++ time library (`<chrono>`). This allows for easier integration
/// with other libraries or code that uses `std::chrono`.
///
/// \see za::Time, za::Clock
///
////////////////////////////////////////////////////////////
