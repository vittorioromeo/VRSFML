#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


#if defined(SFML_SYSTEM_IOS)

    // On iOS, we have no choice but to have our own main,
    // so we need to rename the user one and call it later
    #define main sfmlMain

#endif
