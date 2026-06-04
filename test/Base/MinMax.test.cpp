#include "Tst/Tst.hpp"

#include "ZancleBase/MinMax.hpp"


TEST_CASE("[Base] Base/MinMax.hpp")
{
    SECTION("Min/Max")
    {
        const int a = 10;
        const int b = -10;

        CHECK(&zb::min(a, b) == &b);
        CHECK(&zb::max(a, b) == &a);

        const int c = 10;

        CHECK(&zb::min(a, c) == &a);
        CHECK(&zb::max(a, c) == &a);
    }
}
