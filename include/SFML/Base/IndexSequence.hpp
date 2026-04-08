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
/// \brief Compile-time integer pack, equivalent to `std::index_sequence`
///
/// Empty type whose template parameters carry the indices. Typically
/// produced by `MakeIndexSequence<N>` and matched by an
/// `IndexSequence<Is...>` lambda parameter to expand parameter packs.
///
////////////////////////////////////////////////////////////
template <SizeT...>
struct IndexSequence
{
};

} // namespace sf::base
