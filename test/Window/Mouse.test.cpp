#include <Doctest.hpp>

#include <WindowUtil.hpp>

#include <SFML/Window/Mouse.hpp>

TEST_CASE("[Window] sf::Mouse" * doctest::skip(skipDisplayTests))
{
    SECTION("isButtonPressed()")
    {
        CHECK(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left));
        CHECK(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Right));
        CHECK(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle));
        CHECK(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Extra1));
        CHECK(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Extra2));
    }
}
