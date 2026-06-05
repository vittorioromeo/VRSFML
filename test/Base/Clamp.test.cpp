#include "Tst/Tst.hpp"

#include "Zancle/Math/Clamp.hpp"


TEST_CASE("[Base] Base/Clamp.hpp")
{
    SECTION("Clamp")
    {
        CHECK(za::clamp(5, 0, 10) == 5);
        CHECK(za::clamp(15, 0, 10) == 10);
        CHECK(za::clamp(-15, 0, 10) == 0);
    }
}
