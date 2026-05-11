#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_isnan)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ISNAN __builtin_isnan

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ISNAN ::std::isnan

#endif
