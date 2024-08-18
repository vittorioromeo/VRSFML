#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Time.hpp"
#include "SFML/System/Unix/SleepImpl.hpp"

#include <cerrno>
#include <ctime>


namespace sf::priv
{
////////////////////////////////////////////////////////////
void sleepImpl(Time time)
{
    const std::int64_t usecs = time.asMicroseconds();

    // Construct the time to wait
    timespec ti{};
    ti.tv_sec  = static_cast<time_t>(usecs / 1000000);
    ti.tv_nsec = static_cast<long>((usecs % 1000000) * 1000);

    // Wait...
    // If nanosleep returns -1, we check errno. If it is EINTR
    // nanosleep was interrupted and has set ti to the remaining
    // duration. We continue sleeping until the complete duration
    // has passed. We stop sleeping if it was due to an error.
    while ((nanosleep(&ti, &ti) == -1) && (errno == EINTR))
    {
    }
}

} // namespace sf::priv
