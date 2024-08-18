#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StackTrace.hpp"

#include <cstdio>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void throwIfNotEngaged()
{
    std::puts("\n[[SFML OPTIONAL FAILURE]]: not engaged!");
    printStackTrace();

    throw BadOptionalAccess{};
}

} // namespace sf::base::priv
