#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_strncmp)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRNCMP __builtin_strncmp

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRNCMP ::std::strncmp

#endif
