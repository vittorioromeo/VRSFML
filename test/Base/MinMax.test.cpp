#include "SFML/Base/MinMax.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/Algorithm.hpp")
{
    SECTION("Min/Max")
    {
        const int a = 10;
        const int b = -10;

        CHECK(&sf::base::min(a, b) == &b);
        CHECK(&sf::base::max(a, b) == &a);

        const int c = 10;

        CHECK(&sf::base::min(a, c) == &a);
        CHECK(&sf::base::max(a, c) == &a);
    }
}
