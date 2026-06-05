#include "Tst/Tst.hpp"

#include "Zancle/Math/Frexp.hpp"
#include "Zancle/Math/Ldexp.hpp"


TEST_CASE("[Base] Base/Math.hpp")
{
    SECTION("Frexp")
    {
        int exponent{};

        CHECK(ZA_MATH_FREXPF(0.f, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(ZA_MATH_FREXP(0., &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(ZA_MATH_FREXPL(0.l, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(za::frexp(0.f, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(za::frexp(0., &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(za::frexp(0.l, &exponent) == 0.f);
        CHECK(exponent == 0);
    }

    SECTION("Ldexp")
    {
        int exponent{};

        CHECK(ZA_MATH_LDEXPF(0.f, exponent) == 0.f);
        CHECK(ZA_MATH_LDEXP(0., exponent) == 0.f);
        CHECK(ZA_MATH_LDEXPL(0.l, exponent) == 0.f);
        CHECK(za::ldexp(0.f, exponent) == 0.f);
        CHECK(za::ldexp(0., exponent) == 0.f);
        CHECK(za::ldexp(0.l, exponent) == 0.f);
    }
}
