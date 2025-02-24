#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_strstr)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRSTR __builtin_strstr

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRSTR ::std::strstr

#endif
