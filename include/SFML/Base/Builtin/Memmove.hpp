#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_memmove)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMMOVE __builtin_memmove

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MEMMOVE ::std::memmove

#endif
