#include "Zancle/Graphics/Shape.hpp"

#include "Zancle/Graphics/Color.hpp"

// Other 1st party headers
#include "GraphicsUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/Trait/HasVirtualDestructor.hpp"
#include "ZancleBase/Trait/IsConstructible.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsMoveConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"


class TriangleShape : public za::Shape
{
public:
    explicit TriangleShape(za::Vec2f size) : za::Shape({}), m_size(size)
    {
        m_points[0] = {m_size.x / 2, 0};
        m_points[1] = {0, m_size.y};
        m_points[2] = {m_size.x, m_size.y};

        update(m_points, 3);
    }

private:
    za::Vec2f m_size;
    za::Vec2f m_points[3];
};

TEST_CASE("[Graphics] za::Shape" * tst::skip(skipDisplayTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_CONSTRUCTIBLE(za::Shape));
        STATIC_CHECK(ZB_IS_CONSTRUCTIBLE(za::Shape, za::Shape::Data));
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::Shape));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::Shape));
        STATIC_CHECK(ZB_IS_MOVE_CONSTRUCTIBLE(za::Shape));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Shape));
        STATIC_CHECK(!ZB_HAS_VIRTUAL_DESTRUCTOR(za::Shape));
    }

    SECTION("Default constructor")
    {
        const TriangleShape triangleShape({0, 0});
        CHECK(triangleShape.getTextureRect() == za::Rect2f());
        CHECK(triangleShape.getFillColor() == za::Color::White);
        CHECK(triangleShape.getOutlineColor() == za::Color::White);
        CHECK(triangleShape.getOutlineThickness() == 0.f);
        CHECK(triangleShape.getLocalBounds() == za::Rect2f());
        CHECK(triangleShape.getGlobalBounds() == za::Rect2f());
    }

    SECTION("Set/get texture rect")
    {
        TriangleShape triangleShape({});
        triangleShape.setTextureRect({{4, 5}, {6, 7}});
        CHECK(triangleShape.getTextureRect() == za::Rect2f({4, 5}, {6, 7}));
    }

    SECTION("Set/get fill color")
    {
        TriangleShape triangleShape({});
        triangleShape.setFillColor(za::Color::Cyan);
        CHECK(triangleShape.getFillColor() == za::Color::Cyan);
    }

    SECTION("Set/get outline color")
    {
        TriangleShape triangleShape({});
        triangleShape.setOutlineColor(za::Color::Magenta);
        CHECK(triangleShape.getOutlineColor() == za::Color::Magenta);
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
        CHECK(triangleShape.getLocalBounds() == za::Rect2f({0, 0}, {30, 40}));
        CHECK(triangleShape.getGlobalBounds() == za::Rect2f({0, 0}, {30, 40}));

        SECTION("Move and rotate")
        {
            triangleShape.position += {1, 1};
            triangleShape.rotation += za::degrees(90);
            CHECK(triangleShape.getLocalBounds() == za::Rect2f({0, 0}, {30, 40}));
            CHECK(triangleShape.getGlobalBounds() == Approx(za::Rect2f({-39, 1}, {40, 30})));
        }

        SECTION("Add outline")
        {
            triangleShape.setOutlineThickness(5);
            CHECK(triangleShape.getLocalBounds() == Approx(za::Rect2f({-7.2150f, -14.2400f}, {44.4300f, 59.2400f})));
            CHECK(triangleShape.getGlobalBounds() == Approx(za::Rect2f({-7.2150f, -14.2400f}, {44.4300f, 59.2400f})));
        }

        SECTION("Add beveled outline")
        {
            triangleShape.setMiterLimit(2);
            triangleShape.setOutlineThickness(5);
            CHECK(triangleShape.getLocalBounds() == Approx(za::Rect2f({-7.2150f, -10.f}, {44.4300f, 55.f})));
            CHECK(triangleShape.getGlobalBounds() == Approx(za::Rect2f({-7.2150f, -10.f}, {44.4300f, 55.f})));
        }
    }
}
