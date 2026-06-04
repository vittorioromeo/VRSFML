#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"

#if defined(ZA_SYSTEM_WINDOWS)
    #include <basetsd.h>
#endif


namespace za
{
////////////////////////////////////////////////////////////
// Low-level socket handle type, specific to each platform
////////////////////////////////////////////////////////////
#if defined(ZA_SYSTEM_WINDOWS)

using SocketHandle = UINT_PTR;

#else

using SocketHandle = int;

#endif

} // namespace za
