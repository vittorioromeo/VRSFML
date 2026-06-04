#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/DeclVal.hpp"
#include "ZancleBase/Swap.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isNoThrowSwappable = noexcept(swap(declVal<T&>(), declVal<T&>()));

} // namespace zb


////////////////////////////////////////////////////////////
#define ZB_IS_NOTHROW_SWAPPABLE(...) ::zb::isNoThrowSwappable<__VA_ARGS__>
