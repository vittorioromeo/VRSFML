#include "SFML/System/AutoWrapAngle.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[System] sf::AutoWrapAngle")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::AutoWrapAngle));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::AutoWrapAngle));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::AutoWrapAngle));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::AutoWrapAngle));

        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::AutoWrapAngle));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::AutoWrapAngle));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::AutoWrapAngle));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::AutoWrapAngle, sf::AutoWrapAngle));
    }

    SECTION("Construction")
    {
        constexpr sf::AutoWrapAngle angle;
        STATIC_CHECK(angle.asDegrees() == 0.f);
        STATIC_CHECK(angle.asRadians() == 0.f);
    }

    SECTION("Wrapping")
    {
        STATIC_CHECK(sf::AutoWrapAngle{sf::degrees(360.f)}.asRadians() == 0.f);
        STATIC_CHECK(sf::AutoWrapAngle{sf::degrees(180.f)}.asDegrees() == 180.f);
        CHECK(sf::AutoWrapAngle{sf::degrees(360.f + 180.f)}.asDegrees() == Approx(180.f));
    }
}
