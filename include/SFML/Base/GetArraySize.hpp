#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Get the size of a C-style array at compile time
///
/// \param Array reference
///
/// \return Size of the array (number of elements)
///
////////////////////////////////////////////////////////////
template <typename T, auto N>
[[nodiscard]] consteval auto getArraySize(const T (&)[N]) noexcept
{
    return N;
}


////////////////////////////////////////////////////////////
/// \brief Get the size of a C-style array member at compile time
///
/// \param Pointer to array member
///
/// \return Size of the array member (number of elements)
///
////////////////////////////////////////////////////////////
template <typename S, typename T, auto N>
[[nodiscard]] consteval auto getArraySize(const T (S::*)[N]) noexcept
{
    return N;
}

} // namespace sf::base
