#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp" // IWYU pragma: keep


#ifdef ZA_DEBUG

namespace zb::priv
{
////////////////////////////////////////////////////////////
[[noreturn, gnu::cold, gnu::noinline]] void assertFailure(const char* code, const char* file, int line);

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_ASSERT(...)                                                  \
        do                                                                         \
        {                                                                          \
            if (!static_cast<bool>(__VA_ARGS__)) [[unlikely]]                      \
                ::zb::priv::assertFailure(#__VA_ARGS__, __FILE__, __LINE__); \
                                                                                   \
        } while (false)

#else

    ////////////////////////////////////////////////////////////
    #define ZB_ASSERT(...)

#endif
