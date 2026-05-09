#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_ctzll)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_CTZLL __builtin_ctzll

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_CTZLL ::std::countr_zero

#endif
