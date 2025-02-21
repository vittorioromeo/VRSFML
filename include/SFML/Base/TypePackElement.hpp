#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"


#if __has_builtin(__type_pack_element)

////////////////////////////////////////////////////////////
#define SFML_BASE_TYPE_PACK_ELEMENT(N, ...) __type_pack_element<N, __VA_ARGS__>


namespace sf::base
{
////////////////////////////////////////////////////////////
template <SizeT N, typename... Ts>
using TypePackElement = SFML_BASE_TYPE_PACK_ELEMENT(N, Ts...);

} // namespace sf::base

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T>
struct TypeWrapper
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <SizeT N,
          typename T0 = void,
          typename T1 = void,
          typename T2 = void,
          typename T3 = void,
          typename T4 = void,
          typename T5 = void,
          typename T6 = void,
          typename T7 = void,
          typename T8 = void,
          typename T9 = void,
          typename... Ts>
[[nodiscard, gnu::always_inline]] consteval auto typePackElementImpl() noexcept
{
    // clang-format off
    if constexpr(N == 0)      { return TypeWrapper<T0>{}; }
    else if constexpr(N == 1) { return TypeWrapper<T1>{}; }
    else if constexpr(N == 2) { return TypeWrapper<T2>{}; }
    else if constexpr(N == 3) { return TypeWrapper<T3>{}; }
    else if constexpr(N == 4) { return TypeWrapper<T4>{}; }
    else if constexpr(N == 5) { return TypeWrapper<T5>{}; }
    else if constexpr(N == 6) { return TypeWrapper<T6>{}; }
    else if constexpr(N == 7) { return TypeWrapper<T7>{}; }
    else if constexpr(N == 8) { return TypeWrapper<T8>{}; }
    else if constexpr(N == 9) { return TypeWrapper<T9>{}; }
    else                      { return typePackElementImpl<N - 10, Ts...>(); }
    // clang-format on
}

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <SizeT N, typename... Ts>
using TypePackElement = typename decltype(priv::typePackElementImpl<N, Ts...>())::type;


////////////////////////////////////////////////////////////
#define SFML_BASE_TYPE_PACK_ELEMENT(N, ...) ::sf::base::TypePackElement<N, __VA_ARGS__>

} // namespace sf::base

#endif
