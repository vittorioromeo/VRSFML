#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/TimeChronoUtil.hpp" // NOLINT(misc-header-include-cycle)

#include <chrono>


namespace sf
{
////////////////////////////////////////////////////////////
template <typename Rep, typename Period>
constexpr Time TimeChronoUtil::fromDuration(const std::chrono::duration<Rep, Period>& duration)
{
    return Time(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}


////////////////////////////////////////////////////////////
constexpr std::chrono::microseconds TimeChronoUtil::toDuration(Time time)
{
    return std::chrono::microseconds(time.asMicroseconds());
}


////////////////////////////////////////////////////////////
template <typename Rep, typename Period>
constexpr std::chrono::duration<Rep, Period> TimeChronoUtil::toCustomDuration(Time time)
{
    return std::chrono::microseconds(time.asMicroseconds());
}

} // namespace sf
