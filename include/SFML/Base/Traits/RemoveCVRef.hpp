#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__remove_cvref)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CVREF(...) __remove_cvref(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveCVRefImpl                     { using type = T; };
template <typename T> struct RemoveCVRefImpl<T&>                 { using type = T; };
template <typename T> struct RemoveCVRefImpl<T&&>                { using type = T; };
template <typename T> struct RemoveCVRefImpl<const T>            { using type = T; };
template <typename T> struct RemoveCVRefImpl<const T&>           { using type = T; };
template <typename T> struct RemoveCVRefImpl<const T&&>          { using type = T; };
template <typename T> struct RemoveCVRefImpl<volatile T>         { using type = T; };
template <typename T> struct RemoveCVRefImpl<volatile T&>        { using type = T; };
template <typename T> struct RemoveCVRefImpl<volatile T&&>       { using type = T; };
template <typename T> struct RemoveCVRefImpl<const volatile T>   { using type = T; };
template <typename T> struct RemoveCVRefImpl<const volatile T&>  { using type = T; };
template <typename T> struct RemoveCVRefImpl<const volatile T&&> { using type = T; };ì
// clang-format on

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CVREF(...) typename ::sf::base::priv::RemoveCVRefImpl<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveCVRef = SFML_BASE_REMOVE_CVREF(T);

} // namespace sf::base
