#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/SuspendAwareClock.hpp>

#include <ctime>

namespace sf
{

SuspendAwareClock::time_point SuspendAwareClock::now() noexcept
{
    ::timespec ts{};
#ifdef CLOCK_BOOTTIME
    clock_gettime(CLOCK_BOOTTIME, &ts);
#else
#error "CLOCK_BOOTTIME is essential for SuspendAwareClock to work"
#endif // CLOCK_BOOTTIME
    return time_point(std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec));
}

} // namespace sf
