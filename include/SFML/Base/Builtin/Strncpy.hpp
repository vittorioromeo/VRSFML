#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_strncpy)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRNCPY __builtin_strncpy

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRNCPY ::std::strncpy

#endif
