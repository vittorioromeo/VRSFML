#include "Tst/Tst.hpp"

#include "ZancleBase/Exchange.hpp"


TEST_CASE("[Base] Base/Algorithm.hpp")
{
    SECTION("Exchange")
    {
        int a = 0;
        int b = 1;

        CHECK(zb::exchange(a, b) == 0);
        CHECK(a == 1);
    }
}
