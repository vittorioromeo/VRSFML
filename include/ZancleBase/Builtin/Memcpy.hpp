#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_memcpy)

    ////////////////////////////////////////////////////////////
    #define ZB_MEMCPY __builtin_memcpy

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_MEMCPY ::std::memcpy

#endif
