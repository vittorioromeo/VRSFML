#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#define SFML_BASE_TOKEN_PASTE_IMPL(x, y) x##y


////////////////////////////////////////////////////////////
#define SFML_BASE_TOKEN_PASTE(x, y) SFML_BASE_TOKEN_PASTE_IMPL(x, y)
