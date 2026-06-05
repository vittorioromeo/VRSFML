
#include "Tst/Tst.hpp"

#include "Zancle/Container/BackInserter.hpp"

#include "Zancle/Algorithm/Copy.hpp"
#include "Zancle/Container/Vector.hpp"


TEST_CASE("[Base] Base/BackInserter.hpp")
{
    SECTION("Back Inserter")
    {
        const int       values[]{0, 1, 2, 3};
        za::Vector<int> target{-1};

        za::copy(values, values + 4, za::BackInserter{target});

        CHECK(target[0] == -1);
        CHECK(target[1] == 0);
        CHECK(target[2] == 1);
        CHECK(target[3] == 2);
        CHECK(target[4] == 3);
    }
}
