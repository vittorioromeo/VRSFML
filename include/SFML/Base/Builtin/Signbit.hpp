#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_signbit)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_SIGNBIT __builtin_signbit

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_SIGNBIT ::std::signbit

#endif
