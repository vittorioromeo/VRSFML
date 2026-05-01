#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \brief Macro form of `sf::base::min` for headers that cannot include `MinMax.hpp`
///
/// Beware of multiple-evaluation: arguments may be evaluated more than once.
///
////////////////////////////////////////////////////////////
#define SFML_BASE_MIN(a, b) ((b) < (a) ? (b) : (a))


////////////////////////////////////////////////////////////
/// \brief Macro form of `sf::base::max` for headers that cannot include `MinMax.hpp`
///
/// Beware of multiple-evaluation: arguments may be evaluated more than once.
///
////////////////////////////////////////////////////////////
#define SFML_BASE_MAX(a, b) ((a) < (b) ? (b) : (a))
