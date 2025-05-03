#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_offsetof)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_OFFSETOF __builtin_offsetof

#else

    #include <cstddef>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_OFFSETOF offsetof

#endif
