#include "SFML/System/Rect.hpp"

#include "SFML/System/RectUtils.hpp"
#include "SFML/System/Vector2.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEMPLATE_TEST_CASE("[System] sf::Rect", "", int, float)
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Rect<TestType>));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Rect<TestType>));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Rect<TestType>));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Rect<TestType>));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::Rect<TestType>)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::Rect<TestType>));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Rect<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Rect<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::Rect<TestType>));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::Rect<TestType>, sf::Rect<TestType>));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            constexpr sf::Rect<TestType> rectangle;
            STATIC_CHECK(rectangle.position == sf::Vector2<TestType>());
            STATIC_CHECK(rectangle.size == sf::Vector2<TestType>());
        }

        SECTION("(Vector2, Vector2) constructor")
        {
            constexpr sf::Vector2<TestType> position(1, 2);
            constexpr sf::Vector2<TestType> dimension(3, 4);
            constexpr sf::Rect<TestType>    rectangle(position, dimension);

            STATIC_CHECK(rectangle.position == position);
            STATIC_CHECK(rectangle.size == dimension);
        }

        SECTION("Conversion constructor")
        {
            constexpr sf::FloatRect sourceRectangle{{1.0f, 2.0f}, {3.0f, 4.0f}};
            constexpr auto          rectangle = sourceRectangle.to<sf::IntRect>();

            STATIC_CHECK(rectangle.position == sf::Vector2i{1, 2});
            STATIC_CHECK(rectangle.size == sf::Vector2i{3, 4});
        }
    }

    SECTION("contains(Vector2)")
    {
        constexpr sf::Rect<TestType> rectangle({0, 0}, {10, 10});

        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(0, 0)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(9, 0)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(0, 9)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(9, 9)) == true);
        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(9, 10)) == false);
        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(10, 9)) == false);
        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(10, 10)) == false);
        STATIC_CHECK(rectangle.contains(sf::Vector2<TestType>(15, 15)) == false);
    }

    SECTION("findIntersection()")
    {
        constexpr sf::Rect<TestType> rectangle({0, 0}, {10, 10});
        constexpr sf::Rect<TestType> intersectingRectangle({5, 5}, {10, 10});

        const auto intersectionResult = sf::findIntersection(rectangle, intersectingRectangle);
        REQUIRE(intersectionResult.hasValue());
        SFML_BASE_ASSERT(*intersectionResult == sf::Rect<TestType>({5, 5}, {5, 5}));

        constexpr sf::Rect<TestType> nonIntersectingRectangle({-5, -5}, {5, 5});
        CHECK_FALSE(sf::findIntersection(rectangle, nonIntersectingRectangle).hasValue());
    }

    SECTION("getCenter()")
    {
        STATIC_CHECK(sf::Rect<TestType>({}, {}).getCenter() == sf::Vector2<TestType>());
        STATIC_CHECK(sf::Rect<TestType>({1, 2}, {4, 6}).getCenter() == sf::Vector2<TestType>(3, 5));
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            STATIC_CHECK(sf::Rect<TestType>() == sf::Rect<TestType>());
            STATIC_CHECK(sf::Rect<TestType>({1, 3}, {2, 5}) == sf::Rect<TestType>({1, 3}, {2, 5}));

            STATIC_CHECK_FALSE(sf::Rect<TestType>({1, 0}, {0, 0}) == sf::Rect<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(sf::Rect<TestType>({0, 1}, {0, 0}) == sf::Rect<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(sf::Rect<TestType>({0, 0}, {1, 0}) == sf::Rect<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK_FALSE(sf::Rect<TestType>({0, 0}, {0, 1}) == sf::Rect<TestType>({0, 0}, {0, 0}));
        }

        SECTION("operator!=")
        {
            STATIC_CHECK(sf::Rect<TestType>({1, 0}, {0, 0}) != sf::Rect<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(sf::Rect<TestType>({0, 1}, {0, 0}) != sf::Rect<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(sf::Rect<TestType>({0, 0}, {1, 0}) != sf::Rect<TestType>({0, 0}, {0, 0}));
            STATIC_CHECK(sf::Rect<TestType>({0, 0}, {0, 1}) != sf::Rect<TestType>({0, 0}, {0, 0}));

            STATIC_CHECK_FALSE(sf::Rect<TestType>() != sf::Rect<TestType>());
            STATIC_CHECK_FALSE(sf::Rect<TestType>({1, 3}, {2, 5}) != sf::Rect<TestType>({1, 3}, {2, 5}));
        }
    }
}
