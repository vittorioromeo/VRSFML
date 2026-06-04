#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_unreachable)

    ////////////////////////////////////////////////////////////
    #define ZB_UNREACHABLE() __builtin_unreachable()

#elif __has_builtin(__assume)

    ////////////////////////////////////////////////////////////
    #define ZB_UNREACHABLE() __assume(false)

#else

    #include "ZancleBase/Abort.hpp"
    #include "ZancleBase/Assert.hpp"

    ////////////////////////////////////////////////////////////
    #define ZB_UNREACHABLE()  \
        do                    \
        {                     \
            ZA_ASSERT(false); \
            ::zb::abort();    \
        } while (false)

#endif


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
