// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Optional.hpp"

#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/Base/StackTrace.hpp"

#include <cstdio>

#if !defined(SFML_SYSTEM_EMSCRIPTEN) && defined(__EXCEPTIONS)
    #define SFML_BASE_OPTIONAL_USE_EXCEPTIONS
#endif

#ifndef SFML_BASE_OPTIONAL_USE_EXCEPTIONS
    #include "SFML/Base/Abort.hpp"
#endif

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void throwIfNotEngaged()
{
    std::puts("\n[[SFML OPTIONAL FAILURE]]: not engaged!");
    printStackTrace();

#ifdef SFML_BASE_OPTIONAL_USE_EXCEPTIONS
    throw BadOptionalAccess{};
#else
    base::abort();
#endif
}

} // namespace sf::base::priv
