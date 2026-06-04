#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace zb
{
////////////////////////////////////////////////////////////
/// \brief Default alignment for a type when `spec.align` is unset.
///
/// Specialize for your type to change the default:
/// `template <> inline constexpr char zb::fmtArgDefaultAlign<MyType> = '>';`
///
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr char fmtArgDefaultAlign = '<';

} // namespace zb
