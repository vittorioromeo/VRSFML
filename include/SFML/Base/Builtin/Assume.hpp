#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


// clang-format off
#if __cplusplus >= 202302L
// clang-format on

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
