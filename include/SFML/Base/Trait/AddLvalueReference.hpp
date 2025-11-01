#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__add_lvalue_reference)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ADD_LVALUE_REFERENCE(...) __add_lvalue_reference(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include "SFML/Base/Trait/VoidT.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T, typename = void>
struct AddLvalueReferenceImpl
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <typename T>
struct AddLvalueReferenceImpl<T, VoidT<T&>>
{
    using type = T&;
};

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ADD_LVALUE_REFERENCE(...) typename ::sf::base::priv::AddLvalueReferenceImpl<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using AddLvalueReference = SFML_BASE_ADD_LVALUE_REFERENCE(T);

} // namespace sf::base
