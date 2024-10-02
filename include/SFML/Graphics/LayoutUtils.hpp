#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"


// TODO P0:
namespace sf
{
////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_HELPER_GET_FN(name, ...)                                                                      \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f name(const FloatRect& bounds) \
    {                                                                                                                  \
        const auto& [pos, size] = bounds;                                                                              \
        return pos + Vector2f __VA_ARGS__;                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f name(const auto& object)      \
        requires(requires { object.getGlobalBounds(); })                                                               \
    {                                                                                                                  \
        return name(object.getGlobalBounds());                                                                         \
    }                                                                                                                  \
                                                                                                                       \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f name(const auto& object)      \
        requires(requires { object.getSize(); } && !requires { object.position; })                                     \
    {                                                                                                                  \
        return name({{0.f, 0.f}, object.getSize().toVector2f()});                                                      \
    }


SFML_PRIV_DEFINE_HELPER_GET_FN(getTopLeft, {0.f, 0.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getTopCenter, {size.x / 2.f, 0.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getTopRight, {size.x, 0.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getCenterLeft, {0.f, size.y / 2.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getCenter, {size.x / 2.f, size.y / 2.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getCenterRight, {size.x, size.y / 2.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomLeft, {0.f, size.y});
SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomCenter, {size.x / 2.f, size.y});
SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomRight, {size.x, size.y});

#undef SFML_PRIV_DEFINE_HELPER_GET_FN

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_HELPER_SET_FN(name, ...)                                                                        \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr void name(auto& object, Vector2f newPos) \
        requires(requires { object.getGlobalBounds(); })                                                                 \
    {                                                                                                                    \
        const auto& [pos, size] = object.getGlobalBounds();                                                              \
        object.position += newPos - pos - Vector2f __VA_ARGS__;                                                          \
    }

SFML_PRIV_DEFINE_HELPER_SET_FN(setTopLeft, {0.f, 0.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setTopCenter, {size.x / 2.f, 0.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setTopRight, {size.x, 0.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setCenterLeft, {0.f, size.y / 2.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setCenter, {size.x / 2.f, size.y / 2.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setCenterRight, {size.x, size.y / 2.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setBottomLeft, {0.f, size.y});
SFML_PRIV_DEFINE_HELPER_SET_FN(setBottomCenter, {size.x / 2.f, size.y});
SFML_PRIV_DEFINE_HELPER_SET_FN(setBottomRight, {size.x, size.y});

#undef SFML_PRIV_DEFINE_HELPER_SET_FN

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::LayoutUtils
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Transformable`, `sf::Window`
///
////////////////////////////////////////////////////////////
