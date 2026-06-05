#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_offsetof)

    ////////////////////////////////////////////////////////////
    #define ZA_OFFSETOF __builtin_offsetof

#else

    #include <cstddef>

    ////////////////////////////////////////////////////////////
    #define ZA_OFFSETOF offsetof

#endif
