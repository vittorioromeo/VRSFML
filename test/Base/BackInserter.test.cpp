
#include "SFML/Base/BackInserter.hpp"

#include "SFML/Base/Algorithm/Copy.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/BackInserter.hpp")
{
    SECTION("Back Inserter")
    {
        const int             values[]{0, 1, 2, 3};
        sf::base::Vector<int> target{-1};

        sf::base::copy(values, values + 4, sf::base::BackInserter{target});

        CHECK(target[0] == -1);
        CHECK(target[1] == 0);
        CHECK(target[2] == 1);
        CHECK(target[3] == 2);
        CHECK(target[4] == 3);
    }
}
