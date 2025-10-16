#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__builtin_is_constant_evaluated)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONSTANT_EVALUATED() __builtin_is_constant_evaluated()

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONSTANT_EVALUATED() ::std::is_constant_evaluated()

#endif
