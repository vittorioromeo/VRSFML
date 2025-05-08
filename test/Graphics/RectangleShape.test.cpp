#include "SFML/Graphics/RectangleShape.hpp"

#include "SFML/Graphics/Color.hpp"

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
        const sf::RectangleShape rectangle{{}};
        CHECK(rectangle.getSize() == sf::Vec2f{0, 0});
        CHECK(rectangle.getPointCount() == 4);
        CHECK(rectangle.getPoint(0) == sf::Vec2f{0, 0});
        CHECK(rectangle.getPoint(1) == sf::Vec2f{0, 0});
        CHECK(rectangle.getPoint(2) == sf::Vec2f{0, 0});
        CHECK(rectangle.getPoint(3) == sf::Vec2f{0, 0});
        CHECK(rectangle.getGeometricCenter() == sf::Vec2f{0, 0});
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 0u);
    }

    SECTION("Size constructor")
    {
        const sf::RectangleShape rectangle{{.size = {9.f, 8.f}}};
        CHECK(rectangle.getSize() == sf::Vec2f{9, 8});
        CHECK(rectangle.getPointCount() == 4);
        CHECK(rectangle.getPoint(0) == sf::Vec2f{0, 0});
        CHECK(rectangle.getPoint(1) == sf::Vec2f{9, 0});
        CHECK(rectangle.getPoint(2) == sf::Vec2f{9, 8});
        CHECK(rectangle.getPoint(3) == sf::Vec2f{0, 8});
        CHECK(rectangle.getGeometricCenter() == sf::Vec2f(9.f, 8.f) / 2.f);
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 0u);
    }

    SECTION("Set size")
    {
        sf::RectangleShape rectangle{{.size = {7.f, 6.f}}};
        rectangle.setSize({5, 4});
        CHECK(rectangle.getSize() == sf::Vec2f{5, 4});
        CHECK(rectangle.getGeometricCenter() == sf::Vec2f(5.f, 4.f) / 2.f);
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 0u);
    }

    SECTION("With outline")
    {
        sf::RectangleShape rectangle{{.outlineThickness = 1.f, .size = {7.f, 6.f}}};
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 10u);
    }
}
