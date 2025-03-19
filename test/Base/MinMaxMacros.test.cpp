#include "SFML/Base/MinMaxMacros.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/MinMaxMacros.hpp")
{
    SECTION("Min/Max")
    {
        const int a = 10;
        const int b = -10;

        CHECK(&SFML_BASE_MIN(a, b) == &b);
        CHECK(&SFML_BASE_MAX(a, b) == &a);

        const int c = 10;

        CHECK(&SFML_BASE_MIN(a, c) == &a);
        CHECK(&SFML_BASE_MAX(a, c) == &a);
    }
}
