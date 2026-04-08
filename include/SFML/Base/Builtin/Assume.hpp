#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Portable `[[assume(expr)]]` wrapper
///
/// Tells the optimizer that an expression is true at this point so
/// that it can elide redundant checks. Prefers the C++23 attribute,
/// falls back to `__builtin_assume` (Clang) or MSVC's `__assume`,
/// and expands to nothing on other compilers.
///
/// Use sparingly: an incorrect assumption is undefined behavior.
///
////////////////////////////////////////////////////////////


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
