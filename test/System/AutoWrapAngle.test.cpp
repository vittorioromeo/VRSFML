#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/System/AutoWrapAngle.hpp"

#include "Zancle/System/Angle.hpp"

#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"


TEST_CASE("[System] za::AutoWrapAngle")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::AutoWrapAngle));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::AutoWrapAngle));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::AutoWrapAngle));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::AutoWrapAngle));

        STATIC_CHECK(!ZB_IS_AGGREGATE(za::AutoWrapAngle));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::AutoWrapAngle));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::AutoWrapAngle));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(za::AutoWrapAngle, za::AutoWrapAngle));
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
