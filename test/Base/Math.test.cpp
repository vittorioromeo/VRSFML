#include "Tst/Tst.hpp"

#include "ZancleBase/Math/Frexp.hpp"
#include "ZancleBase/Math/Ldexp.hpp"


TEST_CASE("[Base] Base/Math.hpp")
{
    SECTION("Frexp")
    {
        int exponent{};

        CHECK(ZB_MATH_FREXPF(0.f, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(ZB_MATH_FREXP(0., &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(ZB_MATH_FREXPL(0.l, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(zb::frexp(0.f, &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(zb::frexp(0., &exponent) == 0.f);
        CHECK(exponent == 0);

        CHECK(zb::frexp(0.l, &exponent) == 0.f);
        CHECK(exponent == 0);
    }

    SECTION("Ldexp")
    {
        int exponent{};

        CHECK(ZB_MATH_LDEXPF(0.f, exponent) == 0.f);
        CHECK(ZB_MATH_LDEXP(0., exponent) == 0.f);
        CHECK(ZB_MATH_LDEXPL(0.l, exponent) == 0.f);
        CHECK(zb::ldexp(0.f, exponent) == 0.f);
        CHECK(zb::ldexp(0., exponent) == 0.f);
        CHECK(zb::ldexp(0.l, exponent) == 0.f);
    }
}
