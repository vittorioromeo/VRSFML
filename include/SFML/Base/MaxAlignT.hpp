#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
struct MaxAlignT
{
    alignas(alignof(long long)) long long a;
    alignas(alignof(long double)) long double b;
};

} // namespace sf::base
