#include "SFML/System/Time.hpp"

#include "SFML/System/TimeChronoUtil.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("[System] sf::Time")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::Time));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::Time));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::Time));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::Time));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Time));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            constexpr sf::Time time;
            STATIC_CHECK(time.asSeconds() == 0.0f);
            STATIC_CHECK(time.asMilliseconds() == 0);
            STATIC_CHECK(time.asMicroseconds() == 0);
        }

        SECTION("Construct from seconds")
        {
            constexpr sf::Time time = sf::seconds(123);
            STATIC_CHECK(time.asSeconds() == 123.0f);
            STATIC_CHECK(time.asMilliseconds() == 123'000);
            STATIC_CHECK(time.asMicroseconds() == 123'000'000);

            STATIC_CHECK(sf::seconds(1'000.0f).asMicroseconds() == 1'000'000'000);
            STATIC_CHECK(sf::seconds(0.0000009f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(0.0000001f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(0.00000001f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(0.000000001f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(-0.000000001f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(-0.00000001f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(-0.0000001f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(-0.0000009f).asMicroseconds() == 0);
            STATIC_CHECK(sf::seconds(-1'000.0f).asMicroseconds() == -1'000'000'000);
        }

        SECTION("Construct from milliseconds")
        {
            constexpr sf::Time time = sf::milliseconds(42);
            CHECK(time.asSeconds() == Approx(0.042f));
            STATIC_CHECK(time.asMilliseconds() == 42);
            STATIC_CHECK(time.asMicroseconds() == 42'000);
        }

        SECTION("Construct from microseconds")
        {
            constexpr sf::Time time = sf::microseconds(987654);
            CHECK(time.asSeconds() == Approx(0.987654f));
            STATIC_CHECK(time.asMilliseconds() == 987);
            STATIC_CHECK(time.asMicroseconds() == 987'654);
        }

        SECTION("Convert from chrono duration")
        {
            {
                constexpr sf::Time time = sf::TimeChronoUtil::fromDuration(3min);
                STATIC_CHECK(time.asSeconds() == 180.f);
                STATIC_CHECK(time.asMilliseconds() == 180'000);
                STATIC_CHECK(time.asMicroseconds() == 180'000'000);
            }
            {
                constexpr sf::Time time = sf::TimeChronoUtil::fromDuration(1s);
                STATIC_CHECK(time.asSeconds() == 1.f);
                STATIC_CHECK(time.asMilliseconds() == 1'000);
                STATIC_CHECK(time.asMicroseconds() == 1'000'000);
            }
            {
                constexpr sf::Time time = sf::TimeChronoUtil::fromDuration(10ms);
                CHECK(time.asSeconds() == Approx(0.01f));
                STATIC_CHECK(time.asMilliseconds() == 10);
                STATIC_CHECK(time.asMicroseconds() == 10'000);
            }
            {
                constexpr sf::Time time = sf::TimeChronoUtil::fromDuration(2048us);
                CHECK(time.asSeconds() == Approx(0.002048f));
                STATIC_CHECK(time.asMilliseconds() == 2);
                STATIC_CHECK(time.asMicroseconds() == 2048);
            }
        }
    }

    SECTION("toDuration()")
    {
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(0)) == 0s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(0)) == 0ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(0)) == 0us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(-1)) == -1s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(-1)) == -1ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(-1)) == -1us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(1)) == 1s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(1)) == 1ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(1)) == 1us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(-10)) == -10s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(-10)) == -10ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(-10)) == -10us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(10)) == 10s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(10)) == 10ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(10)) == 10us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::TimeChronoUtil::fromDuration(1s)) == 1s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::TimeChronoUtil::fromDuration(1ms)) == 1ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::TimeChronoUtil::fromDuration(1us)) == 1us);
    }

    SECTION("Conversion to duration")
    {
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(0)) == 0s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(0)) == 0ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(0)) == 0us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(-1)) == -1s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(-1)) == -1ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(-1)) == -1us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(1)) == 1s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(1)) == 1ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(1)) == 1us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(-10)) == -10s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(-10)) == -10ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(-10)) == -10us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::seconds(10)) == 10s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::milliseconds(10)) == 10ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::microseconds(10)) == 10us);

        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::TimeChronoUtil::fromDuration(1s)) == 1s);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::TimeChronoUtil::fromDuration(1ms)) == 1ms);
        STATIC_CHECK(sf::TimeChronoUtil::toDuration(sf::TimeChronoUtil::fromDuration(1us)) == 1us);
    }

    SECTION("Zero time")
    {
        STATIC_CHECK(sf::Time::Zero.asSeconds() == 0.0f);
        STATIC_CHECK(sf::Time::Zero.asMilliseconds() == 0);
        STATIC_CHECK(sf::Time::Zero.asMicroseconds() == 0);
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            STATIC_CHECK(sf::Time() == sf::Time());
            STATIC_CHECK(sf::seconds(1) == sf::seconds(1));
            STATIC_CHECK(sf::seconds(10) == sf::milliseconds(10'000));
            STATIC_CHECK(sf::milliseconds(450'450) == sf::microseconds(450'450'000));
            STATIC_CHECK(sf::seconds(0.5f) == sf::microseconds(500'000));
        }

        SECTION("operator!=")
        {
            STATIC_CHECK(sf::seconds(10.12f) != sf::milliseconds(10'121));
            STATIC_CHECK(sf::microseconds(123'456) != sf::milliseconds(123));
        }

        SECTION("operator<")
        {
            STATIC_CHECK(sf::seconds(54.999f) < sf::seconds(55));
            STATIC_CHECK(sf::microseconds(10) < sf::milliseconds(10));
            STATIC_CHECK(sf::milliseconds(1'000) < sf::microseconds(1'000'001));
        }

        SECTION("operator>")
        {
            STATIC_CHECK(sf::seconds(55.001f) > sf::seconds(55));
            STATIC_CHECK(sf::microseconds(1) > sf::seconds(0.0000001f));
            STATIC_CHECK(sf::microseconds(1'000'001) > sf::milliseconds(1'000));
        }

        SECTION("operator<=")
        {
            STATIC_CHECK(sf::milliseconds(100) <= sf::milliseconds(100));
            STATIC_CHECK(sf::seconds(0.0012f) <= sf::microseconds(1'201));
        }

        SECTION("operator>=")
        {
            STATIC_CHECK(sf::milliseconds(100) >= sf::milliseconds(-100));
            STATIC_CHECK(sf::microseconds(1'201) >= sf::seconds(0.0012f));
        }

        SECTION("operator-")
        {
            STATIC_CHECK(sf::seconds(-1) == -sf::seconds(1));
            STATIC_CHECK(sf::microseconds(1'234) == -sf::microseconds(-1'234));
        }

        SECTION("operator+")
        {
            STATIC_CHECK(sf::seconds(1) + sf::seconds(1) == sf::seconds(2));
            STATIC_CHECK(sf::milliseconds(400) + sf::microseconds(400) == sf::microseconds(400400));
        }

        SECTION("operator+=")
        {
            sf::Time time = sf::seconds(1.5f);
            time += sf::seconds(1);
            CHECK(time == sf::seconds(2.5f));
        }

        SECTION("operator-")
        {
            STATIC_CHECK(sf::seconds(1) - sf::seconds(1) == sf::seconds(0));
            STATIC_CHECK(sf::milliseconds(400) - sf::microseconds(400) == sf::microseconds(399600));
        }

        SECTION("operator-=")
        {
            sf::Time time = sf::seconds(1.5f);
            time -= sf::seconds(10);
            CHECK(time == sf::seconds(-8.5f));
        }

        SECTION("operator*")
        {
            STATIC_CHECK(sf::seconds(1) * 2.0f == sf::seconds(2));
            STATIC_CHECK(sf::seconds(12) * 0.5f == sf::seconds(6));
            STATIC_CHECK(sf::seconds(1) * sf::base::I64{2} == sf::seconds(2));
            STATIC_CHECK(sf::seconds(42) * sf::base::I64{2} == sf::seconds(84));
            STATIC_CHECK(2.0f * sf::seconds(1) == sf::seconds(2));
            STATIC_CHECK(0.5f * sf::seconds(12) == sf::seconds(6));
            STATIC_CHECK(sf::base::I64{2} * sf::seconds(1) == sf::seconds(2));
            STATIC_CHECK(sf::base::I64{2} * sf::seconds(42) == sf::seconds(84));
        }

        SECTION("operator*=")
        {
            sf::Time time = sf::milliseconds(1'000);
            time *= sf::base::I64{10};
            CHECK(time == sf::milliseconds(10'000));
            time *= 0.1f;
            CHECK(time.asMilliseconds() == 1'000);
        }

        SECTION("operator/")
        {
            STATIC_CHECK(sf::seconds(1) / 2.0f == sf::seconds(0.5f));
            STATIC_CHECK(sf::seconds(12) / 0.5f == sf::seconds(24));
            STATIC_CHECK(sf::seconds(1) / sf::base::I64{2} == sf::seconds(0.5f));
            STATIC_CHECK(sf::seconds(42) / sf::base::I64{2} == sf::seconds(21));
            STATIC_CHECK(sf::seconds(1) / sf::seconds(1) == 1.0f);
            CHECK(sf::milliseconds(10) / sf::microseconds(1) == Approx(10'000.f));
        }

        SECTION("operator/=")
        {
            sf::Time time = sf::milliseconds(1'000);
            time /= sf::base::I64{2};
            CHECK(time == sf::milliseconds(500));
            time /= 0.5f;
            CHECK(time.asMilliseconds() == 1'000);
        }

        SECTION("operator%")
        {
            STATIC_CHECK(sf::seconds(10) % sf::seconds(3) == sf::seconds(1));
            STATIC_CHECK(sf::milliseconds(100) % sf::microseconds(10) == sf::seconds(0));
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

        STATIC_CHECK(result == sf::milliseconds(1));
    }
}
