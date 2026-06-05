#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_popcountll)

    ////////////////////////////////////////////////////////////
    #define ZA_POPCOUNTLL __builtin_popcountll

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define ZA_POPCOUNTLL ::std::popcount

#endif
