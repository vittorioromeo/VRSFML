// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Err.hpp"
#include "Zancle/System/SignalErrHandler.hpp"
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
        zb::abort();
    });

    std::signal(SIGILL,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGILL");
        zb::abort();
    });

    std::signal(SIGABRT,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGABRT");
        zb::abort();
    });

    std::signal(SIGFPE,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGFPE");
        zb::abort();
    });
}

} // namespace za::priv
