#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <catch2/catch_test_macros.hpp>

#include <SystemUtil.hpp>
#include <thread>
#include <type_traits>

static_assert(std::is_copy_constructible_v<sf::Clock>);
static_assert(std::is_copy_assignable_v<sf::Clock>);
static_assert(std::is_nothrow_move_constructible_v<sf::Clock>);
static_assert(std::is_nothrow_move_assignable_v<sf::Clock>);

TEST_CASE("[System] sf::Clock")
{
    SECTION("getElapsedTime()")
    {
        const sf::Clock clock;
        CHECK(clock.getElapsedTime() >= sf::microseconds(0));
        const auto elapsed = clock.getElapsedTime();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        CHECK(clock.getElapsedTime() > elapsed);
    }

    SECTION("restart()")
    {
        sf::Clock clock;
        CHECK(clock.restart() >= sf::microseconds(0));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        const auto elapsed = clock.restart();
        CHECK(clock.restart() < elapsed);
    }
}
