#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_memset)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMSET __builtin_memset

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMSET ::std::memset

#endif
