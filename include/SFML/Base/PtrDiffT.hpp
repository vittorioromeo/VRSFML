#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

namespace sf::base
{
////////////////////////////////////////////////////////////
using PtrDiffT = decltype(static_cast<int*>(nullptr) - static_cast<int*>(nullptr));

} // namespace sf::base
