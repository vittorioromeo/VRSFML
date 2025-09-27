#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename From>
struct CopyCVImpl
{
    template <typename To>
    using Type = To;
};


////////////////////////////////////////////////////////////
template <typename From>
struct CopyCVImpl<const From>
{
    template <typename To>
    using Type = const To;
};


////////////////////////////////////////////////////////////
template <typename From>
struct CopyCVImpl<volatile From>
{
    template <typename To>
    using Type = volatile To;
};


////////////////////////////////////////////////////////////
template <typename From>
struct CopyCVImpl<const volatile From>
{
    template <typename To>
    using Type = const volatile To;
};

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename From, typename To>
using CopyCV = typename priv::CopyCVImpl<From>::template Type<To>;

} // namespace sf::base
