#include "SFML/Graphics/ConvexShape.hpp"

// Other 1st party headers
#include "SFML/Graphics/CircleShape.hpp"

#include "SFML/Base/SizeT.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[Graphics] sf::ConvexShape")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::ConvexShape));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::ConvexShape));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::ConvexShape));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::ConvexShape));
    }

    SECTION("Default constructor")
    {
        const sf::ConvexShape convex{{.pointCount = 0u}};
        CHECK(convex.getPointCount() == 0);
    }

    SECTION("Point count constructor")
    {
        const sf::ConvexShape convex{{.pointCount = 15u}};
        CHECK(convex.getPointCount() == 15);
        for (sf::base::SizeT i = 0; i < convex.getPointCount(); ++i)
            CHECK(convex.getPoint(i) == sf::Vector2f{0, 0});
    }

    SECTION("Set point count")
    {
        sf::ConvexShape convex{{.pointCount = 0u}};
        convex.setPointCount(42);
        CHECK(convex.getPointCount() == 42);
        for (sf::base::SizeT i = 0; i < convex.getPointCount(); ++i)
            CHECK(convex.getPoint(i) == sf::Vector2f{0, 0});
    }

    SECTION("Set point")
    {
        sf::ConvexShape convex{{.pointCount = 0u}};
        convex.setPointCount(1);
        convex.setPoint(0, {3, 4});
        CHECK(convex.getPoint(0) == sf::Vector2f{3, 4});
    }

    SECTION(
        "Construct clockwise ConvexShapes from CircleShapes to verify that they get approx. the same geometric center")
    {
        sf::ConvexShape convex{{.pointCount = 0u}};
        for (unsigned int i = 2; i < 10; ++i)
        {
            const sf::CircleShape circle{{.radius = 4.f, .pointCount = i}};
            convex.setPointCount(i);
            for (unsigned int j = 0; j < i; ++j)
            {
                convex.setPoint(j, circle.getPoint(j));
            }
            CHECK(convex.getGeometricCenter() == Approx(circle.getGeometricCenter()));
        }
    }

    SECTION(
        "Construct counterclockwise ConvexShapes from CircleShapes to verify that they get approx. the same geometric "
        "center")
    {
        sf::ConvexShape convex{{.pointCount = 0u}};
        for (unsigned int i = 2; i < 10; ++i)
        {
            const sf::CircleShape circle{{.radius = 4.f, .pointCount = i}};
            convex.setPointCount(i);
            for (unsigned int j = 0; j < i; ++j)
            {
                convex.setPoint(i - 1 - j, circle.getPoint(j));
            }
            CHECK(convex.getGeometricCenter() == Approx(circle.getGeometricCenter()));
        }
    }

    SECTION("Geometric center for one point")
    {
        sf::ConvexShape convex{{.pointCount = 1u}};
        convex.setPoint(0, {1.f, 1.f});
        CHECK(convex.getGeometricCenter() == sf::Vector2f(1.f, 1.f));
    }

    SECTION("Geometric center for two points")
    {
        sf::ConvexShape convex{{.pointCount = 2u}};
        convex.setPoint(0, {0.f, 0.f});
        convex.setPoint(1, {4.f, 2.f});
        CHECK(convex.getGeometricCenter() == sf::Vector2f(2.f, 1.f));
    }

    SECTION("Geometric center for three points with a small area")
    {
        sf::ConvexShape convex{{.pointCount = 3u}};
        convex.setPoint(0, {-100'000.f, 0.f});
        convex.setPoint(1, {100'000.f, 0.f});
        convex.setPoint(2, {100'000.f, 0.000001f});
        CHECK(convex.getGeometricCenter().x == Catch::Approx(100'000. / 3.).margin(1e-2));
        CHECK(convex.getGeometricCenter().y == Catch::Approx(0).margin(1e-5));
    }

    SECTION("Geometric center for aligned points")
    {
        SECTION("Geometric center for partly aligned points")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {-100.f, 0.f});
            convex.setPoint(1, {0.f, 0.f});
            convex.setPoint(2, {100.f, 1.f});
            CHECK(convex.getGeometricCenter() == Approx(sf::Vector2f(0.f, 1.f / 3.f)));
        }

        SECTION("Geometric center for aligned points with the two furthest apart not first and last")
        {
            sf::ConvexShape convex{{.pointCount = 4u}};
            convex.setPoint(0, {-50.f, -50.f});
            convex.setPoint(1, {-150.f, -150.f});
            convex.setPoint(2, {150.f, 150.f});
            convex.setPoint(3, {50.f, 50.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(0.f, 0.f));
        }

        SECTION("Geometric center for aligned points increasing x and y")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {1.f, 1.f});
            convex.setPoint(1, {5.f, 3.f});
            convex.setPoint(2, {9.f, 5.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(5.f, 3.f));
        }

        SECTION("Geometric center for aligned points increasing x, decreasing y")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {1.f, 5.f});
            convex.setPoint(1, {5.f, 3.f});
            convex.setPoint(2, {9.f, 1.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(5.f, 3.f));
        }

        SECTION("Geometric center for aligned points decreasing x and y")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {9.f, 5.f});
            convex.setPoint(1, {5.f, 3.f});
            convex.setPoint(2, {1.f, 1.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(5.f, 3.f));
        }

        SECTION("Geometric center for aligned points decreasing x, increasing y")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {9.f, 1.f});
            convex.setPoint(1, {5.f, 3.f});
            convex.setPoint(2, {1.f, 5.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(5.f, 3.f));
        }

        SECTION("Geometric center for aligned points with the same x value")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {1.f, 2.f});
            convex.setPoint(1, {1.f, 3.f});
            convex.setPoint(2, {1.f, 1.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(1.f, 2.f));
        }

        SECTION("Geometric center for aligned points with the same y value")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {2.f, 5.f});
            convex.setPoint(1, {3.f, 5.f});
            convex.setPoint(2, {1.f, 5.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(2.f, 5.f));
        }

        SECTION("Geometric center for aligned points out of order")
        {
            sf::ConvexShape convex{{.pointCount = 3u}};
            convex.setPoint(0, {5.f, 3.f});
            convex.setPoint(1, {1.f, 5.f});
            convex.setPoint(2, {9.f, 1.f});
            CHECK(convex.getGeometricCenter() == sf::Vector2f(5.f, 3.f));
        }
    }
}
