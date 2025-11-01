#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Macros.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Removes all except the first element from every consecutive group of equivalent elements
///
/// Removing is done by shifting the elements in the range in
/// such a way that the elements that are not to be removed appear
/// in the beginning of the range.
///
/// \param first Iterator to the beginning of the range
/// \param last  Iterator to the end of the range
///
/// \return An iterator to the new logical end of the range
///
////////////////////////////////////////////////////////////
template <typename ForwardIt>
constexpr ForwardIt unique(ForwardIt first, const ForwardIt last)
{
    if (first == last)
        return last;

    ForwardIt result = first;

    while (++first != last)
        if (!(*result == *first) && ++result != first) // NOLINT(bugprone-inc-dec-in-conditions)
            *result = SFML_BASE_MOVE(*first);

    return ++result;
}

} // namespace sf::base
