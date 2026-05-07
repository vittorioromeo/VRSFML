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
/// \brief Move elements not satisfying `predicate` to the front of `[first, last)`
///
/// Elements past the returned iterator are in a valid but unspecified state.
///
/// \return Iterator to the new logical end of the range
///
////////////////////////////////////////////////////////////
template <typename ForwardIt, typename Predicate>
[[nodiscard, gnu::always_inline]] inline constexpr ForwardIt removeIf(ForwardIt first, const ForwardIt last, Predicate&& predicate)
{
    first = findIf(first, last, predicate);

    if (first != last)
        for (ForwardIt i = first; ++i != last;)
            if (!predicate(*i))
                *first++ = SFML_BASE_MOVE(*i);

    return first;
}

} // namespace sf::base
