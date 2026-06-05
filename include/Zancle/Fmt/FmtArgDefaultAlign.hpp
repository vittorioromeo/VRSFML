#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Default alignment for a type when `spec.align` is unset.
///
/// Specialize for your type to change the default:
/// `template <> inline constexpr char za::fmtArgDefaultAlign<MyType> = '>';`
///
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr char fmtArgDefaultAlign = '<';

} // namespace za
