#include "SFML/Base/Math/Frexp.hpp"
#include "SFML/Base/Math/Ldexp.hpp"

#include <Doctest.hpp>


TEST_CASE("[Base] Base/Math.hpp")
{
    SECTION("Frexp")
    {
        int exponent{};

        CHECK(SFML_BASE_MATH_FREXPF(0.f, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(SFML_BASE_MATH_FREXP(0., &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(SFML_BASE_MATH_FREXPL(0.l, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(sf::base::frexp(0.f, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(sf::base::frexp(0., &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(sf::base::frexp(0.l, &exponent) == 0.f);
        CHECK(exponent == 0);
    }

    SECTION("Ldexp")
    {
        int exponent{};

        CHECK(SFML_BASE_MATH_LDEXPF(0.f, exponent) == 0.f);
        CHECK(SFML_BASE_MATH_LDEXP(0., exponent) == 0.f);
        CHECK(SFML_BASE_MATH_LDEXPL(0.l, exponent) == 0.f);
        CHECK(sf::base::ldexp(0.f, exponent) == 0.f);
        CHECK(sf::base::ldexp(0., exponent) == 0.f);
        CHECK(sf::base::ldexp(0.l, exponent) == 0.f);
    }
}
