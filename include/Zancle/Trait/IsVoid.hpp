#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isVoid = false;


////////////////////////////////////////////////////////////
template <>
inline constexpr bool isVoid<void> = true;

} // namespace za
