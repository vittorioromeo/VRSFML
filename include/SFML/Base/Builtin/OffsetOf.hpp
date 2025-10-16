#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_offsetof)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_OFFSETOF __builtin_offsetof

#else

    #include <cstddef>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_OFFSETOF offsetof

#endif
