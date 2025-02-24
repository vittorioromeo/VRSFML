#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

namespace sf::base
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

////////////////////////////////////////////////////////////
template <typename From, typename To>
using CopyCV = typename CopyCVImpl<From>::template Type<To>;

} // namespace sf::base
