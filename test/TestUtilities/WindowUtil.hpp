// Header for SFML unit tests.
//
// For a new window module test case, include this header.
// Specializes `tst::StringMaker` so the bespoke testing library can
// stringify `sf::VideoMode` for failure output.

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


namespace tst
{
template <>
struct StringMaker<sf::VideoMode>
{
    static sf::base::String convert(const sf::VideoMode& videoMode);
};
} // namespace tst
