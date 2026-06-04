#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_strstr)

    ////////////////////////////////////////////////////////////
    #define ZB_STRSTR __builtin_strstr

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_STRSTR ::std::strstr

#endif
