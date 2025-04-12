#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
struct AnchorPointMixin
{
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto getAnchorPoint(const Vector2f factors) const
    {
        if constexpr (requires { static_cast<const T&>(*this).getGlobalBounds(); })
        {
            return static_cast<const T&>(*this).getGlobalBounds().getAnchorPoint(factors);
        }
        else
        {
            // For `sf::WindowBase`
            return static_cast<const T&>(*this).getSize().toVector2f().componentWiseMul(factors);
        }
    }

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_GETTER(name, ...)                                                  \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto name() const \
    {                                                                                             \
        return this->getAnchorPoint(__VA_ARGS__);                                                 \
    }

    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopLeft, {0.f, 0.f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopCenter, {0.5f, 0.f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopRight, {1.f, 0.f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenterLeft, {0.f, 0.5f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenter, {0.5f, 0.5f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenterRight, {1.f, 0.5f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomLeft, {0.f, 1.f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomCenter, {0.5f, 1.f});
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_MIXIN_GETTER

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLeft() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getLeft();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getRight() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getRight();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getTop() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getTop();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getBottom() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getBottom();
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setAnchorPoint(const Vector2f factors, const Vector2f newPosition)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position += newPosition - bounds.position + bounds.getAnchorPointOffset(factors);
    }

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_SETTER(name, ...)                                                \
    [[gnu::always_inline, gnu::flatten]] inline constexpr void name(const Vector2f newPosition) \
    {                                                                                           \
        this->setAnchorPoint(__VA_ARGS__, newPosition);                                         \
    }

    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopLeft, {0.f, 0.f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopCenter, {0.5f, 0.f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopRight, {1.f, 0.f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenterLeft, {0.f, 0.5f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenter, {0.5f, 0.5f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenterRight, {1.f, 0.5f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomLeft, {0.f, 1.f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomCenter, {0.5f, 1.f});
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_MIXIN_SETTER

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setLeft(const float newCoordinate)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position.x += newCoordinate - bounds.position.x;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setRight(const float newCoordinate)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position.x += newCoordinate - bounds.position.x - bounds.size.x;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setTop(const float newCoordinate)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position.y += newCoordinate - bounds.position.y;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setBottom(const float newCoordinate)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position.y += newCoordinate - bounds.position.y - bounds.size.y;
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::AnchorPointMixin
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Transformable`, `sf::WindowBase`
///
////////////////////////////////////////////////////////////
