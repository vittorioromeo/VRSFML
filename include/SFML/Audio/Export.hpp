#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


////////////////////////////////////////////////////////////
// Portable import / export macros
////////////////////////////////////////////////////////////
#if defined(SFML_AUDIO_EXPORTS)

    #define SFML_AUDIO_API SFML_API_EXPORT

#else

    #define SFML_AUDIO_API SFML_API_IMPORT

#endif
