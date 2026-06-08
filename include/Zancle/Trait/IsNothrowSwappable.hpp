#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Base/DeclVal.hpp"
#include "Zancle/Base/Swap.hpp" // IWYU pragma: keep


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isNoThrowSwappable = noexcept(swap(declVal<T&>(), declVal<T&>()));

} // namespace za


////////////////////////////////////////////////////////////
#define ZA_IS_NOTHROW_SWAPPABLE(...) ::za::isNoThrowSwappable<__VA_ARGS__>
