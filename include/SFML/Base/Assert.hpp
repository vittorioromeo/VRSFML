#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


#ifdef SFML_DEBUG

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
[[noreturn, gnu::cold, gnu::noinline]] void assertFailure(const char* code, const char* file, int line);

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ASSERT(...)                                                  \
        do                                                                         \
        {                                                                          \
            if (!static_cast<bool>(__VA_ARGS__)) [[unlikely]]                      \
                ::sf::base::priv::assertFailure(#__VA_ARGS__, __FILE__, __LINE__); \
                                                                                   \
        } while (false)

#else

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ASSERT(...)

#endif
