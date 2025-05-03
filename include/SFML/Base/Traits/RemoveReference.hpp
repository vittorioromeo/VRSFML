#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__remove_reference)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_REFERENCE(...) __remove_reference(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveReferenceImpl      { using type = T; };
template <typename T> struct RemoveReferenceImpl<T&>  { using type = T; };
template <typename T> struct RemoveReferenceImpl<T&&> { using type = T; };
// clang-format on

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_REFERENCE(...) typename ::sf::base::priv::RemoveReferenceImpl<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveReference = SFML_BASE_REMOVE_REFERENCE(T);

} // namespace sf::base
