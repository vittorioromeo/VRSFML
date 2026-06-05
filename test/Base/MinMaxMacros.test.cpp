#include "Tst/Tst.hpp"

#include "Zancle/Math/MinMaxMacros.hpp"


TEST_CASE("[Base] Base/MinMaxMacros.hpp")
{
    SECTION("Min/Max")
    {
        const int a = 10;
        const int b = -10;

        CHECK(&ZA_MIN(a, b) == &b);
        CHECK(&ZA_MAX(a, b) == &a);

        const int c = 10;

        CHECK(&ZA_MIN(a, c) == &a);
        CHECK(&ZA_MAX(a, c) == &a);
    }
}
