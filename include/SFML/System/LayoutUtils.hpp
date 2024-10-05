#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"


// TODO P0:
namespace sf::LayoutUtils
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f getAnchorPoint(const FloatRect& bounds,
                                                                                                    Vector2f factors)
{
    return bounds.position + bounds.size.componentWiseMul(factors);
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f getAnchorPoint(const auto& object,
                                                                                                    Vector2f    factors)
    requires(requires { object.getGlobalBounds(); })
{
    return getAnchorPoint(object.getGlobalBounds(), factors);
}

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_HELPER_GET_FN(name, ...)                                                                      \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f name(const FloatRect& bounds) \
    {                                                                                                                  \
        return bounds.position + bounds.size.componentWiseMul(__VA_ARGS__);                                            \
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

// clang-format off
SFML_PRIV_DEFINE_HELPER_GET_FN(getTopLeft,      {0.f,  0.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getTopCenter,    {0.5f, 0.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getTopRight,     {1.f,  0.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getCenterLeft,   {0.f,  0.5f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getCenter,       {0.5f, 0.5f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getCenterRight,  {1.f,  0.5f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomLeft,   {0.f,  1.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomCenter, {0.5f, 1.f});
SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomRight,  {1.f,  1.f});
// clang-format on

#undef SFML_PRIV_DEFINE_HELPER_GET_FN


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]]
inline constexpr void setAnchorPoint(auto& object, Vector2f factors, Vector2f newPos)
    requires(requires { object.getGlobalBounds(); })
{
    const auto& bounds = object.getGlobalBounds();
    object.position += newPos - bounds.position - Vector2f{bounds.size.x * factors.x, bounds.size.y * factors.y};
}

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_HELPER_SET_FN(name, ...)                                                  \
    [[gnu::always_inline, gnu::flatten]] inline constexpr void name(auto& object, Vector2f newPos) \
        requires(requires { object.getGlobalBounds(); })                                           \
    {                                                                                              \
        setAnchorPoint(object, __VA_ARGS__, newPos);                                               \
    }

// clang-format off
SFML_PRIV_DEFINE_HELPER_SET_FN(setTopLeft,      {0.f,  0.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setTopCenter,    {0.5f, 0.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setTopRight,     {1.f,  0.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setCenterLeft,   {0.f,  0.5f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setCenter,       {0.5f, 0.5f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setCenterRight,  {1.f,  0.5f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setBottomLeft,   {0.f,  1.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setBottomCenter, {0.5f, 1.f});
SFML_PRIV_DEFINE_HELPER_SET_FN(setBottomRight,  {1.f,  1.f});
// clang-format on

#undef SFML_PRIV_DEFINE_HELPER_SET_FN


////////////////////////////////////////////////////////////
template <typename T>
struct Mixin
{
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f getAnchorPoint(Vector2f factors) const
    {
        return LayoutUtils::getAnchorPoint(static_cast<const T&>(*this), factors);
    }

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_GETTER(name)                                                           \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f name() const \
    {                                                                                                 \
        return LayoutUtils::name(static_cast<const T&>(*this));                                       \
    }

    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopLeft);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopCenter);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopRight);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenterLeft);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenter);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenterRight);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomLeft);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomCenter);
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomRight);

#undef SFML_PRIV_DEFINE_MIXIN_GETTER

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setAnchorPoint(Vector2f factors, Vector2f newPos)
    {
        LayoutUtils::setAnchorPoint(static_cast<T&>(*this), factors, newPos);
    }

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_SETTER(name)                                          \
    [[gnu::always_inline, gnu::flatten]] inline constexpr void name(Vector2f newPos) \
    {                                                                                \
        LayoutUtils::name(static_cast<T&>(*this), newPos);                           \
    }

    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopLeft);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopCenter);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopRight);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenterLeft);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenter);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenterRight);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomLeft);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomCenter);
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomRight);

#undef SFML_PRIV_DEFINE_MIXIN_SETTER
};

} // namespace sf::LayoutUtils


////////////////////////////////////////////////////////////
/// \class sf::LayoutUtils
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Transformable`, `sf::Window`
///
////////////////////////////////////////////////////////////
