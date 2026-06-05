#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Trait/IsSame.hpp"


namespace za
{
////////////////////////////////////////////////////////////
#define ZA_IS_FLOATING_POINT(...) \
    (ZA_IS_SAME(__VA_ARGS__, float) || ZA_IS_SAME(__VA_ARGS__, double) || ZA_IS_SAME(__VA_ARGS__, long double))


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isFloatingPoint = ZA_IS_FLOATING_POINT(T);

} // namespace za
