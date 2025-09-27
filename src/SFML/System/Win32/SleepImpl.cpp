// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Win32/SleepImpl.hpp"

#include "SFML/System/Time.hpp"
#include "SFML/System/Win32/WindowsHeader.hpp"

#include <mmsystem.h>


namespace sf::priv
{
////////////////////////////////////////////////////////////
void sleepImpl(Time time)
{
    // Get the minimum supported timer resolution on this system
    static const UINT periodMin = []
    {
        TIMECAPS tc;
        timeGetDevCaps(&tc, sizeof(TIMECAPS));
        return tc.wPeriodMin;
    }();

    // Set the timer resolution to the minimum for the Sleep call
    timeBeginPeriod(periodMin);

    // Wait...
    ::Sleep(static_cast<DWORD>(time.asMilliseconds()));

    // Reset the timer resolution back to the system default
    timeEndPeriod(periodMin);
}

} // namespace sf::priv
