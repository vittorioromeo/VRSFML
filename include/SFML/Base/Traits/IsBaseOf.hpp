#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_base_of)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_BASE_OF(b, d) __is_base_of(b, d)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_BASE_OF(b, d) ::std::is_base_of_v<b, d>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename B, typename D>
inline constexpr bool isBaseOf = SFML_BASE_IS_BASE_OF(B, D);

} // namespace sf::base
