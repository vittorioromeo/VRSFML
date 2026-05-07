#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Priv/Vec2Base.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility mixin providing anchor point functions for positioning
///
/// This class can be inherited by classes that have position
/// and bounds (like `sf::Transformable` based classes)
/// to add convenient functions for getting and setting the object's position
/// based on common anchor points (corners, centers, edges).
///
/// It relies on the inheriting class providing `getGlobalBounds()`
/// and having a `position` member. The derived type is deduced at call
/// time via C++23 explicit object parameters.
///
////////////////////////////////////////////////////////////
struct GlobalAnchorPointMixin
{
    ////////////////////////////////////////////////////////////
    /// \brief World coordinates of an anchor point given normalized `factors` in `[0, 1]`
    ///
    /// `(0, 0)` is the top-left, `(1, 1)` is the bottom-right.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr auto getGlobalAnchorPoint(this const auto& self, const Vec2f factors)
    {
        return self.getGlobalBounds().getAnchorPoint(factors);
    }


////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_GETTER(name, ...)                                                                 \
    /** \brief Get the position of the name anchor point */                                                      \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto name(this auto const& self) \
    {                                                                                                            \
        return self.getGlobalAnchorPoint(__VA_ARGS__);                                                           \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getGlobalBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_MIXIN_GETTER


    ////////////////////////////////////////////////////////////
    /// \brief World X coordinate of the left edge
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalLeft(this const auto& self)
    {
        return self.getGlobalBounds().getLeft();
    }


    ////////////////////////////////////////////////////////////
    /// \brief World X coordinate of the right edge
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalRight(this const auto& self)
    {
        return self.getGlobalBounds().getRight();
    }


    ////////////////////////////////////////////////////////////
    /// \brief World Y coordinate of the top edge
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalTop(this const auto& self)
    {
        return self.getGlobalBounds().getTop();
    }


    ////////////////////////////////////////////////////////////
    /// \brief World Y coordinate of the bottom edge
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalBottom(this const auto& self)
    {
        return self.getGlobalBounds().getBottom();
    }


    ////////////////////////////////////////////////////////////
    /// \brief World width of the object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalWidth(this const auto& self)
    {
        return self.getGlobalBounds().size.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief World height of the object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalHeight(this const auto& self)
    {
        return self.getGlobalBounds().size.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move the object so the anchor point at `factors` lands at `newPosition`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    inline constexpr void setGlobalAnchorPoint(this Self& self, const Vec2f factors, const Vec2f newPosition)
    {
        const auto& bounds = self.getGlobalBounds();
        addPositionImpl(self, newPosition - bounds.position + bounds.getAnchorPointOffset(factors));
    }


////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_SETTER(name, ...)                                                              \
    /** \brief Set the position based on the name anchor point */                                             \
    template <typename Self>                                                                                  \
    [[gnu::always_inline, gnu::flatten]] inline constexpr void name(this Self& self, const Vec2f newPosition) \
    {                                                                                                         \
        self.setGlobalAnchorPoint(__VA_ARGS__, newPosition);                                                  \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the top-left anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the top-center anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the top-right anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the center-left anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the center anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the center-right anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the bottom-left anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the bottom-center anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the world position of the bottom-right anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_MIXIN_SETTER


    ////////////////////////////////////////////////////////////
    /// \brief Move the object horizontally so its left edge lands at `newCoordinate`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    inline constexpr void setGlobalLeft(this Self& self, const float newCoordinate)
    {
        const auto& bounds = self.getGlobalBounds();
        addPositionImpl(self, Vec2f{newCoordinate - bounds.position.x, 0.f});
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move the object horizontally so its right edge lands at `newCoordinate`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    inline constexpr void setGlobalRight(this Self& self, const float newCoordinate)
    {
        const auto& bounds = self.getGlobalBounds();
        addPositionImpl(self, Vec2f{newCoordinate - bounds.position.x - bounds.size.x, 0.f});
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move the object vertically so its top edge lands at `newCoordinate`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    inline constexpr void setGlobalTop(this Self& self, const float newCoordinate)
    {
        const auto& bounds = self.getGlobalBounds();
        addPositionImpl(self, Vec2f{0.f, newCoordinate - bounds.position.y});
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move the object vertically so its bottom edge lands at `newCoordinate`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    inline constexpr void setGlobalBottom(this Self& self, const float newCoordinate)
    {
        const auto& bounds = self.getGlobalBounds();
        addPositionImpl(self, Vec2f{0.f, newCoordinate - bounds.position.y - bounds.size.y});
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move the object horizontally so its center lands at `newCoordinate`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    inline constexpr void setGlobalCenterX(this Self& self, const float newCoordinate)
    {
        const auto& bounds = self.getGlobalBounds();
        addPositionImpl(self, Vec2f{newCoordinate - bounds.position.x - bounds.size.x / 2.f, 0.f});
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move the object vertically so its center lands at `newCoordinate`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    inline constexpr void setGlobalCenterY(this Self& self, const float newCoordinate)
    {
        const auto& bounds = self.getGlobalBounds();
        addPositionImpl(self, Vec2f{0.f, newCoordinate - bounds.position.y - bounds.size.y / 2.f});
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Helper that implicitly converts a `Vec2f` to any `Vec2<U>`
    ///
    /// Used by `addPositionImpl` to feed a computed `Vec2f` offset back
    /// into the inheriting class's `setPosition()` regardless of the
    /// concrete coordinate type the derived type uses for its position.
    ///
    ////////////////////////////////////////////////////////////
    struct AutoConvertingVec2f
    {
        Vec2f data;

        template <typename U>
        [[nodiscard, gnu::always_inline, gnu::flatten]] operator Vec2<U>() const
        {
            return data.to<Vec2<U>>();
        }
    };


    ////////////////////////////////////////////////////////////
    /// \brief Add a world-space `offset` to the inheriting object's position
    ///
    /// Picks between two strategies depending on the derived API: a public
    /// `position` member is mutated in place; otherwise `setPosition` is
    /// called with the current position plus `offset` (auto-converted to
    /// the derived type's position type via `AutoConvertingVec2f`).
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self>
    [[gnu::always_inline, gnu::flatten]]
    static inline constexpr void addPositionImpl(Self& self, const Vec2f offset)
    {
        if constexpr (requires { self.position; })
        {
            self.position += offset;
        }
        else if constexpr (requires { self.getPosition(); })
        {
            self.setPosition(AutoConvertingVec2f{self.getPosition().toVec2f() + offset});
        }
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::GlobalAnchorPointMixin
/// \ingroup system
///
/// Utility mixin providing convenient functions to get and set
/// the position of an object based on anchor points relative
/// to its bounding box (or size, for windows). These anchor points
/// include corners (e.g., `getTopLeft()`, `setBottomRight()`),
/// edge centers (e.g., `getCenterLeft()`, `setTopCenter()`), and the
/// overall center (`getCenter()`, `setCenter()`).
///
/// It also provides functions to get or set the position based
/// on individual edge coordinates (`getLeft()`, `setRight()`, etc.).
///
/// To use this mixin, inherit from it publicly, e.g.:
/// `struct MyObject : public sf::Transformable, public sf::GlobalAnchorPointMixin`
/// The inheriting class must provide `getGlobalBounds()` and have a public
/// `position` member of type `sf::Vec2f` (or expose `getPosition`/`setPosition`).
///
/// \see `sf::Transformable`
///
////////////////////////////////////////////////////////////
