#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_launder)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_LAUNDER __builtin_launder

#else

    #include <new>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_LAUNDER ::std::launder

#endif

////////////////////////////////////////////////////////////
#define SFML_BASE_LAUNDER_CAST(type, buffer) SFML_BASE_LAUNDER(reinterpret_cast<type>(buffer))
