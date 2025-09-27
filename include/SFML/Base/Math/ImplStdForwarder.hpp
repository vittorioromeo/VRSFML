// Intentionally include multiple times.
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

#include <cmath>


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(name)                                \
    namespace sf::base                                                                       \
    {                                                                                        \
                                                                                             \
    template <typename T>                                                                    \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(T arg) noexcept \
    {                                                                                        \
        return ::std::name(arg);                                                             \
    }                                                                                        \
                                                                                             \
    } // namespace sf::base

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(name)                                         \
    namespace sf::base                                                                                \
    {                                                                                                 \
                                                                                                      \
    template <typename T>                                                                             \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(T arg0, T arg1) noexcept \
    {                                                                                                 \
        return ::std::name(arg0, arg1);                                                               \
    }                                                                                                 \
                                                                                                      \
    } // namespace sf::base
