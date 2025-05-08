#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__remove_const)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CONST(...) __remove_const(__VA_ARGS__)

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveConstImpl          { using type = T; };
template <typename T> struct RemoveConstImpl<const T> { using type = T; };
// clang-format on

} // namespace sf::base::priv

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_REMOVE_CONST(...) typename ::sf::base::priv::RemoveConstImpl<__VA_ARGS__>::type

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveConst = SFML_BASE_REMOVE_CONST(T);

} // namespace sf::base
