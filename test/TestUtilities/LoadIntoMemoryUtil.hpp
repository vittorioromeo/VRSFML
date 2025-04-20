// Header for SFML unit tests.
//
// For a new system module test case, include this header.
// This ensures that string conversions are visible and can be used by Catch2 for debug output.

#pragma once

#include "SFML/Base/Vector.hpp"

[[nodiscard]] sf::base::Vector<unsigned char> loadIntoMemory(const char* path);
