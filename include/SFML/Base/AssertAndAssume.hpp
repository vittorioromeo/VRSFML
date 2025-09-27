#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Assume.hpp"


////////////////////////////////////////////////////////////
#define SFML_BASE_ASSERT_AND_ASSUME(...) \
    SFML_BASE_ASSERT(__VA_ARGS__);       \
    SFML_BASE_ASSUME(__VA_ARGS__)
