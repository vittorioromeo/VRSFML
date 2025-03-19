#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__add_lvalue_reference)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ADD_LVALUE_REFERENCE(...) __add_lvalue_reference(__VA_ARGS__)

#else

    #include "SFML/Base/Traits/VoidT.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T, typename = void>
struct AddLvalueReferenceHelper
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <typename T>
struct AddLvalueReferenceHelper<T, VoidT<T&>>
{
    using type = T&;
};

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_ADD_LVALUE_REFERENCE(...) typename ::sf::base::priv::AddLvalueReferenceHelper<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using AddLvalueReference = SFML_BASE_ADD_LVALUE_REFERENCE(T);

} // namespace sf::base
