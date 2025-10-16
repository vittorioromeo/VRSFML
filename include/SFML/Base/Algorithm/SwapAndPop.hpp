#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Erase elements satisfying a predicate from a vector (unstable)
///
/// Removes all elements from the vector for which the predicate
/// returns `true`. This function modifies the vector in-place.
/// The relative order of elements is not preserved.
///
/// \param vector    Vector to modify
/// \param predicate Unary predicate function
///
/// \return The number of elements removed
///
////////////////////////////////////////////////////////////
template <typename Vector, typename Predicate>
[[gnu::always_inline]] inline constexpr SizeT vectorSwapAndPopIf(Vector& vector, Predicate&& predicate)
{
    const SizeT initialSize = vector.size();
    SizeT       currentSize = initialSize;

    for (SizeT i = currentSize; i-- > 0u;)
    {
        if (!predicate(vector[i]))
            continue;

        --currentSize;
        vector[i] = SFML_BASE_MOVE(vector[currentSize]);
    }

    vector.resize(currentSize);
    return static_cast<SizeT>(initialSize - currentSize);
}

} // namespace sf::base
