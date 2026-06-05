#include "Tst/Tst.hpp"

#include "Zancle/Math/MinMax.hpp"


TEST_CASE("[Base] Base/MinMax.hpp")
{
    SECTION("Min/Max")
    {
        const int a = 10;
        const int b = -10;

        CHECK(&za::min(a, b) == &b);
        CHECK(&za::max(a, b) == &a);

        const int c = 10;

        CHECK(&za::min(a, c) == &a);
        CHECK(&za::max(a, c) == &a);
    }
}
