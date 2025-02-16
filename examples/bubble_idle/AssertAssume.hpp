#pragma once

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Assume.hpp"


////////////////////////////////////////////////////////////
// TODO P2: (lib) put in lib?
#define ASSERT_AND_ASSUME(...)     \
    SFML_BASE_ASSERT(__VA_ARGS__); \
    SFML_BASE_ASSUME(__VA_ARGS__)
