#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_memcmp)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMCMP __builtin_memcmp

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMCMP ::std::memcmp

#endif
