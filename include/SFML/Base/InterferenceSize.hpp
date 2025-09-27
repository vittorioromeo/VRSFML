#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"


#if defined(__GCC_DESTRUCTIVE_SIZE) && defined(__GCC_CONSTRUCTIVE_SIZE)

namespace sf::base
{
////////////////////////////////////////////////////////////
enum : SizeT
{
    hardwareDestructiveInterferenceSize  = __GCC_DESTRUCTIVE_SIZE,
    hardwareConstructiveInterferenceSize = __GCC_CONSTRUCTIVE_SIZE,
};

} // namespace sf::base

#elif defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)

namespace sf::base
{
////////////////////////////////////////////////////////////
enum : SizeT
{
    hardwareDestructiveInterferenceSize  = 64u,
    hardwareConstructiveInterferenceSize = 64u,
};

} // namespace sf::base

#else

    #include <new>

namespace sf::base
{
////////////////////////////////////////////////////////////
enum : SizeT
{
    hardwareDestructiveInterferenceSize  = std::hardware_destructive_interference_size,
    hardwareConstructiveInterferenceSize = std::hardware_constructive_interference_size,
};

} // namespace sf::base

#endif
