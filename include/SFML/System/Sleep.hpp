#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>


namespace sf
{
class Time;

////////////////////////////////////////////////////////////
/// \ingroup system
/// \brief Make the current thread sleep for a given duration
///
/// sf::sleep is the best way to block a program or one of its
/// threads, as it doesn't consume any CPU power. Compared to
/// the standard std::this_thread::sleep_for function, this
/// one provides more accurate sleeping time thanks to some
/// platform-specific tweaks.
///
/// sf::sleep only guarantees millisecond precision. Sleeping
/// for a duration less than 1 millisecond is prone to result
/// in the actual sleep duration being less than what is
/// requested.
///
/// \param duration Time to sleep
///
////////////////////////////////////////////////////////////
void SFML_SYSTEM_API sleep(Time duration);

} // namespace sf
