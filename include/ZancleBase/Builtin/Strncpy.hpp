#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_strncpy)

    ////////////////////////////////////////////////////////////
    #define ZB_STRNCPY __builtin_strncpy

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_STRNCPY ::std::strncpy

#endif
