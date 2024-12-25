#include "SFML/System/Sleep.hpp"

// Other 1st party headers
#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

#include <Doctest.hpp>

#define CHECK_SLEEP_DURATION(duration)                           \
    do                                                           \
    {                                                            \
        sf::Clock  clock;                                        \
        const auto startTime = clock.getElapsedTime();           \
        sf::sleep(duration);                                     \
        const auto elapsed = clock.getElapsedTime() - startTime; \
        CHECK((elapsed >= duration));                            \
    } while (false)

TEST_CASE("[System] sf::sleep")
{
    CHECK_SLEEP_DURATION(sf::milliseconds(1));
    CHECK_SLEEP_DURATION(sf::milliseconds(5));
    CHECK_SLEEP_DURATION(sf::milliseconds(25));
}
