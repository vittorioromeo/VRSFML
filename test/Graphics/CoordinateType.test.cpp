#include "SFML/Graphics/CoordinateType.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>


TEST_CASE("[Graphics] sf::CoordinateType")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::CoordinateType));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::CoordinateType));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::CoordinateType));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::CoordinateType));
    }
}
