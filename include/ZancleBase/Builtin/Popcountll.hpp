#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_popcountll)

    ////////////////////////////////////////////////////////////
    #define ZB_POPCOUNTLL __builtin_popcountll

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define ZB_POPCOUNTLL ::std::popcount

#endif
