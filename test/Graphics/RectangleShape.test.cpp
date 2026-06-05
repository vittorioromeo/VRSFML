#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/RectangleShape.hpp"

#include "Zancle/Graphics/Color.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Graphics] za::RectangleShape")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::RectangleShape));
        STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::RectangleShape));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::RectangleShape));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::RectangleShape));
    }

    SECTION("Default constructor")
    {
        const za::RectangleShape rectangle{{}};
        CHECK(rectangle.getSize() == za::Vec2f{0, 0});
        CHECK(rectangle.getPointCount() == 4);
        CHECK(rectangle.getPoint(0) == za::Vec2f{0, 0});
        CHECK(rectangle.getPoint(1) == za::Vec2f{0, 0});
        CHECK(rectangle.getPoint(2) == za::Vec2f{0, 0});
        CHECK(rectangle.getPoint(3) == za::Vec2f{0, 0});
        CHECK(rectangle.getGeometricCenter() == za::Vec2f{0, 0});
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 0u);
    }

    SECTION("Size constructor")
    {
        const za::RectangleShape rectangle{{.size = {9.f, 8.f}}};
        CHECK(rectangle.getSize() == za::Vec2f{9, 8});
        CHECK(rectangle.getPointCount() == 4);
        CHECK(rectangle.getPoint(0) == za::Vec2f{0, 0});
        CHECK(rectangle.getPoint(1) == za::Vec2f{9, 0});
        CHECK(rectangle.getPoint(2) == za::Vec2f{9, 8});
        CHECK(rectangle.getPoint(3) == za::Vec2f{0, 8});
        CHECK(rectangle.getGeometricCenter() == za::Vec2f(9.f, 8.f) / 2.f);
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 0u);
    }

    SECTION("Set size")
    {
        za::RectangleShape rectangle{{.size = {7.f, 6.f}}};
        rectangle.setSize({5, 4});
        CHECK(rectangle.getSize() == za::Vec2f{5, 4});
        CHECK(rectangle.getGeometricCenter() == za::Vec2f(5.f, 4.f) / 2.f);
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 0u);
    }

    SECTION("With outline")
    {
        za::RectangleShape rectangle{{.outlineThickness = 1.f, .size = {7.f, 6.f}}};
        CHECK(rectangle.getFillVertices().size() == 6u);
        CHECK(rectangle.getOutlineVertices().size() == 10u);
    }
}
