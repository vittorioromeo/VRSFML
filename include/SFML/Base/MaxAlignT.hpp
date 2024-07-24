#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

namespace sf::base
{
////////////////////////////////////////////////////////////
struct MaxAlignT
{
    alignas(alignof(long long)) long long a;
    alignas(alignof(long double)) long double b;
};

} // namespace sf::base
