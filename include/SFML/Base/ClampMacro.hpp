#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#define SFML_BASE_CLAMP(value, minValue, maxValue) \
    ((value) < (minValue) ? (minValue) : ((value) > (maxValue) ? (maxValue) : (value)))
