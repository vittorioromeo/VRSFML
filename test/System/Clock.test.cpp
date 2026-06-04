#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/System/Clock.hpp"

#include "Zancle/System/Thread.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[System] za::Clock")
{
    SECTION("Type traits")
    {
        // cannot be trivially copiable/movable due to pimpl
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::Clock));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::Clock));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Clock));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Clock));
    }

    SECTION("Construction")
    {
        const za::Clock clock;
        CHECK(clock.isRunning());
        CHECK(clock.getElapsedTime() >= za::microseconds(0));
    }

    SECTION("getElapsedTime()")
    {
        const za::Clock clock;
        CHECK(clock.getElapsedTime() >= za::microseconds(0));
        const auto elapsed = clock.getElapsedTime();
        za::ThisThread::sleepFor(za::milliseconds(1));
        CHECK(clock.getElapsedTime() > elapsed);
    }

    SECTION("start/stop")
    {
        za::Clock clock;
        clock.stop();
        CHECK(!clock.isRunning());
        const auto elapsed = clock.getElapsedTime();
        za::ThisThread::sleepFor(za::milliseconds(1));
        CHECK(elapsed == clock.getElapsedTime());

        clock.start();
        CHECK(clock.isRunning());
        CHECK(clock.getElapsedTime() >= elapsed);
    }

    SECTION("restart()")
    {
        za::Clock clock;
        CHECK(clock.restart() >= za::microseconds(0));
        CHECK(clock.isRunning());
        za::ThisThread::sleepFor(za::milliseconds(1));
        const auto elapsed = clock.restart();
        CHECK(clock.restart() < elapsed);
    }

    SECTION("reset()")
    {
        za::Clock clock;
        CHECK(clock.reset() >= za::microseconds(0));
        CHECK(!clock.isRunning());
    }
}
