#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#if defined(SFML_SYSTEM_WINDOWS)
#include <basetsd.h>
#endif


namespace sf
{
////////////////////////////////////////////////////////////
// Low-level socket handle type, specific to each platform
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)

using SocketHandle = UINT_PTR;

#else

using SocketHandle = int;

#endif

} // namespace sf
