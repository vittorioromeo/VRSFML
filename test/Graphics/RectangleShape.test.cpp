#include "SFML/Graphics/RectangleShape.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[Graphics] sf::RectangleShape")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::RectangleShape));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::RectangleShape));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::RectangleShape));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::RectangleShape));
    }

    SECTION("Default constructor")
    {
        const sf::RectangleShape rectangle;
        CHECK(rectangle.getSize() == sf::Vector2f{0, 0});
        CHECK(rectangle.getPointCount() == 4);
        CHECK(rectangle.getPoint(0) == sf::Vector2f{0, 0});
        CHECK(rectangle.getPoint(1) == sf::Vector2f{0, 0});
        CHECK(rectangle.getPoint(2) == sf::Vector2f{0, 0});
        CHECK(rectangle.getPoint(3) == sf::Vector2f{0, 0});
        CHECK(rectangle.getGeometricCenter() == sf::Vector2f{0, 0});
    }

    SECTION("Size constructor")
    {
        const sf::RectangleShape rectangle({9, 8});
        CHECK(rectangle.getSize() == sf::Vector2f{9, 8});
        CHECK(rectangle.getPointCount() == 4);
        CHECK(rectangle.getPoint(0) == sf::Vector2f{0, 0});
        CHECK(rectangle.getPoint(1) == sf::Vector2f{9, 0});
        CHECK(rectangle.getPoint(2) == sf::Vector2f{9, 8});
        CHECK(rectangle.getPoint(3) == sf::Vector2f{0, 8});
        CHECK(rectangle.getGeometricCenter() == sf::Vector2f(9.f, 8.f) / 2.f);
    }

    SECTION("Set size")
    {
        sf::RectangleShape rectangle({7, 6});
        rectangle.setSize({5, 4});
        CHECK(rectangle.getSize() == sf::Vector2f{5, 4});
        CHECK(rectangle.getGeometricCenter() == sf::Vector2f(5.f, 4.f) / 2.f);
    }
}
