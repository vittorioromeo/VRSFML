#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


// clang-format off
#if __has_builtin(__builtin_is_constant_evaluated)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONSTANT_EVALUATED() __builtin_is_constant_evaluated()

#else

    #include <type_traits>

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_IS_CONSTANT_EVALUATED() ::std::is_constant_evaluated()

#endif
