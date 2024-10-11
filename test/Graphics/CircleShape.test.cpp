#include "SFML/Graphics/CircleShape.hpp"

#include "SFML/Base/SizeT.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[Graphics] sf::CircleShape")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::CircleShape));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::CircleShape));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::CircleShape));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::CircleShape));
    }

    SECTION("Default constructor")
    {
        const sf::CircleShape circle{{.radius = 0.f}};
        CHECK(circle.getRadius() == 0.f);
        CHECK(circle.getPointCount() == 30);
        for (sf::base::SizeT i = 0; i < circle.getPointCount(); ++i)
            CHECK(circle.getPoint(i) == sf::Vector2f{0, 0});
        CHECK(circle.getGeometricCenter() == sf::Vector2f{0, 0});
    }

    SECTION("Radius constructor")
    {
        const sf::CircleShape circle{{.radius = 15.f}};
        CHECK(circle.getRadius() == 15.f);
        CHECK(circle.getPointCount() == 30);
        CHECK(circle.getPoint(0) == Approx(sf::Vector2f(15.000000000f, 30.000000000f)));
        CHECK(circle.getPoint(1) == Approx(sf::Vector2f(18.120029449f, 29.671918869f)));
        CHECK(circle.getPoint(2) == Approx(sf::Vector2f(21.102930069f, 28.702350616f)));
        CHECK(circle.getPoint(3) == Approx(sf::Vector2f(23.817825317f, 27.134489059f)));
        CHECK(circle.getPoint(4) == Approx(sf::Vector2f(26.148479462f, 25.035495758f)));
        CHECK(circle.getPoint(5) == Approx(sf::Vector2f(27.990974426f, 22.498964310f)));
        CHECK(circle.getPoint(6) == Approx(sf::Vector2f(29.266424179f, 19.633468628f)));
        CHECK(circle.getPoint(7) == Approx(sf::Vector2f(29.917949677f, 16.566825867f)));
        CHECK(circle.getPoint(8) == Approx(sf::Vector2f(29.917648315f, 13.430310249f)));
        CHECK(circle.getPoint(9) == Approx(sf::Vector2f(29.265539169f, 10.363785744f)));
        CHECK(circle.getPoint(10) == Approx(sf::Vector2f(27.989534378f, 7.498545170f)));
        CHECK(circle.getPoint(11) == Approx(sf::Vector2f(26.146560669f, 4.962360382f)));
        CHECK(circle.getPoint(12) == Approx(sf::Vector2f(23.815500259f, 2.863815069f)));
        CHECK(circle.getPoint(13) == Approx(sf::Vector2f(21.100305557f, 1.296479702f)));
        CHECK(circle.getPoint(14) == Approx(sf::Vector2f(18.117225647f, 0.327479839f)));
        CHECK(circle.getPoint(15) == Approx(sf::Vector2f(15.000000000f, 0.000000000f)));
        CHECK(circle.getPoint(16) == Approx(sf::Vector2f(11.879970551f, 0.328079760f)));
        CHECK(circle.getPoint(17) == Approx(sf::Vector2f(8.897069931f, 1.297650099f)));
        CHECK(circle.getPoint(18) == Approx(sf::Vector2f(6.182175159f, 2.865510225f)));
        CHECK(circle.getPoint(19) == Approx(sf::Vector2f(3.851504564f, 4.964505196f)));
        CHECK(circle.getPoint(20) == Approx(sf::Vector2f(2.009010315f, 7.501035213f)));
        CHECK(circle.getPoint(21) == Approx(sf::Vector2f(0.733575225f, 10.366530418f)));
        CHECK(circle.getPoint(22) == Approx(sf::Vector2f(0.082049668f, 13.433175087f)));
        CHECK(circle.getPoint(23) == Approx(sf::Vector2f(0.082350075f, 16.569688797f)));
        CHECK(circle.getPoint(24) == Approx(sf::Vector2f(0.734460354f, 19.636215210f)));
        CHECK(circle.getPoint(25) == Approx(sf::Vector2f(2.010464907f, 22.501455307f)));
        CHECK(circle.getPoint(26) == Approx(sf::Vector2f(3.853440285f, 25.037639618f)));
        CHECK(circle.getPoint(27) == Approx(sf::Vector2f(6.184499741f, 27.136184692f)));
        CHECK(circle.getPoint(28) == Approx(sf::Vector2f(8.899694443f, 28.703519821f)));
        CHECK(circle.getPoint(29) == Approx(sf::Vector2f(11.882775307f, 29.672519684f)));
        CHECK(circle.getGeometricCenter() == sf::Vector2f(15.f, 15.f));
    }

    SECTION("Radius and point count constructor")
    {
        const sf::CircleShape circle{{.radius = 5.f, .pointCount = 8}};
        CHECK(circle.getRadius() == 5.f);
        CHECK(circle.getPointCount() == 8);
        CHECK(circle.getPoint(0) == Approx(sf::Vector2f(5.000000000f, 10.000000000f)));
        CHECK(circle.getPoint(1) == Approx(sf::Vector2f(8.535533905f, 8.535533905f)));
        CHECK(circle.getPoint(2) == Approx(sf::Vector2f(10.000000000f, 5.000000000f)));
        CHECK(circle.getPoint(3) == Approx(sf::Vector2f(8.535533905f, 1.464466095f)));
        CHECK(circle.getPoint(4) == Approx(sf::Vector2f(5.000000000f, 0.000000000f)));
        CHECK(circle.getPoint(5) == Approx(sf::Vector2f(1.464466095f, 1.464466095f)));
        CHECK(circle.getPoint(6) == Approx(sf::Vector2f(0.000000000f, 4.999999523f)));
        CHECK(circle.getPoint(7) == Approx(sf::Vector2f(1.464466095f, 8.535533905f)));
        CHECK(circle.getGeometricCenter() == sf::Vector2f(5.f, 5.f));
    }

    SECTION("Set radius")
    {
        sf::CircleShape circle{{.radius = 1.f, .pointCount = 6}};
        circle.setRadius(10.f);
        CHECK(circle.getRadius() == 10.f);
        CHECK(circle.getPointCount() == 6);
        CHECK(circle.getPoint(0) == Approx(sf::Vector2f(10.000000000f, 20.000000000f)));
        CHECK(circle.getPoint(1) == Approx(sf::Vector2f(18.660650253f, 14.999309540f)));
        CHECK(circle.getPoint(2) == Approx(sf::Vector2f(18.659688950f, 4.999030113f)));
        CHECK(circle.getPoint(3) == Approx(sf::Vector2f(10.000000000f, 0.000000000f)));
        CHECK(circle.getPoint(4) == Approx(sf::Vector2f(1.339340210f, 5.000690460f)));
        CHECK(circle.getPoint(5) == Approx(sf::Vector2f(1.340309978f, 15.000970840f)));
        CHECK(circle.getGeometricCenter() == sf::Vector2f(10.f, 10.f));
    }

    SECTION("Set point count")
    {
        sf::CircleShape circle{{.radius = 4.f, .pointCount = 10}};
        circle.setPointCount(4);
        CHECK(circle.getRadius() == 4.f);
        CHECK(circle.getPointCount() == 4);
        CHECK(circle.getPoint(0) == Approx(sf::Vector2f(4.000000000f, 8.000000000f)));
        CHECK(circle.getPoint(1) == Approx(sf::Vector2f(8.000000000f, 4.000000000f)));
        CHECK(circle.getPoint(2) == Approx(sf::Vector2f(4.000000000f, 0.000000000f)));
        CHECK(circle.getPoint(3) == Approx(sf::Vector2f(0.000000000f, 3.999999762f)));
        CHECK(circle.getGeometricCenter() == sf::Vector2f(4.f, 4.f));
    }

    SECTION("Equilateral triangle")
    {
        sf::CircleShape triangle{{.radius = 2.f, .pointCount = 3}};
        CHECK(triangle.getRadius() == 2.f);
        CHECK(triangle.getPointCount() == 3);
        CHECK(triangle.getPoint(0) == Approx(sf::Vector2f(2.000000000f, 4.000000000f)));
        CHECK(triangle.getPoint(1) == Approx(sf::Vector2f(3.731937885f, 0.999806046f)));
        CHECK(triangle.getPoint(2) == Approx(sf::Vector2f(0.267868042f, 1.000138044f)));
        CHECK(triangle.getGeometricCenter() == sf::Vector2f(2.f, 2.f));
    }

    SECTION("Geometric center")
    {
        SECTION("2 points")
        {
            CHECK(sf::CircleShape{{.radius = 2.f, .pointCount = 2}}.getGeometricCenter() == sf::Vector2f(2.f, 2.f));
        }

        SECTION("3 points")
        {
            CHECK(sf::CircleShape{{.radius = 4.f, .pointCount = 3}}.getGeometricCenter() == sf::Vector2f(4.f, 4.f));
        }
    }
}
