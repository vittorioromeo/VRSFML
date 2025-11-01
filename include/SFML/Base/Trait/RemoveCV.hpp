#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__remove_cv)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CV(...) __remove_cv(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveCVImpl                   { using type = T; };
template <typename T> struct RemoveCVImpl<const T>          { using type = T; };
template <typename T> struct RemoveCVImpl<volatile T>       { using type = T; };
template <typename T> struct RemoveCVImpl<const volatile T> { using type = T; };
// clang-format on

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CV(...) typename ::sf::base::priv::RemoveCVImpl<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveCV = SFML_BASE_REMOVE_CV(T);

} // namespace sf::base
