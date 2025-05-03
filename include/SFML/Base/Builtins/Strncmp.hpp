#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_strncmp)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRNCMP __builtin_strncmp

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRNCMP ::std::strncmp

#endif
