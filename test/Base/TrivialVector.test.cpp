#include "Tst/Tst.hpp"

#include "ZancleBase/Macros.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsMoveAssignable.hpp"
#include "ZancleBase/Trait/IsMoveConstructible.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"
#include "ZancleBase/Vector.hpp"


namespace
{
TEST_CASE("[Base] Base/Vector.hpp")
{
    const auto asConst = [](auto& x) -> const auto& { return x; };

    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIAL(int));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(int));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(int));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(int));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(int));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(int));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(int));

        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::Vector<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPYABLE(zb::Vector<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_DESTRUCTIBLE(zb::Vector<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(zb::Vector<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(zb::Vector<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(zb::Vector<int>));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(zb::Vector<int>));

        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(zb::Vector<int>));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(zb::Vector<int>));
        STATIC_CHECK(ZB_IS_MOVE_CONSTRUCTIBLE(zb::Vector<int>));
        STATIC_CHECK(ZB_IS_MOVE_ASSIGNABLE(zb::Vector<int>));

        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::Vector<int>));
    }

    SECTION("Empty")
    {
#define DO_EMPTY_CHECKS(tv)                  \
    CHECK((tv).begin() == nullptr);          \
    CHECK((tv).end() == nullptr);            \
    CHECK((tv).data() == nullptr);           \
                                             \
    CHECK(asConst((tv)).begin() == nullptr); \
    CHECK(asConst((tv)).end() == nullptr);   \
    CHECK(asConst((tv)).data() == nullptr);  \
                                             \
    CHECK((tv).size() == 0u);                \
    CHECK((tv).empty());

        zb::Vector<int> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.clear();
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        zb::Vector<int> tv2 = tv;
        DO_EMPTY_CHECKS(tv2);
        CHECK(tv2.capacity() == 0u);

        zb::Vector<int> tv3 = ZB_MOVE(tv);
        DO_EMPTY_CHECKS(tv3);
        CHECK(tv3.capacity() == 0u);

        zb::Vector<int> tv4;
        tv4 = tv;
        DO_EMPTY_CHECKS(tv4);
        CHECK(tv4.capacity() == 0u);

        zb::Vector<int> tv5;
        tv5 = ZB_MOVE(tv4);
        DO_EMPTY_CHECKS(tv5);
        CHECK(tv5.capacity() == 0u);
    }

    SECTION("Copy construct empty vector")
    {
        const zb::Vector<int> source;
        const zb::Vector<int> copy = source; // NOLINT(performance-unnecessary-copy-initialization)

        CHECK(copy.data() == nullptr);
        CHECK(copy.begin() == nullptr);
        CHECK(copy.end() == nullptr);
        CHECK(copy.size() == 0u);
        CHECK(copy.capacity() == 0u);
        CHECK(copy.empty());
    }

    SECTION("Non-empty")
    {
        zb::Vector<int> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.reserve(1);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 0u);
        CHECK(tv.capacity() == 1u);

        tv.unsafeEmplaceBack(42);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() == 1u);
        CHECK(tv[0] == 42);

        tv.reserveMore(10);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() >= 11u);
        CHECK(tv[0] == 42);

        tv.resize(100);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 100u);
        CHECK(tv.capacity() >= 100u);
        CHECK(tv[0] == 42);

        for (zb::SizeT i = 1; i < 100; ++i)
            CHECK(tv[i] == 0);
    }

    SECTION("Non default constructible")
    {
        struct S
        {
            S(int x) : value(x)
            {
            }

            int value;
        };

        zb::Vector<S> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.reserve(1);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 0u);
        CHECK(tv.capacity() == 1u);

        tv.unsafeEmplaceBack(42);
        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() == 1u);
        CHECK(tv[0].value == 42);

        tv.reserveMore(10);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 1u);
        CHECK(tv.capacity() >= 11u);
        CHECK(tv[0].value == 42);

        tv.reserve(100);
        for (zb::SizeT i = 1; i < 100; ++i)
            tv.unsafeEmplaceBack(0);

        CHECK(tv.data() != nullptr);
        CHECK(tv.begin() == tv.data());
        CHECK(tv.end() == tv.data() + tv.size());
        CHECK(tv.size() == 100u);
        CHECK(tv.capacity() >= 100u);
        CHECK(tv[0].value == 42);

        for (zb::SizeT i = 1; i < 100; ++i)
            CHECK(tv[i].value == 0);
    }

    SECTION("Shrink to fit")
    {
        zb::Vector<int> tv;
        DO_EMPTY_CHECKS(tv);
        CHECK(tv.capacity() == 0u);

        tv.reserve(255);

        for (int i = 0; i < 100; ++i)
            tv.unsafeEmplaceBack(5);

        CHECK(tv.size() == 100);
        CHECK(tv.capacity() > 100);

        tv.shrinkToFit();

        CHECK(tv.size() == 100);
        CHECK(tv.capacity() == 100);
    }
}

} // namespace
