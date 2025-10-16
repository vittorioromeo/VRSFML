#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_strcmp)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRCMP __builtin_strcmp

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRCMP ::std::strcmp

#endif
