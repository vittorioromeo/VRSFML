#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Geometry/AutoWrapAngle.hpp"

#include "Zancle/Geometry/Angle.hpp"

#include "Zancle/Trait/IsAggregate.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"
#include "Zancle/Trait/IsTriviallyAssignable.hpp"
#include "Zancle/Trait/IsTriviallyCopyable.hpp"
#include "Zancle/Trait/IsTriviallyDestructible.hpp"


TEST_CASE("[System] za::AutoWrapAngle")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::AutoWrapAngle));
        STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::AutoWrapAngle));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::AutoWrapAngle));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::AutoWrapAngle));

        STATIC_CHECK(!ZA_IS_AGGREGATE(za::AutoWrapAngle));
        STATIC_CHECK(ZA_IS_TRIVIALLY_COPYABLE(za::AutoWrapAngle));
        STATIC_CHECK(ZA_IS_TRIVIALLY_DESTRUCTIBLE(za::AutoWrapAngle));
        STATIC_CHECK(ZA_IS_TRIVIALLY_ASSIGNABLE(za::AutoWrapAngle, za::AutoWrapAngle));
    }

    SECTION("Construction")
    {
        constexpr za::AutoWrapAngle angle;
        STATIC_CHECK(angle.asDegrees() == 0.f);
        STATIC_CHECK(angle.asRadians() == 0.f);
    }

    SECTION("Wrapping")
    {
        STATIC_CHECK(za::AutoWrapAngle{za::degrees(360.f)}.asRadians() == 0.f);
        STATIC_CHECK(za::AutoWrapAngle{za::degrees(180.f)}.asDegrees() == 180.f);
        CHECK(za::AutoWrapAngle{za::degrees(360.f + 180.f)}.asDegrees() == Approx(180.f));
    }
}
