// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Err/SignalErrHandler.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Base/Abort.hpp"

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
        za::abort();
    });

    std::signal(SIGILL,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGILL");
        za::abort();
    });

    std::signal(SIGABRT,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGABRT");
        za::abort();
    });

    std::signal(SIGFPE,
                [](int)
    {
        priv::errMsg("FATAL SIGNAL: SIGFPE");
        za::abort();
    });
}

} // namespace za::priv
