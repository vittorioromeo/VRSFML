#include "SFML/System/Rect2.hpp"
#include "SFML/System/RectUtils.hpp"
#include "SFML/System/Vec2.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEMPLATE_TEST_CASE("[System] sf::Rect2", "", int, float)
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Rect2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Rect2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Rect2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Rect2<TestType>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::Rect2<TestType>)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::Rect2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Rect2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Rect2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::Rect2<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::Rect2<TestType>, sf::Rect2<TestType>));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            constexpr sf::Rect2<TestType> rectangle{};
            STATIC_CHECK(rectangle.position == sf::Vec2<TestType>());
            STATIC_CHECK(rectangle.size == sf::Vec2<TestType>());
        }

        SECTION("(Vec2, Vec2) constructor")
        {
            constexpr sf::Vec2<TestType>  position(1, 2);
            constexpr sf::Vec2<TestType>  dimension(3, 4);
            constexpr sf::Rect2<TestType> rectangle(position, dimension);

            STATIC_CHECK(rectangle.position == position);
            STATIC_CHECK(rectangle.size == dimension);
        }

        SECTION("Conversion constructor")
        {
            constexpr sf::Rect2f sourceRectangle{{1.f, 2.f}, {3.f, 4.f}};
            constexpr auto       rectangle = sourceRectangle.toRect2i();

            STATIC_CHECK(rectangle.position == sf::Vec2i{1, 2});
            STATIC_CHECK(rectangle.size == sf::Vec2i{3, 4});
        }
    }

    SECTION("contains(Vec2)")
    {
        constexpr sf::Rect2<TestType> rectangle({0, 0}, {10, 10});

        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(0, 0)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(9, 0)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(0, 9)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(9, 9)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(9, 10)) == false);
        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(10, 9)) == false);
        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(10, 10)) == false);
        STATIC_CHECK(rectangle.contains(sf::Vec2<TestType>(15, 15)) == false);
    }

    SECTION("findIntersection()")
    {
        constexpr sf::Rect2<TestType> rectangle({0, 0}, {10, 10});
        constexpr sf::Rect2<TestType> intersectingRectangle({5, 5}, {10, 10});

        const auto intersectionResult = sf::findIntersection(rectangle, intersectingRectangle);
        REQUIRE(intersectionResult.hasValue());
        SFML_BASE_ASSERT(*intersectionResult == sf::Rect2<TestType>({5, 5}, {5, 5}));

        constexpr sf::Rect2<TestType> nonIntersectingRectangle({-5, -5}, {5, 5});
        CHECK_FALSE(sf::findIntersection(rectangle, nonIntersectingRectangle).hasValue());
    }

    SECTION("getCenter()")
    {
        STATIC_CHECK(sf::Rect2<TestType>({}, {}).getCenter() == sf::Vec2<TestType>());
        STATIC_CHECK(sf::Rect2<TestType>({1, 2}, {4, 6}).getCenter() == sf::Vec2<TestType>(3, 5));
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            STATIC_CHECK(sf::Rect2<TestType>() == sf::Rect2<TestType>());
            STATIC_CHECK(sf::Rect2<TestType>({1, 3}, {2, 5}) == sf::Rect2<TestType>({1, 3}, {2, 5}));

            STATIC_CHECK_FALSE(sf::Rect2<TestType>({1, 0}, {0, 0}) == sf::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(sf::Rect2<TestType>({0, 1}, {0, 0}) == sf::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(sf::Rect2<TestType>({0, 0}, {1, 0}) == sf::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(sf::Rect2<TestType>({0, 0}, {0, 1}) == sf::Rect2<TestType>({0, 0}, {0, 0}));
        }

        SECTION("operator!=")
        {
            STATIC_CHECK(sf::Rect2<TestType>({1, 0}, {0, 0}) != sf::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(sf::Rect2<TestType>({0, 1}, {0, 0}) != sf::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(sf::Rect2<TestType>({0, 0}, {1, 0}) != sf::Rect2<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(sf::Rect2<TestType>({0, 0}, {0, 1}) != sf::Rect2<TestType>({0, 0}, {0, 0}));

            STATIC_CHECK_FALSE(sf::Rect2<TestType>() != sf::Rect2<TestType>());
            STATIC_CHECK_FALSE(sf::Rect2<TestType>({1, 3}, {2, 5}) != sf::Rect2<TestType>({1, 3}, {2, 5}));
        }
    }

    SECTION("Get anchor point")
    {
        constexpr sf::Rect2<TestType> r({0, 0}, {1024, 1024});

        STATIC_CHECK(r.getAnchorPoint({0.f, 0.f}) == sf::Vec2<TestType>{0, 0});
        STATIC_CHECK(r.getAnchorPoint({0.5f, 0.f}) == sf::Vec2<TestType>{512, 0});
        STATIC_CHECK(r.getAnchorPoint({1.f, 0.f}) == sf::Vec2<TestType>{1024, 0});
        STATIC_CHECK(r.getAnchorPoint({0.f, 0.5f}) == sf::Vec2<TestType>{0, 512});
        STATIC_CHECK(r.getAnchorPoint({0.5f, 0.5f}) == sf::Vec2<TestType>{512, 512});
        STATIC_CHECK(r.getAnchorPoint({1.f, 0.5f}) == sf::Vec2<TestType>{1024, 512});
        STATIC_CHECK(r.getAnchorPoint({0.f, 1.f}) == sf::Vec2<TestType>{0, 1024});
        STATIC_CHECK(r.getAnchorPoint({0.5f, 1.f}) == sf::Vec2<TestType>{512, 1024});
        STATIC_CHECK(r.getAnchorPoint({1.f, 1.f}) == sf::Vec2<TestType>{1024, 1024});
    }

    SECTION("Get anchor point offset")
    {
        constexpr sf::Rect2<TestType> r({0, 0}, {1000, 1000});

        STATIC_CHECK(r.getAnchorPointOffset({0.f, 0.f}) == sf::Vec2<TestType>{0, 0});
        STATIC_CHECK(r.getAnchorPointOffset({0.5f, 0.f}) == sf::Vec2<TestType>{-500, 0});
        STATIC_CHECK(r.getAnchorPointOffset({1.f, 0.f}) == sf::Vec2<TestType>{-1000, 0});

        STATIC_CHECK(r.getAnchorPointOffset({0.f, 0.5f}) == sf::Vec2<TestType>{0, -500});
        STATIC_CHECK(r.getAnchorPointOffset({0.5f, 0.5f}) == sf::Vec2<TestType>{-500, -500});
        STATIC_CHECK(r.getAnchorPointOffset({1.f, 0.5f}) == sf::Vec2<TestType>{-1000, -500});

        STATIC_CHECK(r.getAnchorPointOffset({0.f, 1.f}) == sf::Vec2<TestType>{0, -1000});
        STATIC_CHECK(r.getAnchorPointOffset({0.5f, 1.f}) == sf::Vec2<TestType>{-500, -1000});
        STATIC_CHECK(r.getAnchorPointOffset({1.f, 1.f}) == sf::Vec2<TestType>{-1000, -1000});
    }

    SECTION("Set anchor point")
    {
        const auto doTest = [](sf::Vec2f factors, sf::Vec2<TestType> expected)
        {
            sf::Rect2<TestType> r({0, 0}, {1000, 1000});
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
