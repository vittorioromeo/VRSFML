#include "Tst/Tst.hpp"

#include "ZancleBase/Span.hpp"

#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsConstructible.hpp"
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
#include "ZancleBase/Vector.hpp"


TEST_CASE("[Base] Base/Span.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Span<int>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Span<int>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Span<int>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Span<int>));

        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::Span<int>)); // because of member initializers
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(zb::Span<int>));
        STATIC_CHECK(!ZB_IS_AGGREGATE(zb::Span<int>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(zb::Span<int>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Span<int>));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(zb::Span<int>, zb::Span<int>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::Span<int>));
    }

    SECTION("Structured bindings")
    {
        zb::Span<int> span{nullptr, 0u};

        auto [data, size] = span;

        CHECK(data == nullptr);
        CHECK(size == 0u);
    }

    SECTION("Range constructor from zb::Vector")
    {
        zb::Vector<int> vec;
        vec.pushBack(10);
        vec.pushBack(20);
        vec.pushBack(30);

        zb::Span<int> span{vec};

        CHECK(span.data() == vec.data());
        CHECK(span.size() == 3u);
        CHECK(span[0] == 10);
        CHECK(span[1] == 20);
        CHECK(span[2] == 30);
    }

    SECTION("Range constructor const-correctness")
    {
        zb::Vector<int> vec;
        vec.pushBack(1);
        vec.pushBack(2);

        const zb::Vector<int>& cvec = vec;

        zb::Span<const int> cspan{cvec};
        CHECK(cspan.data() == cvec.data());
        CHECK(cspan.size() == 2u);

        zb::Span<const int> cspan2{vec};
        CHECK(cspan2.size() == 2u);

        // Constructing a non-const Span from a non-const range is allowed.
        STATIC_CHECK(ZB_IS_CONSTRUCTIBLE(zb::Span<int>, zb::Vector<int>&));

        // Constructing a non-const Span from a const range must be rejected.
        STATIC_CHECK(!ZB_IS_CONSTRUCTIBLE(zb::Span<int>, const zb::Vector<int>&));

        // Constructing a const Span from either is allowed.
        STATIC_CHECK(ZB_IS_CONSTRUCTIBLE(zb::Span<const int>, zb::Vector<int>&));
        STATIC_CHECK(ZB_IS_CONSTRUCTIBLE(zb::Span<const int>, const zb::Vector<int>&));

        // Types lacking .data()/.size() must not match the range constructor.
        STATIC_CHECK(!ZB_IS_CONSTRUCTIBLE(zb::Span<int>, int));
    }
}
