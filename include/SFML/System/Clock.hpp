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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/System/InPlacePImpl.hpp>


namespace sf
{
class Time;

////////////////////////////////////////////////////////////
/// \brief Utility class that measures the elapsed time
///
/// The clock starts automatically after being constructed.
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API Clock
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    Clock();

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    ~Clock();

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    Clock(const Clock& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    Clock& operator=(const Clock& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    Clock(Clock&&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    Clock& operator=(Clock&&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Get the elapsed time
    ///
    /// This function returns the time elapsed since the last call
    /// to restart() (or the construction of the instance if restart()
    /// has not been called).
    ///
    /// \return Time elapsed
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Time getElapsedTime() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the clock is running
    ///
    /// \return True if the clock is running, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isRunning() const;

    ////////////////////////////////////////////////////////////
    /// \brief Start the clock
    ///
    /// \see stop
    ///
    ////////////////////////////////////////////////////////////
    void start();

    ////////////////////////////////////////////////////////////
    /// \brief Stop the clock
    ///
    /// \see start
    ///
    ////////////////////////////////////////////////////////////
    void stop();

    ////////////////////////////////////////////////////////////
    /// \brief Restart the clock
    ///
    /// This function puts the time counter back to zero, returns
    /// the elapsed time, and leaves the clock in a running state.
    ///
    /// \return Time elapsed
    ///
    /// \see reset
    ///
    ////////////////////////////////////////////////////////////
    Time restart();

    ////////////////////////////////////////////////////////////
    /// \brief Reset the clock
    ///
    /// This function puts the time counter back to zero, returns
    /// the elapsed time, and leaves the clock in a paused state.
    ///
    /// \return Time elapsed
    ///
    /// \see restart
    ///
    ////////////////////////////////////////////////////////////
    Time reset();

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ///////////////////////////////////////////////////////////
    struct Impl;
    priv::InPlacePImpl<Impl, 16> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Clock
/// \ingroup system
///
/// sf::Clock is a lightweight class for measuring time.
///
/// It provides the most precise time that the underlying
/// OS can achieve (generally microseconds or nanoseconds).
/// It also ensures monotonicity, which means that the returned
/// time can never go backward, even if the system time is
/// changed.
///
/// Usage example:
/// \code
/// sf::Clock clock;
/// ...
/// Time time1 = clock.getElapsedTime();
/// ...
/// Time time2 = clock.restart();
/// ...
/// Time time3 = clock.reset();
/// \endcode
///
/// The sf::Time value returned by the clock can then be
/// converted to a number of seconds, milliseconds or even
/// microseconds.
///
/// \see sf::Time
///
////////////////////////////////////////////////////////////
