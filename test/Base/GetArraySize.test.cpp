#include "Tst/Tst.hpp"

#include "Zancle/Base/GetArraySize.hpp"


TEST_CASE("[Base] Base/Algorithm.hpp")
{
    SECTION("Get Array Size")
    {
        const int values[]{0, 1, 2, 3, 4, 5, 6, 7};
        CHECK(za::getArraySize(values) == 8);
    }
}
