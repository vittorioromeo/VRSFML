#include "SFML/Graphics/Shape.hpp"

#include "SFML/Graphics/Color.hpp"

// Other 1st party headers
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Rect.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>

class TriangleShape : public sf::Shape
{
public:
    explicit TriangleShape(sf::Vec2f size) : sf::Shape({}), m_size(size)
    {
        m_points[0] = {m_size.x / 2, 0};
        m_points[1] = {0, m_size.y};
        m_points[2] = {m_size.x, m_size.y};

        update(m_points, 3);
    }

private:
    sf::Vec2f m_size;
    sf::Vec2f m_points[3];
};

TEST_CASE("[Graphics] sf::Shape" * doctest::skip(skipDisplayTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Shape));
        STATIC_CHECK(SFML_BASE_IS_CONSTRUCTIBLE(sf::Shape, sf::Shape::Data));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Shape));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Shape));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::Shape));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Shape));
        STATIC_CHECK(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::Shape));
    }

    SECTION("Default constructor")
    {
        const TriangleShape triangleShape({0, 0});
        CHECK(triangleShape.getTextureRect() == sf::FloatRect());
        CHECK(triangleShape.getFillColor() == sf::Color::White);
        CHECK(triangleShape.getOutlineColor() == sf::Color::White);
        CHECK(triangleShape.getOutlineThickness() == 0.f);
        CHECK(triangleShape.getLocalBounds() == sf::FloatRect());
        CHECK(triangleShape.getGlobalBounds() == sf::FloatRect());
    }

    SECTION("Set/get texture rect")
    {
        TriangleShape triangleShape({});
        triangleShape.setTextureRect({{4, 5}, {6, 7}});
        CHECK(triangleShape.getTextureRect() == sf::FloatRect({4, 5}, {6, 7}));
    }

    SECTION("Set/get fill color")
    {
        TriangleShape triangleShape({});
        triangleShape.setFillColor(sf::Color::Cyan);
        CHECK(triangleShape.getFillColor() == sf::Color::Cyan);
    }

    SECTION("Set/get outline color")
    {
        TriangleShape triangleShape({});
        triangleShape.setOutlineColor(sf::Color::Magenta);
        CHECK(triangleShape.getOutlineColor() == sf::Color::Magenta);
    }

    SECTION("Set/get outline thickness")
    {
        TriangleShape triangleShape({});
        triangleShape.setOutlineThickness(3.14f);
        CHECK(triangleShape.getOutlineThickness() == 3.14f);
    }

    SECTION("Get bounds")
    {
        TriangleShape triangleShape({30, 40});
        CHECK(triangleShape.getLocalBounds() == sf::FloatRect({0, 0}, {30, 40}));
        CHECK(triangleShape.getGlobalBounds() == sf::FloatRect({0, 0}, {30, 40}));

        SECTION("Move and rotate")
        {
            triangleShape.position += {1, 1};
            triangleShape.rotation += sf::degrees(90);
            CHECK(triangleShape.getLocalBounds() == sf::FloatRect({0, 0}, {30, 40}));
            CHECK(triangleShape.getGlobalBounds() == Approx(sf::FloatRect({-39, 1}, {40, 30})));
        }

        SECTION("Add outline")
        {
            triangleShape.setOutlineThickness(5);
            CHECK(triangleShape.getLocalBounds() == Approx(sf::FloatRect({-7.2150f, -14.2400f}, {44.4300f, 59.2400f})));
            CHECK(triangleShape.getGlobalBounds() == Approx(sf::FloatRect({-7.2150f, -14.2400f}, {44.4300f, 59.2400f})));
        }

        SECTION("Add beveled outline")
        {
            triangleShape.setMiterLimit(2);
            triangleShape.setOutlineThickness(5);
            CHECK(triangleShape.getLocalBounds() == Approx(sf::FloatRect({-7.2150f, -10.f}, {44.4300f, 55.f})));
            CHECK(triangleShape.getGlobalBounds() == Approx(sf::FloatRect({-7.2150f, -10.f}, {44.4300f, 55.f})));
        }
    }
}
