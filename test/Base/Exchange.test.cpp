#include "Tst/Tst.hpp"

#include "SFML/Base/Exchange.hpp"


TEST_CASE("[Base] Base/Algorithm.hpp")
{
    SECTION("Exchange")
    {
        int a = 0;
        int b = 1;

        CHECK(sf::base::exchange(a, b) == 0);
        CHECK(a == 1);
    }
}
