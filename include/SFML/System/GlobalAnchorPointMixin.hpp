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
    /// \brief Get the world position of a specific anchor point
    ///
    /// Calculates the world coordinates of a point based on normalized
    /// factors within the object's bounding box (or size for windows).
    /// `(0, 0)` corresponds to the top-left, `(1, 1)` to the bottom-right.
    ///
    /// \param factors Normalized factors `(x, y)` within the bounds `[0, 1]`
    ///
    /// \return World coordinates of the calculated anchor point
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
    /// \brief Get the world X coordinate of the left edge
    ///
    /// \return Left edge X coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalLeft(this const auto& self)
    {
        return self.getGlobalBounds().getLeft();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world X coordinate of the right edge
    ///
    /// \return Right edge X coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalRight(this const auto& self)
    {
        return self.getGlobalBounds().getRight();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world Y coordinate of the top edge
    ///
    /// \return Top edge Y coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalTop(this const auto& self)
    {
        return self.getGlobalBounds().getTop();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world Y coordinate of the bottom edge
    ///
    /// \return Bottom edge Y coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalBottom(this const auto& self)
    {
        return self.getGlobalBounds().getBottom();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world width of the object
    ///
    /// \return Width of the object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalWidth(this const auto& self)
    {
        return self.getGlobalBounds().size.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world height of the object
    ///
    /// \return Height of the object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]]
    inline constexpr float getGlobalHeight(this const auto& self)
    {
        return self.getGlobalBounds().size.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Set the object's position based on an anchor point
    ///
    /// Moves the object so that the anchor point specified by `factors`
    /// aligns with the given `newPosition` in world coordinates.
    ///
    /// \param factors     Normalized factors (x, y) defining the anchor point
    /// \param newPosition Target world coordinates for the anchor point
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
    /// \brief Set the position based on the top-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setGlobalBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_MIXIN_SETTER


    ////////////////////////////////////////////////////////////
    /// \brief Set the world X coordinate of the left edge
    ///
    /// This moves the entire object horizontally so its left edge
    /// aligns with `newCoordinate`.
    ///
    /// \param newCoordinate Target X coordinate for the left edge
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
    /// \brief Set the world X coordinate of the right edge
    ///
    /// This moves the entire object horizontally so its right edge
    /// aligns with `newCoordinate`.
    ///
    /// \param newCoordinate Target X coordinate for the right edge
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
    /// \brief Set the world Y coordinate of the top edge
    ///
    /// This moves the entire object vertically so its top edge
    /// aligns with `newCoordinate`.
    ///
    /// \param newCoordinate Target Y coordinate for the top edge
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
    /// \brief Set the world Y coordinate of the bottom edge
    ///
    /// This moves the entire object vertically so its bottom edge
    /// aligns with `newCoordinate`.
    ///
    /// \param newCoordinate Target Y coordinate for the bottom edge
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
    /// \brief Set the world X coordinate of the center
    ///
    /// This moves the entire object horizontally so its center
    /// aligns with `newCoordinate`.
    ///
    /// \param newCoordinate Target X coordinate for the center
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
    /// \brief Set the world Y coordinate of the center
    ///
    /// This moves the entire object vertically so its center
    /// aligns with `newCoordinate`.
    ///
    /// \param newCoordinate Target Y coordinate for the center
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
    /// \brief Add an offset to the inheriting object's position
    ///
    /// Selects between two strategies based on the API exposed by the
    /// derived type: if it has a public `position` member, the offset
    /// is added in place; otherwise, `setPosition` is called with the
    /// current position plus the offset (with automatic conversion to
    /// the derived type's position type via `AutoConvertingVec2f`).
    ///
    /// \param self   Inheriting object to mutate
    /// \param offset Offset to add to the current position, in world coordinates
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
