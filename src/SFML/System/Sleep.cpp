// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#if defined(SFML_SYSTEM_WINDOWS)
    #include "SFML/System/Win32/SleepImpl.hpp"
#else
    #include "SFML/System/Unix/SleepImpl.hpp"
#endif


namespace sf
{
////////////////////////////////////////////////////////////
void sleep(Time duration)
{
    // Note that 'std::this_thread::sleep_for' is intentionally not used here
    // as it results in inconsistent sleeping times under MinGW-w64.

    if (duration >= Time{})
        priv::sleepImpl(duration);
}

} // namespace sf
