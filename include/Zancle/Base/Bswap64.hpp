#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_bswap64)

    ////////////////////////////////////////////////////////////
    #define ZA_BSWAP64 __builtin_bswap64

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define ZA_BSWAP64 ::std::byteswap

#endif
