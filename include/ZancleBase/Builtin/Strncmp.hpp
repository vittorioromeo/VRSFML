#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_strncmp)

    ////////////////////////////////////////////////////////////
    #define ZB_STRNCMP __builtin_strncmp

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_STRNCMP ::std::strncmp

#endif
