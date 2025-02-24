#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Angle.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <StringifyVectorUtil.hpp>

TEST_CASE("[Graphics] sf::Transform")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Transform));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Transform));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Transform));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Transform));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::Transform)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::Transform));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Transform));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Transform));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::Transform));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::Transform, sf::Transform));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            STATIC_CHECK(sf::Transform() == sf::Transform::Identity);
        }

        SECTION("3x3 matrix constructor")
        {
            constexpr sf::Transform transform(10.f, 11.f, 12.f, 13.f, 14.f, 15.f);

            // clang-format off
            float matrix[]{{},  {},  0.f, 0.f,
                           {},  {},  0.f, 0.f,
                           0.f, 0.f, 1.f, 0.f,
                           {},  {},  0.f, 1.f};
            // clang-format on

            transform.getMatrix(matrix);

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

        sf::Transform::Identity.getMatrix(matrix);

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
        STATIC_CHECK(sf::Transform::Identity.getInverse() == sf::Transform::Identity);
        STATIC_CHECK(sf::Transform(1.f, 2.f, 3.f, 4.f, 8.f, 6.f).getInverse() == sf::Transform::Identity);
        STATIC_CHECK(sf::Transform(1.f, 2.f, 3.f, 5.f, 2.f, 4.f).getInverse() ==
                     sf::Transform(-0.25f, 0.25f, -0.25f, 0.625f, -0.125f, -1.375f));
    }

    SECTION("transformPoint()")
    {
        STATIC_CHECK(sf::Transform::Identity.transformPoint({-10.f, -10.f}) == sf::Vector2f(-10.f, -10.f));
        STATIC_CHECK(sf::Transform::Identity.transformPoint({-1.f, -1.f}) == sf::Vector2f(-1.f, -1.f));
        STATIC_CHECK(sf::Transform::Identity.transformPoint({-1.f, 0.f}) == sf::Vector2f(-1.f, 0.f));
        STATIC_CHECK(sf::Transform::Identity.transformPoint({0.f, 0.f}) == sf::Vector2f(0.f, 0.f));
        STATIC_CHECK(sf::Transform::Identity.transformPoint({0.f, 1.f}) == sf::Vector2f(0.f, 1.f));
        STATIC_CHECK(sf::Transform::Identity.transformPoint({1.f, 1.f}) == sf::Vector2f(1.f, 1.f));
        STATIC_CHECK(sf::Transform::Identity.transformPoint({10.f, 10.f}) == sf::Vector2f(10.f, 10.f));

        constexpr sf::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
        STATIC_CHECK(transform.transformPoint({-1.f, -1.f}) == sf::Vector2f(0.f, -5.f));
        STATIC_CHECK(transform.transformPoint({0.f, 0.f}) == sf::Vector2f(3.f, 4.f));
        STATIC_CHECK(transform.transformPoint({1.f, 1.f}) == sf::Vector2f(6.f, 13.f));
    }

    SECTION("transformRect()")
    {
        STATIC_CHECK(sf::Transform::Identity.transformRect({{-200.f, -200.f}, {-100.f, -100.f}}) ==
                     sf::FloatRect({-300.f, -300.f}, {100.f, 100.f}));
        STATIC_CHECK(sf::Transform::Identity.transformRect({{0.f, 0.f}, {0.f, 0.f}}) ==
                     sf::FloatRect({0.f, 0.f}, {0.f, 0.f}));
        STATIC_CHECK(sf::Transform::Identity.transformRect({{100.f, 100.f}, {200.f, 200.f}}) ==
                     sf::FloatRect({100.f, 100.f}, {200.f, 200.f}));

        constexpr sf::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
        STATIC_CHECK(transform.transformRect({{-100.f, -100.f}, {200.f, 200.f}}) ==
                     sf::FloatRect({-297.f, -896.f}, {600.f, 1800.f}));
        STATIC_CHECK(transform.transformRect({{0.f, 0.f}, {0.f, 0.f}}) == sf::FloatRect({3.f, 4.f}, {0.f, 0.f}));
        STATIC_CHECK(transform.transformRect({{100.f, 100.f}, {200.f, 200.f}}) ==
                     sf::FloatRect({303.f, 904.f}, {600.f, 1800.f}));
    }

    SECTION("combine()")
    {
        auto identity = sf::Transform::Identity;
        CHECK(identity.combine(sf::Transform::Identity) == sf::Transform::Identity);
        CHECK(identity.combine(sf::Transform::Identity).combine(sf::Transform::Identity) == sf::Transform::Identity);

        sf::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
        CHECK(identity.combine(transform) == transform);
        CHECK(transform.combine(sf::Transform::Identity) == transform);
        CHECK(transform.combine(transform) == sf::Transform(9.f, 12.f, 14.f, 24.f, 33.f, 36.f));
        CHECK(transform.combine(sf::Transform(10.f, 2.f, 3.f, 4.f, 50.f, 40.f)) ==
              sf::Transform(138.f, 618.f, 521.f, 372.f, 1698.f, 1428.f));
    }

    SECTION("translate()")
    {
        sf::Transform transform(9, 8, 7, 6, 5, 4);
        CHECK(transform.translate({10.f, 20.f}) == sf::Transform(9, 8, 257, 6, 5, 164));
        CHECK(transform.translate({10.f, 20.f}) == sf::Transform(9, 8, 507, 6, 5, 324));
    }

    SECTION("rotate()")
    {
        SECTION("Around origin")
        {
            sf::Transform transform;
            transform.rotate(sf::degrees(90));
            CHECK(transform == Approx(sf::Transform(0, -1, 0, 1, 0, 0)));
        }

        SECTION("Around custom point")
        {
            sf::Transform transform;
            transform.rotate(sf::degrees(90), {1.f, 0.f});
            CHECK(transform == Approx(sf::Transform(0, -1, 1, 1, 0, -1)));
        }
    }

    SECTION("scale()")
    {
        SECTION("About origin")
        {
            sf::Transform transform(1, 2, 3, 4, 5, 4);
            CHECK(transform.scaleBy({2.f, 4.f}) == sf::Transform(2, 8, 3, 8, 20, 4));
            CHECK(transform.scaleBy({0.f, 0.f}) == sf::Transform(0, 0, 3, 0, 0, 4));
            CHECK(transform.scaleBy({10.f, 10.f}) == sf::Transform(0, 0, 3, 0, 0, 4));
        }

        SECTION("About custom point")
        {
            sf::Transform transform(1, 2, 3, 4, 5, 4);
            CHECK(transform.scaleBy({1.f, 2.f}, {1.f, 0.f}) == sf::Transform(1, 4, 3, 4, 10, 4));
            CHECK(transform.scaleBy({0.f, 0.f}, {1.f, 0.f}) == sf::Transform(0, 0, 4, 0, 0, 8));
        }
    }

    SECTION("Operators")
    {
        SECTION("operator*")
        {
            STATIC_CHECK(sf::Transform::Identity * sf::Transform::Identity == sf::Transform::Identity);
            STATIC_CHECK(sf::Transform::Identity * sf::Transform::Identity * sf::Transform::Identity ==
                         sf::Transform::Identity);

            constexpr sf::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
            STATIC_CHECK(sf::Transform::Identity * transform == transform);
            STATIC_CHECK(transform * sf::Transform::Identity == transform);
            STATIC_CHECK(transform * transform == sf::Transform(9.f, 12.f, 14.f, 24.f, 33.f, 36.f));
            STATIC_CHECK(transform * sf::Transform(10.f, 2.f, 3.f, 4.f, 50.f, 40.f) ==
                         sf::Transform(18.f, 102.f, 86.f, 60.f, 258.f, 216.f));
        }

        SECTION("operator*=")
        {
            sf::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
            transform *= sf::Transform::Identity;
            CHECK(transform == sf::Transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f));
            transform *= transform;
            CHECK(transform == sf::Transform(9.f, 12.f, 14.f, 24.f, 33.f, 36.f));
            transform *= sf::Transform(10.f, 2.f, 3.f, 4.f, 50.f, 40.f);
            CHECK(transform == sf::Transform(138.f, 618.f, 521.f, 372.f, 1698.f, 1428.f));
        }

        SECTION("operator* with vector")
        {
            STATIC_CHECK(sf::Transform::Identity * sf::Vector2f(-10.f, -10.f) == sf::Vector2f(-10.f, -10.f));
            STATIC_CHECK(sf::Transform::Identity * sf::Vector2f(-1.f, -1.f) == sf::Vector2f(-1.f, -1.f));
            STATIC_CHECK(sf::Transform::Identity * sf::Vector2f(-1.f, 0.f) == sf::Vector2f(-1.f, 0.f));
            STATIC_CHECK(sf::Transform::Identity * sf::Vector2f(0.f, 0.f) == sf::Vector2f(0.f, 0.f));
            STATIC_CHECK(sf::Transform::Identity * sf::Vector2f(0.f, 1.f) == sf::Vector2f(0.f, 1.f));
            STATIC_CHECK(sf::Transform::Identity * sf::Vector2f(1.f, 1.f) == sf::Vector2f(1.f, 1.f));
            STATIC_CHECK(sf::Transform::Identity * sf::Vector2f(10.f, 10.f) == sf::Vector2f(10.f, 10.f));

            constexpr sf::Transform transform(1.f, 2.f, 3.f, 4.f, 5.f, 4.f);
            STATIC_CHECK(transform * sf::Vector2f(-1.f, -1.f) == sf::Vector2f(0.f, -5.f));
            STATIC_CHECK(transform * sf::Vector2f(0.f, 0.f) == sf::Vector2f(3.f, 4.f));
            STATIC_CHECK(transform * sf::Vector2f(1.f, 1.f) == sf::Vector2f(6.f, 13.f));
        }

        SECTION("operator==")
        {
            STATIC_CHECK(sf::Transform::Identity == sf::Transform::Identity);
            STATIC_CHECK(sf::Transform() == sf::Transform());
            STATIC_CHECK(sf::Transform(0, 0, 0, 0, 0, 0) == sf::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(sf::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f) ==
                         sf::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f));
            STATIC_CHECK(sf::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f) ==
                         sf::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f));
        }

        SECTION("operator!=")
        {
            STATIC_CHECK_FALSE(sf::Transform::Identity != sf::Transform::Identity);
            STATIC_CHECK_FALSE(sf::Transform() != sf::Transform());
            STATIC_CHECK_FALSE(sf::Transform(0, 0, 0, 0, 0, 0) != sf::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK_FALSE(sf::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f) !=
                               sf::Transform(0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f));
            STATIC_CHECK_FALSE(sf::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f) !=
                               sf::Transform(1000.f, 1000.f, 1000.f, 1000.f, 1000.f, 1000.f));

            STATIC_CHECK(sf::Transform(1, 0, 0, 0, 0, 0) != sf::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(sf::Transform(0, 1, 0, 0, 0, 0) != sf::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(sf::Transform(0, 0, 1, 0, 0, 0) != sf::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(sf::Transform(0, 0, 0, 1, 0, 0) != sf::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(sf::Transform(0, 0, 0, 0, 1, 0) != sf::Transform(0, 0, 0, 0, 0, 0));
            STATIC_CHECK(sf::Transform(0, 0, 0, 0, 0, 1) != sf::Transform(0, 0, 0, 0, 0, 0));
        }
    }
}
