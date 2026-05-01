#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Portable wrapper for the C99 `restrict` qualifier
///
/// Tells the compiler that two pointer parameters do not alias each
/// other, which can unlock vectorization. Expands to `__restrict__`
/// on GCC/Clang, `__restrict` on MSVC, and nothing elsewhere.
///
////////////////////////////////////////////////////////////


#if defined(__GNUC__) || defined(__clang__)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_RESTRICT __restrict__

#elif defined(_MSC_VER)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_RESTRICT __restrict

#else

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_RESTRICT

#endif
