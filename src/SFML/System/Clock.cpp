////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/TimeChronoUtil.hpp>

#ifdef SFML_SYSTEM_ANDROID
#include <SFML/System/SuspendAwareClock.hpp>
#endif

#include <chrono>
#include <ratio>
#include <type_traits>


namespace sf::priv
{

////////////////////////////////////////////////////////////
/// \brief Chooses a monotonic clock of highest resolution
///
/// The high_resolution_clock is usually an alias for other
/// clocks: steady_clock or system_clock, whichever has a
/// higher precision.
///
/// sf::Clock, however, is aimed towards monotonic time
/// measurements and so system_clock could never be a choice
/// as its subject to discontinuous jumps in the system time
/// (e.g., if the system administrator manually changes
/// the clock), and by the incremental adjustments performed
/// by `adjtime` and Network Time Protocol. On the other
/// hand, monotonic clocks are unaffected by this behavior.
///
/// Note: Linux implementation of a monotonic clock that
/// takes sleep time into account is represented by
/// CLOCK_BOOTTIME. Android devices can define the macro:
/// SFML_ANDROID_USE_SUSPEND_AWARE_CLOCK to use a separate
/// implementation of that clock, instead.
///
/// For more information on Linux clocks visit:
/// https://linux.die.net/man/2/clock_gettime
///
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_ANDROID) && defined(SFML_ANDROID_USE_SUSPEND_AWARE_CLOCK)
using ClockImpl = SuspendAwareClock;
#else
using ClockImpl = std::conditional_t<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>;
#endif

static_assert(ClockImpl::is_steady, "Provided implementation is not a monotonic clock");
static_assert(std::ratio_less_equal_v<ClockImpl::period, std::micro>,
              "Clock resolution is too low. Expecting at least a microsecond precision");

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
struct Clock::Impl
{
    priv::ClockImpl::time_point refPoint{priv::ClockImpl::now()}; //!< Time of last reset
    priv::ClockImpl::time_point stopPoint;                        //!< Time of last stop
};


////////////////////////////////////////////////////////////
Clock::Clock() = default;


////////////////////////////////////////////////////////////
Clock::~Clock() = default;


////////////////////////////////////////////////////////////
Clock::Clock(const Clock&) = default;


////////////////////////////////////////////////////////////
Clock& Clock::operator=(const Clock&) = default;


////////////////////////////////////////////////////////////
Clock::Clock(Clock&&) noexcept = default;


////////////////////////////////////////////////////////////
Clock& Clock::operator=(Clock&&) noexcept = default;


////////////////////////////////////////////////////////////
Time Clock::getElapsedTime() const
{
    const priv::ClockImpl::duration diff = isRunning() ? priv::ClockImpl::now() - m_impl->refPoint
                                                       : m_impl->stopPoint - m_impl->refPoint;

    return TimeChronoUtil::fromDuration(std::chrono::duration_cast<std::chrono::microseconds>(diff));
}


////////////////////////////////////////////////////////////
bool Clock::isRunning() const
{
    return m_impl->stopPoint == priv::ClockImpl::time_point();
}


////////////////////////////////////////////////////////////
void Clock::start()
{
    if (isRunning())
        return;

    m_impl->refPoint += priv::ClockImpl::now() - m_impl->stopPoint;
    m_impl->stopPoint = {};
}


////////////////////////////////////////////////////////////
void Clock::stop()
{
    if (!isRunning())
        return;

    m_impl->stopPoint = priv::ClockImpl::now();
}


////////////////////////////////////////////////////////////
Time Clock::restart()
{
    const Time elapsed = getElapsedTime();
    m_impl->refPoint   = priv::ClockImpl::now();
    m_impl->stopPoint  = {};
    return elapsed;
}


////////////////////////////////////////////////////////////
Time Clock::reset()
{
    const Time elapsed = getElapsedTime();
    m_impl->refPoint   = priv::ClockImpl::now();
    m_impl->stopPoint  = m_impl->refPoint;
    return elapsed;
}

} // namespace sf
