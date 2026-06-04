#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_offsetof)

    ////////////////////////////////////////////////////////////
    #define ZB_OFFSETOF __builtin_offsetof

#else

    #include <cstddef>

    ////////////////////////////////////////////////////////////
    #define ZB_OFFSETOF offsetof

#endif
