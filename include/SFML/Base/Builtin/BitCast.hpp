#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_bit_cast)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_BIT_CAST(type, ...) __builtin_bit_cast(type, __VA_ARGS__)

#else

    #include <bit>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_BIT_CAST(type, ...) ::std::bit_cast<type>(__VA_ARGS__)

#endif
