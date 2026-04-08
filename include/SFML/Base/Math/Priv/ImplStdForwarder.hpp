// Intentionally include multiple times.
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Fallback for math wrappers when no compiler builtin is available
///
/// Same interface as `ImplBuiltinWrapper.hpp` (defines the
/// `SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_*` macros), but the
/// generated functions forward to the corresponding `std::name` from
/// `<cmath>`. Used when one or more `__builtin_<name>*` overloads are
/// missing on the current compiler.
///
////////////////////////////////////////////////////////////


#include <cmath> // IWYU pragma: keep


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_1ARG(name)                                      \
    namespace sf::base                                                                             \
    {                                                                                              \
                                                                                                   \
    template <typename T>                                                                          \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(const T arg) noexcept \
    {                                                                                              \
        return ::std::name(arg);                                                                   \
    }                                                                                              \
                                                                                                   \
    } // namespace sf::base

////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_BUILTIN_MATH_WRAPPER_2ARG(name)                                                     \
    namespace sf::base                                                                                            \
    {                                                                                                             \
                                                                                                                  \
    template <typename T>                                                                                         \
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr auto name(const T arg0, const T arg1) noexcept \
    {                                                                                                             \
        return ::std::name(arg0, arg1);                                                                           \
    }                                                                                                             \
                                                                                                                  \
    } // namespace sf::base
