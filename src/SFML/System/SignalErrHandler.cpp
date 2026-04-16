// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/SignalErrHandler.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"

#include <csignal>


namespace sf::priv
{
////////////////////////////////////////////////////////////
void installSignalErrHandler() noexcept
{
    std::signal(SIGSEGV,
                [](int)
    {
        priv::err() << "FATAL SIGNAL: SIGSEGV";
        base::abort();
    });

    std::signal(SIGILL,
                [](int)
    {
        priv::err() << "FATAL SIGNAL: SIGILL";
        base::abort();
    });

    std::signal(SIGABRT,
                [](int)
    {
        priv::err() << "FATAL SIGNAL: SIGABRT";
        base::abort();
    });

    std::signal(SIGFPE,
                [](int)
    {
        priv::err() << "FATAL SIGNAL: SIGFPE";
        base::abort();
    });
}

} // namespace sf::priv
