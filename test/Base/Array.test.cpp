#include "Tst/Tst.hpp"

#include "ZancleBase/Array.hpp"

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


namespace
{
namespace ArrayTest // for unity builds
{
////////////////////////////////////////////////////////////
struct NonTrivial
{
    static inline int si{};

    NonTrivial(const NonTrivial&) : i(si)
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    ~NonTrivial()
    {
    }

    int& i; // NOLINT(cppcoreguidelines-use-default-member-init, modernize-use-default-member-init)
};


////////////////////////////////////////////////////////////
struct NonTrivialButRelocatable
{
    enum : bool
    {
        enableTrivialRelocation = true
    };

    static inline int si{};

    NonTrivialButRelocatable(const NonTrivialButRelocatable&) : i(si)
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    ~NonTrivialButRelocatable()
    {
    }

    int& i; // NOLINT(cppcoreguidelines-use-default-member-init, modernize-use-default-member-init)
};

} // namespace ArrayTest
} // namespace


TEST_CASE("[Base] Base/Array.hpp")
{
    SECTION("Type traits")
    {
        using namespace ArrayTest;

        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Array<int, 5>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Array<int, 5>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Array<int, 5>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Array<int, 5>));

        STATIC_CHECK(ZB_IS_TRIVIAL(zb::Array<int, 5>)); // because of member initializers
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(zb::Array<int, 5>));
        STATIC_CHECK(ZB_IS_AGGREGATE(zb::Array<int, 5>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(zb::Array<int, 5>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Array<int, 5>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(zb::Array<int, 5>, zb::Array<int, 5>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::Array<int, 5>));


        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Array<NonTrivial, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Array<NonTrivial, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Array<NonTrivial, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Array<NonTrivial, 5>));

        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::Array<NonTrivial, 5>)); // because of member initializers
        STATIC_CHECK(!ZB_IS_STANDARD_LAYOUT(zb::Array<NonTrivial, 5>));
        STATIC_CHECK(ZB_IS_AGGREGATE(zb::Array<NonTrivial, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPYABLE(zb::Array<NonTrivial, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Array<NonTrivial, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_ASSIGNABLE(zb::Array<NonTrivial, 5>, zb::Array<NonTrivial, 5>));

        STATIC_CHECK(!ZB_IS_TRIVIALLY_RELOCATABLE(zb::Array<NonTrivial, 5>));


        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Array<NonTrivialButRelocatable, 5>));

        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::Array<NonTrivialButRelocatable, 5>)); // because of member initializers
        STATIC_CHECK(!ZB_IS_STANDARD_LAYOUT(zb::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(ZB_IS_AGGREGATE(zb::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPYABLE(zb::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Array<NonTrivialButRelocatable, 5>));
        STATIC_CHECK(
            !ZB_IS_TRIVIALLY_ASSIGNABLE(zb::Array<NonTrivialButRelocatable, 5>, zb::Array<NonTrivialButRelocatable, 5>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::Array<NonTrivialButRelocatable, 5>));
    }
}
