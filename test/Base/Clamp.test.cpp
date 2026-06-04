#include "Tst/Tst.hpp"

#include "ZancleBase/Clamp.hpp"


TEST_CASE("[Base] Base/Clamp.hpp")
{
    SECTION("Clamp")
    {
        CHECK(zb::clamp(5, 0, 10) == 5);
        CHECK(zb::clamp(15, 0, 10) == 10);
        CHECK(zb::clamp(-15, 0, 10) == 0);
    }
}
