#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_memcpy)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMCPY __builtin_memcpy

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMCPY ::std::memcpy

#endif
