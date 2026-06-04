// Header for Zancle unit tests.
//
// For a new system module test case, include this header.
// This ensures that string conversions are visible and can be used by Catch2 for debug output.

#pragma once

#include "ZancleBase/Vector.hpp"

[[nodiscard]] zb::Vector<char> loadIntoMemory(const char* path);
