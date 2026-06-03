// Header for SFML unit tests.
//
// For a new window module test case, include this header.
// Declares the `stringifyValue` ADL overload so the bespoke testing
// library can render `sf::VideoMode` for failure output.

#pragma once

#include "SFML/Base/SizeT.hpp"


#ifdef SFML_RUN_DISPLAY_TESTS
inline constexpr bool skipDisplayTests = false;
#else
inline constexpr bool skipDisplayTests = true;
#endif


namespace sf
{
struct VideoMode;

// Found by ADL when a `VideoMode` operand needs rendering for a failed assertion.
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const VideoMode& videoMode) noexcept;
} // namespace sf
