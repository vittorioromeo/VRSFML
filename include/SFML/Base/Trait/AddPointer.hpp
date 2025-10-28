#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__add_pointer)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ADD_POINTER(...) __add_pointer(__VA_ARGS__)

#else

    #include "SFML/Base/Trait/VoidT.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T, typename = void>
struct AddPointerHelper
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <typename T>
struct AddPointerHelper<T, VoidT<T*>>
{
    using type = T*;
};


////////////////////////////////////////////////////////////
template <typename T>
struct AddPointer : public AddPointerHelper<T>
{
};


////////////////////////////////////////////////////////////
template <typename T>
struct AddPointer<T&>
{
    using type = T*;
};


////////////////////////////////////////////////////////////
template <typename T>
struct AddPointer<T&&>
{
    using type = T*;
};

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ADD_POINTER(...) typename ::sf::base::priv::AddPointer<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using AddPointer = SFML_BASE_ADD_POINTER(T);

} // namespace sf::base
