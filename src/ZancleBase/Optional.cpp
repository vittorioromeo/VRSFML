// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp" // IWYU pragma: keep
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/StackTrace.hpp"

#include <cstdio>

#if !defined(ZA_SYSTEM_EMSCRIPTEN) && defined(__EXCEPTIONS)
    #define ZB_OPTIONAL_USE_EXCEPTIONS
#endif

#ifndef ZB_OPTIONAL_USE_EXCEPTIONS
    #include "ZancleBase/Abort.hpp"
#endif

namespace zb::priv
{
////////////////////////////////////////////////////////////
void throwIfNotEngaged()
{
    std::puts("\n[[ZANCLE OPTIONAL FAILURE]]: not engaged!");
    printStackTrace();

#ifdef ZB_OPTIONAL_USE_EXCEPTIONS
    throw BadOptionalAccess{};
#else
    zb::abort();
#endif
}

} // namespace zb::priv
