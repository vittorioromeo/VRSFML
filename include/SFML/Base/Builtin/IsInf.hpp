#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_isinf)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ISINF __builtin_isinf

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ISINF ::std::isinf

#endif
