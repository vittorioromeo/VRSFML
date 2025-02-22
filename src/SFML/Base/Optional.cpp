#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StackTrace.hpp"

#include <cstdio>

#ifndef __EXCEPTIONS
#include "SFML/Base/Abort.hpp"
#endif

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void throwIfNotEngaged()
{
    std::puts("\n[[SFML OPTIONAL FAILURE]]: not engaged!");
    printStackTrace();

#ifdef __EXCEPTIONS
    throw BadOptionalAccess{};
#else
    base::abort();
#endif
}

} // namespace sf::base::priv
