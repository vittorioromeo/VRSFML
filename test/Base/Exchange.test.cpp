#include "Tst/Tst.hpp"

#include "Zancle/Base/Exchange.hpp"


TEST_CASE("[Base] Base/Algorithm.hpp")
{
    SECTION("Exchange")
    {
        int a = 0;
        int b = 1;

        CHECK(za::exchange(a, b) == 0);
        CHECK(a == 1);
    }
}
