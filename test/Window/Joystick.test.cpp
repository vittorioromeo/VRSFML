#include "SFML/Window/Joystick.hpp"

#include <Doctest.hpp>

#include <SystemUtil.hpp>

TEST_CASE("[Window] sf::Joystick")
{
    SECTION("Constants")
    {
        STATIC_CHECK(sf::Joystick::MaxCount == 8);
        STATIC_CHECK(sf::Joystick::MaxButtonCount == 32);
        STATIC_CHECK(sf::Joystick::MaxAxisCount == 8);
    }

    // By avoiding calling sf::Joystick::update() we can guarantee that
    // no joysticks will be detected. This is how we can ensure these
    // tests are portable and reliable.

    for (unsigned int joystickId = 0u; joystickId < sf::Joystick::MaxCount; ++joystickId)
    {
        SECTION("query()")
        {
            CHECK(!sf::Joystick::query(joystickId).hasValue());
        }
    }
}
