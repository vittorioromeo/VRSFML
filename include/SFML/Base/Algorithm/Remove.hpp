#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/Macros.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Remove elements satisfying a predicate from a range
///
/// Moves elements for which the predicate is false to the beginning
/// of the range `[first, last)`. Returns an iterator to the new logical
/// end of the range. The elements after the returned iterator are in
/// a valid but unspecified state.
/// Equivalent to `std::remove_if`.
///
/// \param first     Iterator to the beginning of the range
/// \param last      Iterator to the end of the range
/// \param predicate Unary predicate function
///
/// \return Iterator to the new logical end of the range
///
////////////////////////////////////////////////////////////
template <typename Iter, typename Predicate>
[[nodiscard, gnu::always_inline]] inline constexpr Iter removeIf(Iter first, Iter last, Predicate&& predicate)
{
    first = findIf(first, last, predicate);

    if (first != last)
        for (Iter i = first; ++i != last;)
            if (!predicate(*i))
                *first++ = SFML_BASE_MOVE(*i);

    return first;
}

} // namespace sf::base
