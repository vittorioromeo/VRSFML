#include "SFML/Base/GetArraySize.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/Algorithm.hpp")
{
    SECTION("Get Array Size")
    {
        const int values[]{0, 1, 2, 3, 4, 5, 6, 7};
        CHECK(sf::base::getArraySize(values) == 8);
    }
}
