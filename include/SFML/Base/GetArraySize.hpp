#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Get the size of a C-style array at compile time
///
/// \return Size of the array (number of elements)
///
////////////////////////////////////////////////////////////
template <typename T, auto N>
[[nodiscard]] consteval SizeT getArraySize(const T (&)[N]) noexcept
{
    return N;
}


////////////////////////////////////////////////////////////
/// \brief Get the size of a C-style array member at compile time
///
/// \return Size of the array member (number of elements)
///
////////////////////////////////////////////////////////////
template <typename S, typename T, auto N>
[[nodiscard]] consteval SizeT getArraySize(const T (S::*)[N]) noexcept
{
    return N;
}

} // namespace sf::base
