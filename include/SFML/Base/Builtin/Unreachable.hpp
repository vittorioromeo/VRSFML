#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Portable "unreachable" hint
///
/// Marks a code path as unreachable so that the optimizer can omit
/// any safety checks leading to it. Reaching it at run time is
/// undefined behavior in release mode; on compilers that lack a
/// dedicated builtin it falls back to a debug-only assertion.
///
////////////////////////////////////////////////////////////


#if __has_builtin(__builtin_unreachable)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNREACHABLE() __builtin_unreachable()

#elif __has_builtin(__assume)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNREACHABLE() __assume(false)

#else

    #include "SFML/Base/Assert.hpp"

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNREACHABLE() SFML_ASSERT(false)

#endif
