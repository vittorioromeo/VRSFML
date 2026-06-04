#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_ctzll)

    ////////////////////////////////////////////////////////////
    #define ZB_CTZLL __builtin_ctzll

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define ZB_CTZLL ::std::countr_zero

#endif
