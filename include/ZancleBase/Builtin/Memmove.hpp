#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_memmove)

    ////////////////////////////////////////////////////////////
    #define ZB_MEMMOVE __builtin_memmove

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_MEMMOVE ::std::memmove

#endif
