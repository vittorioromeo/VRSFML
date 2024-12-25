#include "SFML/System/Clock.hpp"

#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

TEST_CASE("[System] sf::Clock")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Clock));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Clock));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Clock));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Clock));
    }

    SECTION("Construction")
    {
        const sf::Clock clock;
        CHECK(clock.isRunning());
        CHECK(clock.getElapsedTime() >= sf::microseconds(0));
    }

    SECTION("getElapsedTime()")
    {
        const sf::Clock clock;
        CHECK(clock.getElapsedTime() >= sf::microseconds(0));
        const auto elapsed = clock.getElapsedTime();
        sf::sleep(sf::milliseconds(1));
        CHECK(clock.getElapsedTime() > elapsed);
    }

    SECTION("start/stop")
    {
        sf::Clock clock;
        clock.stop();
        CHECK(!clock.isRunning());
        const auto elapsed = clock.getElapsedTime();
        sf::sleep(sf::milliseconds(1));
        CHECK(elapsed == clock.getElapsedTime());

        clock.start();
        CHECK(clock.isRunning());
        CHECK(clock.getElapsedTime() >= elapsed);
    }

    SECTION("restart()")
    {
        sf::Clock clock;
        CHECK(clock.restart() >= sf::microseconds(0));
        CHECK(clock.isRunning());
        sf::sleep(sf::milliseconds(1));
        const auto elapsed = clock.restart();
        CHECK(clock.restart() < elapsed);
    }

    SECTION("reset()")
    {
        sf::Clock clock;
        CHECK(clock.reset() >= sf::microseconds(0));
        CHECK(!clock.isRunning());
    }
}
