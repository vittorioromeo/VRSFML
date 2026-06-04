#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_clzll)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_CLZLL __builtin_clzll

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_CLZLL ::std::countl_zero

#endif
