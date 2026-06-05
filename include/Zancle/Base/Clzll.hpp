#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_clzll)

    ////////////////////////////////////////////////////////////
    #define ZA_CLZLL __builtin_clzll

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define ZA_CLZLL ::std::countl_zero

#endif
