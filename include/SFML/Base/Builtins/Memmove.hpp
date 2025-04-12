#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_memmove)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMMOVE __builtin_memmove

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMMOVE ::std::memmove

#endif
