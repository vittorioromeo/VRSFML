#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_clzll)

    ////////////////////////////////////////////////////////////
    #define ZB_CLZLL __builtin_clzll

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define ZB_CLZLL ::std::countl_zero

#endif
