#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"
#include "Zancle/Geometry/RectUtils.hpp"

#include "Zancle/Diagnostic/Assert.hpp"

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

TEMPLATE_TEST_CASE("[System] za::Rect2", "", int, float)
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::Rect2<TestType>));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::Rect2<TestType>));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::Rect2<TestType>));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::Rect2<TestType>));

        STATIC_CHECK(!ZA_IS_TRIVIAL(za::Rect2<TestType>)); // because of member initializers
        STATIC_CHECK(ZA_IS_STANDARD_LAYOUT(za::Rect2<TestType>));
        STATIC_CHECK(ZA_IS_AGGREGATE(za::Rect2<TestType>));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPYABLE(za::Rect2<TestType>));
        STATIC_CHECK(ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::Rect2<TestType>));
        STATIC_CHECK(ZA_IS_TRIVIALLY_ASSIGNABLE(za::Rect2<TestType>, za::Rect2<TestType>));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            constexpr za::Rect2<TestType> rectangle{};
            STATIC_CHECK(rectangle.position == za::Vec2<TestType>());
            STATIC_CHECK(rectangle.size == za::Vec2<TestType>());
        }

        SECTION("(Vec2, Vec2) constructor")
        {
            constexpr za::Vec2<TestType>  position(1, 2);
            constexpr za::Vec2<TestType>  dimension(3, 4);
            constexpr za::Rect2<TestType> rectangle(position, dimension);

            STATIC_CHECK(rectangle.position == position);
            STATIC_CHECK(rectangle.size == dimension);
        }

        SECTION("Conversion constructor")
        {
            constexpr za::Rect2f sourceRectangle{{1.f, 2.f}, {3.f, 4.f}};
            constexpr auto       rectangle = sourceRectangle.toRect2i();

            STATIC_CHECK(rectangle.position == za::Vec2i{1, 2});
            STATIC_CHECK(rectangle.size == za::Vec2i{3, 4});
        }
    }

    SECTION("contains(Vec2)")
    {
        constexpr za::Rect2<TestType> rectangle({0, 0}, {10, 10});

        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(0, 0)) == true);
        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(9, 0)) == true);
        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(0, 9)) == true);
        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(9, 9)) == true);
        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(9, 10)) == false);
        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(10, 9)) == false);
        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(10, 10)) == false);
        STATIC_CHECK(rectangle.contains(za::Vec2<TestType>(15, 15)) == false);
    }

    SECTION("findIntersection()")
    {
        constexpr za::Rect2<TestType> rectangle({0, 0}, {10, 10});
        constexpr za::Rect2<TestType> intersectingRectangle({5, 5}, {10, 10});

        const auto intersectionResult = za::findIntersection(rectangle, intersectingRectangle);
        REQUIRE(intersectionResult.hasValue());
        ZA_ASSERT(*intersectionResult == za::Rect2<TestType>({5, 5}, {5, 5}));

        constexpr za::Rect2<TestType> nonIntersectingRectangle({-5, -5}, {5, 5});
        CHECK_FALSE(za::findIntersection(rectangle, nonIntersectingRectangle).hasValue());
    }

    SECTION("getCenter()")
    {
        STATIC_CHECK(za::Rect2<TestType>({}, {}).getCenter() == za::Vec2<TestType>());
        STATIC_CHECK(za::Rect2<TestType>({1, 2}, {4, 6}).getCenter() == za::Vec2<TestType>(3, 5));
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            STATIC_CHECK(za::Rect2<TestType>() == za::Rect2<TestType>());
            STATIC_CHECK(za::Rect2<TestType>({1, 3}, {2, 5}) == za::Rect2<TestType>({1, 3}, {2, 5}));

            STATIC_CHECK_FALSE(za::Rect2<TestType>({1, 0}, {0, 0}) == za::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(za::Rect2<TestType>({0, 1}, {0, 0}) == za::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(za::Rect2<TestType>({0, 0}, {1, 0}) == za::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(za::Rect2<TestType>({0, 0}, {0, 1}) == za::Rect2<TestType>({0, 0}, {0, 0}));
        }

        SECTION("operator!=")
        {
            STATIC_CHECK(za::Rect2<TestType>({1, 0}, {0, 0}) != za::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(za::Rect2<TestType>({0, 1}, {0, 0}) != za::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(za::Rect2<TestType>({0, 0}, {1, 0}) != za::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(za::Rect2<TestType>({0, 0}, {0, 1}) != za::Rect2<TestType>({0, 0}, {0, 0}));

            STATIC_CHECK_FALSE(za::Rect2<TestType>() != za::Rect2<TestType>());
            STATIC_CHECK_FALSE(za::Rect2<TestType>({1, 3}, {2, 5}) != za::Rect2<TestType>({1, 3}, {2, 5}));
        }
    }

    SECTION("Get anchor point")
    {
        constexpr za::Rect2<TestType> r({0, 0}, {1024, 1024});

        STATIC_CHECK(r.getAnchorPoint({0.f, 0.f}) == za::Vec2<TestType>{0, 0});
        STATIC_CHECK(r.getAnchorPoint({0.5f, 0.f}) == za::Vec2<TestType>{512, 0});
        STATIC_CHECK(r.getAnchorPoint({1.f, 0.f}) == za::Vec2<TestType>{1024, 0});
        STATIC_CHECK(r.getAnchorPoint({0.f, 0.5f}) == za::Vec2<TestType>{0, 512});
        STATIC_CHECK(r.getAnchorPoint({0.5f, 0.5f}) == za::Vec2<TestType>{512, 512});
        STATIC_CHECK(r.getAnchorPoint({1.f, 0.5f}) == za::Vec2<TestType>{1024, 512});
        STATIC_CHECK(r.getAnchorPoint({0.f, 1.f}) == za::Vec2<TestType>{0, 1024});
        STATIC_CHECK(r.getAnchorPoint({0.5f, 1.f}) == za::Vec2<TestType>{512, 1024});
        STATIC_CHECK(r.getAnchorPoint({1.f, 1.f}) == za::Vec2<TestType>{1024, 1024});
    }

    SECTION("Get anchor point offset")
    {
        constexpr za::Rect2<TestType> r({0, 0}, {1000, 1000});

        STATIC_CHECK(r.getAnchorPointOffset({0.f, 0.f}) == za::Vec2<TestType>{0, 0});
        STATIC_CHECK(r.getAnchorPointOffset({0.5f, 0.f}) == za::Vec2<TestType>{-500, 0});
        STATIC_CHECK(r.getAnchorPointOffset({1.f, 0.f}) == za::Vec2<TestType>{-1000, 0});

        STATIC_CHECK(r.getAnchorPointOffset({0.f, 0.5f}) == za::Vec2<TestType>{0, -500});
        STATIC_CHECK(r.getAnchorPointOffset({0.5f, 0.5f}) == za::Vec2<TestType>{-500, -500});
        STATIC_CHECK(r.getAnchorPointOffset({1.f, 0.5f}) == za::Vec2<TestType>{-1000, -500});

        STATIC_CHECK(r.getAnchorPointOffset({0.f, 1.f}) == za::Vec2<TestType>{0, -1000});
        STATIC_CHECK(r.getAnchorPointOffset({0.5f, 1.f}) == za::Vec2<TestType>{-500, -1000});
        STATIC_CHECK(r.getAnchorPointOffset({1.f, 1.f}) == za::Vec2<TestType>{-1000, -1000});
    }

    SECTION("Set anchor point")
    {
        const auto doTest = [](za::Vec2f factors, za::Vec2<TestType> expected)
        {
            za::Rect2<TestType> r({0, 0}, {1000, 1000});
            r.setAnchorPoint(factors, {500, 500});
            CHECK(r.position == expected);
        };

        doTest({0.f, 0.f}, {500, 500});
        doTest({0.5f, 0.f}, {0, 500});
        doTest({1.f, 0.f}, {-500, 500});

        doTest({0.f, 0.5f}, {500, 0});
        doTest({0.5f, 0.5f}, {0, 0});
        doTest({1.f, 0.5f}, {-500, 0});

        doTest({0.f, 1.f}, {500, -500});
        doTest({0.5f, 1.f}, {0, -500});
        doTest({1.f, 1.f}, {-500, -500});
    }
}
