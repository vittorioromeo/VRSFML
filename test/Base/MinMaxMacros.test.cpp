#include "Tst/Tst.hpp"

#include "ZancleBase/MinMaxMacros.hpp"


TEST_CASE("[Base] Base/MinMaxMacros.hpp")
{
    SECTION("Min/Max")
    {
        const int a = 10;
        const int b = -10;

        CHECK(&ZB_MIN(a, b) == &b);
        CHECK(&ZB_MAX(a, b) == &a);

        const int c = 10;

        CHECK(&ZB_MIN(a, c) == &a);
        CHECK(&ZB_MAX(a, c) == &a);
    }
}
