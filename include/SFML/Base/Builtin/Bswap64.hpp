#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_bswap64)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_BSWAP64 __builtin_bswap64

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_BSWAP64 ::std::byteswap

#endif
