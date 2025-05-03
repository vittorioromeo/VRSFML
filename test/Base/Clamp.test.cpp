#include "SFML/Base/Clamp.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/Clamp.hpp")
{
    SECTION("Clamp")
    {
        CHECK(sf::base::clamp(5, 0, 10) == 5);
        CHECK(sf::base::clamp(15, 0, 10) == 10);
        CHECK(sf::base::clamp(-15, 0, 10) == 0);
    }
}
