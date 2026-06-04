#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Macro form of `zb::clamp` for use in headers that cannot include `Clamp.hpp`
///
/// Beware of multiple-evaluation: each argument is evaluated up to twice.
///
////////////////////////////////////////////////////////////
#define ZB_CLAMP(value, minValue, maxValue) \
    ((value) < (minValue) ? (minValue) : ((value) > (maxValue) ? (maxValue) : (value)))
