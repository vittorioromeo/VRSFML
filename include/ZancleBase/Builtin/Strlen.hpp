#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_strlen)

    ////////////////////////////////////////////////////////////
    #define ZB_STRLEN __builtin_strlen

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_STRLEN ::std::strlen

#endif
