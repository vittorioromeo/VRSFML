#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"
#include "SFML/System/SignalErrHandler.hpp"

#include <csignal>


namespace sf::priv
{
////////////////////////////////////////////////////////////
void installSignalErrHandler() noexcept
{
    std::signal(SIGSEGV, [](int) { priv::err() << "FATAL SIGNAL: SIGSEGV"; });
    std::signal(SIGILL, [](int) { priv::err() << "FATAL SIGNAL: SIGILL"; });
    std::signal(SIGABRT, [](int) { priv::err() << "FATAL SIGNAL: SIGABRT"; });
    std::signal(SIGFPE, [](int) { priv::err() << "FATAL SIGNAL: SIGFPE"; });
}

} // namespace sf::priv
