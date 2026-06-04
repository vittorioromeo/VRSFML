#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Trait/IsSame.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
#define ZB_IS_FLOATING_POINT(...)                                                \
    (ZB_IS_SAME(__VA_ARGS__, float) || ZB_IS_SAME(__VA_ARGS__, double) || \
     ZB_IS_SAME(__VA_ARGS__, long double))


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isFloatingPoint = ZB_IS_FLOATING_POINT(T);

} // namespace zb
