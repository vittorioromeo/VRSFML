#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_strstr)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRSTR __builtin_strstr

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_STRSTR ::std::strstr

#endif
