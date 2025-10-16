#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Algorithm/Remove.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Erase elements satisfying a predicate from a vector (stable)
///
/// Removes all elements from the vector for which the predicate
/// returns `true`. This function modifies the vector in-place.
/// Equivalent to `std::erase_if` for vectors.
///
/// \param vector    Vector to modify
/// \param predicate Unary predicate function
///
/// \return The number of elements removed
///
////////////////////////////////////////////////////////////
template <typename Vector, typename Predicate>
[[gnu::always_inline]] inline constexpr SizeT vectorEraseIf(Vector& vector, Predicate&& predicate)
{
    const auto it       = removeIf(vector.begin(), vector.end(), predicate);
    const auto nRemoved = static_cast<SizeT>(vector.end() - it);

    vector.erase(it, vector.end());
    return nRemoved;
}

} // namespace sf::base
