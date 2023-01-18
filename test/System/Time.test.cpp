#include <SFML/System/Time.hpp>

#include <catch2/catch_test_macros.hpp>

#include <SystemUtil.hpp>
#include <type_traits>

static_assert(std::is_copy_constructible_v<sf::Time>);
static_assert(std::is_copy_assignable_v<sf::Time>);
static_assert(std::is_nothrow_move_constructible_v<sf::Time>);
static_assert(std::is_nothrow_move_assignable_v<sf::Time>);

using namespace std::chrono_literals;

TEST_CASE("[System] sf::Time")
{
    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const sf::Time time;
            CHECK(time.asSeconds() == 0.0f);
            CHECK(time.asMilliseconds() == 0);
            CHECK(time.asMicroseconds() == 0);
        }

        SECTION("Construct from seconds")
        {
            const sf::Time time = sf::seconds(123);
            CHECK(time.asSeconds() == 123.0f);
            CHECK(time.asMilliseconds() == 123'000);
            CHECK(time.asMicroseconds() == 123'000'000);

            CHECK(sf::seconds(1'000.0f).asMicroseconds() == 1'000'000'000);
            CHECK(sf::seconds(0.0000009f).asMicroseconds() == 0);
            CHECK(sf::seconds(0.0000001f).asMicroseconds() == 0);
            CHECK(sf::seconds(0.00000001f).asMicroseconds() == 0);
            CHECK(sf::seconds(0.000000001f).asMicroseconds() == 0);
            CHECK(sf::seconds(-0.000000001f).asMicroseconds() == 0);
            CHECK(sf::seconds(-0.00000001f).asMicroseconds() == 0);
            CHECK(sf::seconds(-0.0000001f).asMicroseconds() == 0);
            CHECK(sf::seconds(-0.0000009f).asMicroseconds() == 0);
            CHECK(sf::seconds(-1'000.0f).asMicroseconds() == -1'000'000'000);
        }

        SECTION("Construct from milliseconds")
        {
            const sf::Time time = sf::milliseconds(42);
            CHECK(time.asSeconds() == 0.042f);
            CHECK(time.asMilliseconds() == 42);
            CHECK(time.asMicroseconds() == 42'000);
        }

        SECTION("Construct from microseconds")
        {
            const sf::Time time = sf::microseconds(987654);
            CHECK(time.asSeconds() == 0.987654f);
            CHECK(time.asMilliseconds() == 987);
            CHECK(time.asMicroseconds() == 987'654);
        }

        SECTION("Convert from chrono duration")
        {
            {
                const sf::Time time = 3min;
                CHECK(time.asSeconds() == 180.f);
                CHECK(time.asMilliseconds() == 180'000);
                CHECK(time.asMicroseconds() == 180'000'000);
            }
            {
                const sf::Time time = 1s;
                CHECK(time.asSeconds() == 1.f);
                CHECK(time.asMilliseconds() == 1'000);
                CHECK(time.asMicroseconds() == 1'000'000);
            }
            {
                const sf::Time time = 10ms;
                CHECK(time.asSeconds() == 0.01f);
                CHECK(time.asMilliseconds() == 10);
                CHECK(time.asMicroseconds() == 10'000);
            }
            {
                const sf::Time time = 2048us;
                CHECK(time.asSeconds() == 0.002048f);
                CHECK(time.asMilliseconds() == 2);
                CHECK(time.asMicroseconds() == 2048);
            }
        }
    }

    SECTION("toDuration()")
    {
        CHECK(sf::seconds(0).toDuration() == 0s);
        CHECK(sf::milliseconds(0).toDuration() == 0ms);
        CHECK(sf::microseconds(0).toDuration() == 0us);

        CHECK(sf::seconds(-1).toDuration() == -1s);
        CHECK(sf::milliseconds(-1).toDuration() == -1ms);
        CHECK(sf::microseconds(-1).toDuration() == -1us);

        CHECK(sf::seconds(1).toDuration() == 1s);
        CHECK(sf::milliseconds(1).toDuration() == 1ms);
        CHECK(sf::microseconds(1).toDuration() == 1us);

        CHECK(sf::seconds(-10).toDuration() == -10s);
        CHECK(sf::milliseconds(-10).toDuration() == -10ms);
        CHECK(sf::microseconds(-10).toDuration() == -10us);

        CHECK(sf::seconds(10).toDuration() == 10s);
        CHECK(sf::milliseconds(10).toDuration() == 10ms);
        CHECK(sf::microseconds(10).toDuration() == 10us);

        CHECK(sf::Time(1s).toDuration() == 1s);
        CHECK(sf::Time(1ms).toDuration() == 1ms);
        CHECK(sf::Time(1us).toDuration() == 1us);
    }

    SECTION("Implicit conversion to duration")
    {
        const auto toDuration = [](const std::chrono::microseconds& duration) { return duration; };
        CHECK(toDuration(sf::seconds(0)) == 0s);
        CHECK(toDuration(sf::milliseconds(0)) == 0ms);
        CHECK(toDuration(sf::microseconds(0)) == 0us);

        CHECK(toDuration(sf::seconds(-1)) == -1s);
        CHECK(toDuration(sf::milliseconds(-1)) == -1ms);
        CHECK(toDuration(sf::microseconds(-1)) == -1us);

        CHECK(toDuration(sf::seconds(1)) == 1s);
        CHECK(toDuration(sf::milliseconds(1)) == 1ms);
        CHECK(toDuration(sf::microseconds(1)) == 1us);

        CHECK(toDuration(sf::seconds(-10)) == -10s);
        CHECK(toDuration(sf::milliseconds(-10)) == -10ms);
        CHECK(toDuration(sf::microseconds(-10)) == -10us);

        CHECK(toDuration(sf::seconds(10)) == 10s);
        CHECK(toDuration(sf::milliseconds(10)) == 10ms);
        CHECK(toDuration(sf::microseconds(10)) == 10us);

        CHECK(toDuration(sf::Time(1s)) == 1s);
        CHECK(toDuration(sf::Time(1ms)) == 1ms);
        CHECK(toDuration(sf::Time(1us)) == 1us);
    }

    SECTION("Zero time")
    {
        CHECK(sf::Time::Zero.asSeconds() == 0.0f);
        CHECK(sf::Time::Zero.asMilliseconds() == 0);
        CHECK(sf::Time::Zero.asMicroseconds() == 0);
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(sf::Time() == sf::Time());
            CHECK(sf::seconds(1) == sf::seconds(1));
            CHECK(sf::seconds(10) == sf::milliseconds(10'000));
            CHECK(sf::milliseconds(450'450) == sf::microseconds(450'450'000));
            CHECK(sf::seconds(0.5f) == sf::microseconds(500'000));
        }

        SECTION("operator!=")
        {
            CHECK(sf::seconds(10.12f) != sf::milliseconds(10'121));
            CHECK(sf::microseconds(123'456) != sf::milliseconds(123));
        }

        SECTION("operator<")
        {
            CHECK(sf::seconds(54.999f) < sf::seconds(55));
            CHECK(sf::microseconds(10) < sf::milliseconds(10));
            CHECK(sf::milliseconds(1'000) < sf::microseconds(1'000'001));
        }

        SECTION("operator>")
        {
            CHECK(sf::seconds(55.001f) > sf::seconds(55));
            CHECK(sf::microseconds(1) > sf::seconds(0.0000001f));
            CHECK(sf::microseconds(1'000'001) > sf::milliseconds(1'000));
        }

        SECTION("operator<=")
        {
            CHECK(sf::milliseconds(100) <= sf::milliseconds(100));
            CHECK(sf::seconds(0.0012f) <= sf::microseconds(1'201));
        }

        SECTION("operator>=")
        {
            CHECK(sf::milliseconds(100) >= sf::milliseconds(-100));
            CHECK(sf::microseconds(1'201) >= sf::seconds(0.0012f));
        }

        SECTION("operator-")
        {
            CHECK(sf::seconds(-1) == -sf::seconds(1));
            CHECK(sf::microseconds(1'234) == -sf::microseconds(-1'234));
        }

        SECTION("operator+")
        {
            CHECK(sf::seconds(1) + sf::seconds(1) == sf::seconds(2));
            CHECK(sf::milliseconds(400) + sf::microseconds(400) == sf::microseconds(400400));
        }

        SECTION("operator+=")
        {
            sf::Time time = sf::seconds(1.5f);
            time += sf::seconds(1);
            CHECK(time == sf::seconds(2.5f));
        }

        SECTION("operator-")
        {
            CHECK(sf::seconds(1) - sf::seconds(1) == sf::seconds(0));
            CHECK(sf::milliseconds(400) - sf::microseconds(400) == sf::microseconds(399600));
        }

        SECTION("operator-=")
        {
            sf::Time time = sf::seconds(1.5f);
            time -= sf::seconds(10);
            CHECK(time == sf::seconds(-8.5f));
        }

        SECTION("operator*")
        {
            CHECK(sf::seconds(1) * 2.0f == sf::seconds(2));
            CHECK(sf::seconds(12) * 0.5f == sf::seconds(6));
            CHECK(sf::seconds(1) * static_cast<std::int64_t>(2) == sf::seconds(2));
            CHECK(sf::seconds(42) * static_cast<std::int64_t>(2) == sf::seconds(84));
            CHECK(2.0f * sf::seconds(1) == sf::seconds(2));
            CHECK(0.5f * sf::seconds(12) == sf::seconds(6));
            CHECK(static_cast<std::int64_t>(2) * sf::seconds(1) == sf::seconds(2));
            CHECK(static_cast<std::int64_t>(2) * sf::seconds(42) == sf::seconds(84));
        }

        SECTION("operator*=")
        {
            sf::Time time = sf::milliseconds(1'000);
            time *= static_cast<std::int64_t>(10);
            CHECK(time == sf::milliseconds(10'000));
            time *= 0.1f;
            CHECK(time.asMilliseconds() == 1'000);
        }

        SECTION("operator/")
        {
            CHECK(sf::seconds(1) / 2.0f == sf::seconds(0.5f));
            CHECK(sf::seconds(12) / 0.5f == sf::seconds(24));
            CHECK(sf::seconds(1) / static_cast<std::int64_t>(2) == sf::seconds(0.5f));
            CHECK(sf::seconds(42) / static_cast<std::int64_t>(2) == sf::seconds(21));
            CHECK(sf::seconds(1) / sf::seconds(1) == 1.0f);
            CHECK(sf::milliseconds(10) / sf::microseconds(1) == Approx(10'000.f));
        }

        SECTION("operator/=")
        {
            sf::Time time = sf::milliseconds(1'000);
            time /= static_cast<std::int64_t>(2);
            CHECK(time == sf::milliseconds(500));
            time /= 0.5f;
            CHECK(time.asMilliseconds() == 1'000);
        }

        SECTION("operator%")
        {
            CHECK(sf::seconds(10) % sf::seconds(3) == sf::seconds(1));
            CHECK(sf::milliseconds(100) % sf::microseconds(10) == sf::seconds(0));
        }

        SECTION("operator%=")
        {
            sf::Time time = sf::milliseconds(100);
            time %= sf::milliseconds(99);
            CHECK(time == sf::milliseconds(1));
        }
    }

    SECTION("Constexpr support")
    {
        constexpr auto result = []
        {
            sf::Time time = sf::milliseconds(100);
            time %= sf::milliseconds(99);
            return time;
        }();

        static_assert(result == sf::milliseconds(1));
    }
}
