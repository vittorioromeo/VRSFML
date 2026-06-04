#include "Tst/Tst.hpp"

#include "ZancleBase/UniquePtr.hpp"

#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyRelocatable.hpp"


TEST_CASE("[Base] Base/UniquePtr.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::UniquePtr<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::UniquePtr<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::UniquePtr<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::UniquePtr<int>));

        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::UniquePtr<int>)); // because of member initializers
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(zb::UniquePtr<int>));
        STATIC_CHECK(!ZB_IS_AGGREGATE(zb::UniquePtr<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPYABLE(zb::UniquePtr<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::UniquePtr<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_ASSIGNABLE(zb::UniquePtr<int>, zb::UniquePtr<int>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::UniquePtr<int>));
    }
}
