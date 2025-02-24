#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __cplusplus >= 202'302L

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ASSUME(...) [[assume(__VA_ARGS__)]]

#elif __has_builtin(__builtin_assume)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ASSUME(...) __builtin_assume(__VA_ARGS__)

#elif __has_builtin(__assume)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ASSUME(...) __assume(__VA_ARGS__)

#else

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ASSUME(...)

#endif
