#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
#define SFML_BASE_CLAMP(value, minValue, maxValue) \
    ((value) < (minValue) ? (minValue) : ((value) > (maxValue) ? (maxValue) : (value)))
