#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Trait/RemoveReference.hpp"


////////////////////////////////////////////////////////////
/// \brief `std::move` equivalent without including `<utility>`
///
/// Casts its argument to an rvalue reference. Implemented as a
/// `static_cast` so it has zero compile-time and zero debug-mode
/// overhead compared to `std::move`.
///
////////////////////////////////////////////////////////////
#define SFML_BASE_MOVE(...) static_cast<SFML_BASE_REMOVE_REFERENCE(decltype(__VA_ARGS__)) &&>(__VA_ARGS__)


////////////////////////////////////////////////////////////
/// \brief `std::forward` equivalent without including `<utility>`
///
/// Forwards its argument while preserving its value category. Equivalent
/// to `std::forward<decltype(x)>(x)` but cheaper to compile and to debug.
///
////////////////////////////////////////////////////////////
#define SFML_BASE_FORWARD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
