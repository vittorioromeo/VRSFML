#include "GraphicsUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/Transform.hpp"

#include "Zancle/Geometry/Angle.hpp"
#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"

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


TEST_CASE("[Graphics] za::Transform")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::Transform));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPY_ASSIGNABLE(za::Transform));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::Transform));
        STATIC_CHECK(ZA_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::Transform));

        STATIC_CHECK(!ZA_IS_TRIVIAL(za::Transform)); // because of member initializers
        STATIC_CHECK(ZA_IS_STANDARD_LAYOUT(za::Transform));
        STATIC_CHECK(ZA_IS_AGGREGATE(za::Transform));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPYABLE(za::Transform));
        STATIC_CHECK(ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::Transform));
        STATIC_CHECK(ZA_IS_TRIVIALLY_ASSIGNABLE(za::Transform, za::Transform));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            STATIC_CHECK(za::Transform() == za::Transform::Identity);
        }

        SECTION("3x3 matrix constructor")
        {
            constexpr za::Transform transform(10.f, 11.f, 12.f, 13.f, 14.f, 15.f);

            // clang-format off
            float matrix[]{{},  {},  0.f, 0.f,
                           {},  {},  0.f, 0.f,
                           0.f, 0.f, 1.f, 0.f,
                           {},  {},  0.f, 1.f};
            // clang-format on

            transform.writeTo4x4Matrix(matrix);

            CHECK(matrix[0] == 10.f);
            CHECK(matrix[1] == 13.f);
            CHECK(matrix[2] == 0.f);
            CHECK(matrix[3] == 0.f);
            CHECK(matrix[4] == 11.f);
            CHECK(matrix[5] == 14.f);
            CHECK(matrix[6] == 0.f);
            CHECK(matrix[7] == 0.f);
            CHECK(matrix[8] == 0.f);
            CHECK(matrix[9] == 0.f);
            CHECK(matrix[10] == 1.f);
            CHECK(matrix[11] == 0.f);
            CHECK(matrix[12] == 12.f);
            CHECK(matrix[13] == 15.f);
            CHECK(matrix[14] == 0.f);
            CHECK(matrix[15] == 1.f);
        }
    }

    SECTION("Identity matrix")
    {
        // clang-format off
        float matrix[]{{},  {},  0.f, 0.f,
                       {},  {},  0.f, 0.f,
                       0.f, 0.f, 1.f, 0.f,
                       {},  {},  0.f, 1.f};
        // clang-format on

        za::Transform::Identity.writeTo4x4Matrix(matrix);

        CHECK(matrix[0] == 1.f);
        CHECK(matrix[1] == 0.f);
        CHECK(matrix[2] == 0.f);
        CHECK(matrix[3] == 0.f);
        CHECK(matrix[4] == 0.f);
        CHECK(matrix[5] == 1.f);
        CHECK(matrix[6] == 0.f);
        CHECK(matrix[7] == 0.f);
        CHECK(matrix[8] == 0.f);
        CHECK(matrix[9] == 0.f);
        CHECK(matrix[10] == 1.f);
        CHECK(matrix[11] == 0.f);
        CHECK(matrix[12] == 0.f);
        CHECK(matrix[13] == 0.f);
        CHECK(matrix[14] == 0.f);
        CHECK(matrix[15] == 1.f);
    }

    SECTION("getInverse()")
    {
        STATIC_CHECK(za::Transform::Identity.getInverse() == za::Transform::Identity);
        STATIC_CHECK(za::Transform(1.f, 2.f, 3.f, 4.f, 8.f, 6.f).getInverse() == za::Transform::Identity);
        STATIC_CHECK(za::Transform(1.f, 2.f, 3.f, 5.f, 2.f, 4.f).getInverse() ==
                     za::Transform(-0.25f, 0.25f, -0.25f, 0.625f, -0.125f, -1.375f));
    }

    SECTION("transformPoint()")
    {
        STATIC_CHECK(za::Transform::Identity.transformPoint({-10.f, -10.f}) == za::Vec2f(-10.f, -10.f));
        STATIC_CHECK(za::Transform::Identity.transformPoint({-1.f, -1.f}) == za::Vec2f(-1.f, -1.f));
        STATIC_CHECK(za::Transform::Identity.transformPoint({-1.f, 0.f}) == za::Vec2f(-1.f, 0.f));
        STATIC_CHECK(za::Transform::Identity.transformPoint({0.f, 0.f}) == za::Vec2f(0.f, 0.f));
        STATIC_CHECK(za::Transform::Identity.transformPoint({0.f, 1.f}) == za::Vec2f(0.f, 1.f));
        STATIC_CHECK(za::Transform::Identity.transformPoint({1.f, 1.f}) == za::Vec2f(1.f, 1.f));
        STATIC_CHECK(za::Transform::Identity.transformPoint({10.f, 10.f}) == za::Vec2f(10.f, 10.f));

        constexpr za::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
        STATIC_CHECK(transform.transformPoint({-1.f, -1.f}) == za::Vec2f(0.f, -5.f));
        STATIC_CHECK(transform.transformPoint({0.f, 0.f}) == za::Vec2f(3.f, 4.f));
        STATIC_CHECK(transform.transformPoint({1.f, 1.f}) == za::Vec2f(6.f, 13.f));
    }

    SECTION("transformRect()")
    {
        STATIC_CHECK(za::Transform::Identity.transformRect({{-200.f, -200.f}, {-100.f, -100.f}}) ==
                     za::Rect2f({-300.f, -300.f}, {100.f, 100.f}));
        STATIC_CHECK(za::Transform::Identity.transformRect({{0.f, 0.f}, {0.f, 0.f}}) == za::Rect2f({0.f, 0.f}, {0.f, 0.f}));
        STATIC_CHECK(za::Transform::Identity.transformRect({{100.f, 100.f}, {200.f, 200.f}}) ==
                     za::Rect2f({100.f, 100.f}, {200.f, 200.f}));

        constexpr za::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
        STATIC_CHECK(transform.transformRect({{-100.f, -100.f}, {200.f, 200.f}}) ==
                     za::Rect2f({-297.f, -896.f}, {600.f, 1800.f}));
        STATIC_CHECK(transform.transformRect({{0.f, 0.f}, {0.f, 0.f}}) == za::Rect2f({3.f, 4.f}, {0.f, 0.f}));
        STATIC_CHECK(transform.transformRect({{100.f, 100.f}, {200.f, 200.f}}) ==
                     za::Rect2f({303.f, 904.f}, {600.f, 1800.f}));
    }

    SECTION("combine()")
    {
        auto identity = za::Transform::Identity;
        CHECK(identity.combine(za::Transform::Identity) == za::Transform::Identity);
        CHECK(identity.combine(za::Transform::Identity).combine(za::Transform::Identity) == za::Transform::Identity);

        za::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
        CHECK(identity.combine(transform) == transform);
        CHECK(transform.combine(za::Transform::Identity) == transform);
        CHECK(transform.combine(transform) == za::Transform(9.f, 12.f, 14.f, 24.f, 33.f, 36.f));
        CHECK(transform.combine(za::Transform(10.f, 2.f, 3.f, 4.f, 50.f, 40.f)) ==
              za::Transform(138.f, 618.f, 521.f, 372.f, 1698.f, 1428.f));
    }

    SECTION("translate()")
    {
        za::Transform transform(9, 8, 7, 6, 5, 4);
        CHECK(transform.translate({10.f, 20.f}) == za::Transform(9, 8, 257, 6, 5, 164));
        CHECK(transform.translate({10.f, 20.f}) == za::Transform(9, 8, 507, 6, 5, 324));
    }

    SECTION("rotate()")
    {
        SECTION("Around origin")
        {
            za::Transform transform;
            transform.rotate(za::degrees(90));
            CHECK(transform == Approx(za::Transform(0, -1, 0, 1, 0, 0)));
        }

        SECTION("Around custom point")
        {
            za::Transform transform;
            transform.rotate(za::degrees(90), {1.f, 0.f});
            CHECK(transform == Approx(za::Transform(0, -1, 1, 1, 0, -1)));
        }
    }

    SECTION("scale()")
    {
        SECTION("About origin")
        {
            za::Transform transform(1, 2, 3, 4, 5, 4);
            CHECK(transform.scaleBy({2.f, 4.f}) == za::Transform(2, 8, 3, 8, 20, 4));
            CHECK(transform.scaleBy({0.f, 0.f}) == za::Transform(0, 0, 3, 0, 0, 4));
            CHECK(transform.scaleBy({10.f, 10.f}) == za::Transform(0, 0, 3, 0, 0, 4));
        }

        SECTION("About custom point")
        {
            za::Transform transform(1, 2, 3, 4, 5, 4);
            CHECK(transform.scaleBy({1.f, 2.f}, {1.f, 0.f}) == za::Transform(1, 4, 3, 4, 10, 4));
            CHECK(transform.scaleBy({0.f, 0.f}, {1.f, 0.f}) == za::Transform(0, 0, 4, 0, 0, 8));
        }
    }

    SECTION("Operators")
    {
        SECTION("operator*")
        {
            STATIC_CHECK(za::Transform::Identity * za::Transform::Identity == za::Transform::Identity);
            STATIC_CHECK(za::Transform::Identity * za::Transform::Identity * za::Transform::Identity ==
                         za::Transform::Identity);

            constexpr za::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
            STATIC_CHECK(za::Transform::Identity * transform == transform);
            STATIC_CHECK(transform * za::Transform::Identity == transform);
            STATIC_CHECK(transform * transform == za::Transform(9.f, 12.f, 14.f, 24.f, 33.f, 36.f));
            STATIC_CHECK(transform * za::Transform(10.f, 2.f, 3.f, 4.f, 50.f, 40.f) ==
                         za::Transform(18.f, 102.f, 86.f, 60.f, 258.f, 216.f));
        }

        SECTION("operator*=")
        {
            za::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
            transform *= za::Transform::Identity;
            CHECK(transform == za::Transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f));
            transform *= transform;
            CHECK(transform == za::Transform(9.f, 12.f, 14.f, 24.f, 33.f, 36.f));
            transform *= za::Transform(10.f, 2.f, 3.f, 4.f, 50.f, 40.f);
            CHECK(transform == za::Transform(138.f, 618.f, 521.f, 372.f, 1698.f, 1428.f));
        }

        SECTION("operator* with vec2")
        {
            STATIC_CHECK(za::Transform::Identity * za::Vec2f(-10.f, -10.f) == za::Vec2f(-10.f, -10.f));
            STATIC_CHECK(za::Transform::Identity * za::Vec2f(-1.f, -1.f) == za::Vec2f(-1.f, -1.f));
            STATIC_CHECK(za::Transform::Identity * za::Vec2f(-1.f, 0.f) == za::Vec2f(-1.f, 0.f));
            STATIC_CHECK(za::Transform::Identity * za::Vec2f(0.f, 0.f) == za::Vec2f(0.f, 0.f));
            STATIC_CHECK(za::Transform::Identity * za::Vec2f(0.f, 1.f) == za::Vec2f(0.f, 1.f));
            STATIC_CHECK(za::Transform::Identity * za::Vec2f(1.f, 1.f) == za::Vec2f(1.f, 1.f));
            STATIC_CHECK(za::Transform::Identity * za::Vec2f(10.f, 10.f) == za::Vec2f(10.f, 10.f));

            constexpr za::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
            STATIC_CHECK(transform * za::Vec2f(-1.f, -1.f) == za::Vec2f(0.f, -5.f));
            STATIC_CHECK(transform * za::Vec2f(0.f, 0.f) == za::Vec2f(3.f, 4.f));
            STATIC_CHECK(transform * za::Vec2f(1.f, 1.f) == za::Vec2f(6.f, 13.f));
        }

        SECTION("operator==")
        {
            STATIC_CHECK(za::Transform::Identity == za::Transform::Identity);
            STATIC_CHECK(za::Transform() == za::Transform());
            STATIC_CHECK(za::Transform(0, 0, 0, 0, 0, 0) == za::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(za::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f) ==
                         za::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f));
            STATIC_CHECK(za::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f) ==
                         za::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f));
        }

        SECTION("operator!=")
        {
            STATIC_CHECK_FALSE(za::Transform::Identity != za::Transform::Identity);
            STATIC_CHECK_FALSE(za::Transform() != za::Transform());
            STATIC_CHECK_FALSE(za::Transform(0, 0, 0, 0, 0, 0) != za::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK_FALSE(za::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f) !=
                               za::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f));
            STATIC_CHECK_FALSE(za::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f) !=
                               za::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f));

            STATIC_CHECK(za::Transform(1, 0, 0, 0, 0, 0) != za::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(za::Transform(0, 1, 0, 0, 0, 0) != za::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(za::Transform(0, 0, 1, 0, 0, 0) != za::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(za::Transform(0, 0, 0, 1, 0, 0) != za::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(za::Transform(0, 0, 0, 0, 1, 0) != za::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(za::Transform(0, 0, 0, 0, 0, 1) != za::Transform(0, 0, 0, 0, 0, 0));
        }
    }
}
