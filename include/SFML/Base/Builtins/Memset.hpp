#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_memset)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMSET __builtin_memset

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMSET ::std::memset

#endif
