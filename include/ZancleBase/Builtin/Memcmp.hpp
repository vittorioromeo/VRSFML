#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_memcmp)

    ////////////////////////////////////////////////////////////
    #define ZB_MEMCMP __builtin_memcmp

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_MEMCMP ::std::memcmp

#endif
