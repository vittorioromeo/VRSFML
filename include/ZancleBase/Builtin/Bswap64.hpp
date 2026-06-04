#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_bswap64)

    ////////////////////////////////////////////////////////////
    #define ZB_BSWAP64 __builtin_bswap64

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define ZB_BSWAP64 ::std::byteswap

#endif
