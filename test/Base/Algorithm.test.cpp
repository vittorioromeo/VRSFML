#include "SFML/Base/Algorithm.hpp"

#include <Doctest.hpp>

#include <vector>


TEST_CASE("[Base] Base/Algorithm.hpp")
{
    SECTION("Copy")
    {
        const int values[]{0, 1, 2, 3};
        int       target[4];

        CHECK(sf::base::copy(values, values + 4, target) == target + 4);

        CHECK(target[0] == 0);
        CHECK(target[1] == 1);
        CHECK(target[2] == 2);
        CHECK(target[3] == 3);
    }

    SECTION("Find/FindIf/AnyOf")
    {
        const int values[]{0, 1, 2, 3, 4, 5, 6, 7};

        CHECK(sf::base::find(values, values + 8, 4) == &values[4]);
        CHECK(sf::base::find(values, values + 8, 5) == &values[5]);
        CHECK(sf::base::find(values, values + 8, 400) == values + 8);

        CHECK(sf::base::findIf(values, values + 8, [](const int x) { return x == 4; }) == &values[4]);
        CHECK(sf::base::findIf(values, values + 8, [](const int x) { return x == 5; }) == &values[5]);
        CHECK(sf::base::findIf(values, values + 8, [](const int x) { return x == 400; }) == values + 8);

        CHECK(sf::base::anyOf(values, values + 8, [](const int x) { return x == 4; }));
        CHECK(sf::base::anyOf(values, values + 8, [](const int x) { return x == 5; }));
        CHECK(!sf::base::anyOf(values, values + 8, [](const int x) { return x == 400; }));
    }

    SECTION("Get Array Size")
    {
        const int values[]{0, 1, 2, 3, 4, 5, 6, 7};
        CHECK(sf::base::getArraySize(values) == 8);
    }

    SECTION("Back Inserter")
    {
        const int        values[]{0, 1, 2, 3};
        std::vector<int> target{-1};

        sf::base::copy(values, values + 4, sf::base::BackInserter{target});

        CHECK(target[0] == -1);
        CHECK(target[1] == 0);
        CHECK(target[2] == 1);
        CHECK(target[3] == 2);
        CHECK(target[4] == 3);
    }


    SECTION("Exchange")
    {
        int a = 0;
        int b = 1;

        CHECK(sf::base::exchange(a, b) == 0);
        CHECK(a == 1);
    }
}
