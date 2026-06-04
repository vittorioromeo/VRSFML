// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/SignalErrHandler.hpp"

#include "Zancle/System/Err.hpp"

#include "ZancleBase/Abort.hpp"

#include <csignal>


namespace za::priv
{
////////////////////////////////////////////////////////////
void installSignalErrHandler() noexcept
{
    std::signal(SIGSEGV,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGSEGV");
        base::abort();
    });

    std::signal(SIGILL,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGILL");
        base::abort();
    });

    std::signal(SIGABRT,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGABRT");
        base::abort();
    });

    std::signal(SIGFPE,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGFPE");
        base::abort();
    });
}

} // namespace za::priv
