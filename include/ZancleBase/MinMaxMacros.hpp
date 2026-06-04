#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Macro form of `zb::min` for headers that cannot include `MinMax.hpp`
///
/// Beware of multiple-evaluation: arguments may be evaluated more than once.
///
////////////////////////////////////////////////////////////
#define ZB_MIN(a, b) ((b) < (a) ? (b) : (a))


////////////////////////////////////////////////////////////
/// \brief Macro form of `zb::max` for headers that cannot include `MinMax.hpp`
///
/// Beware of multiple-evaluation: arguments may be evaluated more than once.
///
////////////////////////////////////////////////////////////
#define ZB_MAX(a, b) ((a) < (b) ? (b) : (a))
