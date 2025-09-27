#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <bool, typename TTrue, typename TFalse>
struct ConditionalImpl
{
    using type = TFalse;
};


////////////////////////////////////////////////////////////
template <typename TTrue, typename TFalse>
struct ConditionalImpl<true, TTrue, TFalse>
{
    using type = TTrue;
};


////////////////////////////////////////////////////////////
template <bool B, typename TTrue, typename TFalse>
using Conditional = typename ConditionalImpl<B, TTrue, TFalse>::type;

} // namespace sf::base
