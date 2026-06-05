// Header for Zancle unit tests.
//
// For a new system module test case, include this header.
// This ensures that string conversions are visible and can be used by Catch2 for debug output.

#pragma once

#include "Zancle/Container/Vector.hpp"

[[nodiscard]] za::Vector<char> loadIntoMemory(const char* path);
