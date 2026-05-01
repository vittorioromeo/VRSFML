// Header for SFML unit tests.
//
// For a new window module test case, include this header.
// This specializes `doctest::StringMaker` so doctest can stringify
// `sf::VideoMode` for failure output without dragging `<ostream>`.

#pragma once

#include <DoctestFwd.hpp>


#ifdef SFML_RUN_DISPLAY_TESTS
inline constexpr bool skipDisplayTests = false;
#else
inline constexpr bool skipDisplayTests = true;
#endif


namespace sf
{
struct VideoMode;
} // namespace sf


namespace doctest
{
template <>
struct StringMaker<sf::VideoMode>
{
    static String convert(const sf::VideoMode& videoMode);
};
} // namespace doctest
