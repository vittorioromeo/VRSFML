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
/// \brief Unstable in-place erase of elements satisfying `predicate`; returns the number removed
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
