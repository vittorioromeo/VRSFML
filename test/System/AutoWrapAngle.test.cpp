#include "SystemUtil.hpp"

#include "SFML/System/AutoWrapAngle.hpp"

#include "SFML/System/Angle.hpp"

#include "SFML/Base/Trait/IsAggregate.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"

#include <Doctest.hpp>


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
