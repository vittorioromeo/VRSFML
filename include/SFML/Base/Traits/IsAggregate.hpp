#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__is_aggregate)

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_AGGREGATE(...) __is_aggregate(__VA_ARGS__)

#else

#include <type_traits>

////////////////////////////////////////////////////////////
#define SFML_BASE_IS_AGGREGATE(...) ::std::is_aggregate_v<__VA_ARGS__>

#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isAggregate = SFML_BASE_IS_AGGREGATE(T);

} // namespace sf::base
