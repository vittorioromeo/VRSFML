#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_strncpy)

    ////////////////////////////////////////////////////////////
    #define ZA_STRNCPY __builtin_strncpy

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZA_STRNCPY ::std::strncpy

#endif
