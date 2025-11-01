#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <bool>
struct SelectImpl
{
    template <typename TTrue, typename TFalse>
    using type = TFalse;
};


////////////////////////////////////////////////////////////
template <>
struct SelectImpl<true>
{
    template <typename TTrue, typename TFalse>
    using type = TTrue;
};

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <bool B, typename TTrue, typename TFalse>
using Conditional = typename priv::SelectImpl<B>::template type<TTrue, TFalse>;

} // namespace sf::base
