#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Default alignment for a type when `spec.align` is unset.
///
/// Specialize for your type to change the default:
/// `template <> inline constexpr char sf::base::fmtArgDefaultAlign<MyType> = '>';`
///
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr char fmtArgDefaultAlign = '<';

} // namespace sf::base
