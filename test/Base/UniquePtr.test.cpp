#include "Tst/Tst.hpp"

#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Trait/IsAggregate.hpp"
#include "Zancle/Trait/IsStandardLayout.hpp"
#include "Zancle/Trait/IsTrivial.hpp"
#include "Zancle/Trait/IsTriviallyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyConstructible.hpp"
#include "Zancle/Trait/IsTriviallyCopyable.hpp"
#include "Zancle/Trait/IsTriviallyDestructible.hpp"
#include "Zancle/Trait/IsTriviallyMoveAssignable.hpp"
#include "Zancle/Trait/IsTriviallyMoveConstructible.hpp"
#include "Zancle/Trait/IsTriviallyRelocatable.hpp"


TEST_CASE("[Base] Base/UniquePtr.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::UniquePtr<int>));
        STATIC_CHECK(!ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::UniquePtr<int>));
        STATIC_CHECK(!ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::UniquePtr<int>));
        STATIC_CHECK(!ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::UniquePtr<int>));

        STATIC_CHECK(!ZA_IS_TRIVIAL(za::UniquePtr<int>)); // because of member initializers
        STATIC_CHECK(ZA_IS_STANDARD_LAYOUT(za::UniquePtr<int>));
        STATIC_CHECK(!ZA_IS_AGGREGATE(za::UniquePtr<int>));
        STATIC_CHECK(!ZA_IS_TRIVIALLY_COPYABLE(za::UniquePtr<int>));
        STATIC_CHECK(!ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::UniquePtr<int>));
        STATIC_CHECK(!ZA_IS_TRIVIALLY_ASSIGNABLE(za::UniquePtr<int>, za::UniquePtr<int>));

        STATIC_CHECK(ZA_IS_TRIVIALLY_RELOCATABLE(za::UniquePtr<int>));
    }
}
