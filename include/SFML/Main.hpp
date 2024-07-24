#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Config.hpp>


#if defined(SFML_SYSTEM_IOS)

// On iOS, we have no choice but to have our own main,
// so we need to rename the user one and call it later
#define main sfmlMain

#endif
