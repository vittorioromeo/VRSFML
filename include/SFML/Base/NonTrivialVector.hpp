#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/TrivialVector.hpp"
#include "SFML/Base/InitializerList.hpp" // used
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"


namespace sf::base
{
// TODO P0:

////////////////////////////////////////////////////////////
template <typename TItem>
using NonTrivialVector = TrivialVector<TItem>;

} // namespace sf::base
