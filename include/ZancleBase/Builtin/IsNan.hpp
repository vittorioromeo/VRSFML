#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_isnan)

    ////////////////////////////////////////////////////////////
    #define ZB_ISNAN __builtin_isnan

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define ZB_ISNAN ::std::isnan

#endif
