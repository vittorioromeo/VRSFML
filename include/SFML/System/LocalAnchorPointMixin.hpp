#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility mixin providing anchor point functions for positioning
///
/// This template class can be inherited by classes that have position
/// and bounds (like `sf::Transformable` based classes)
/// to add convenient functions for getting and setting the object's position
/// based on common anchor points (corners, centers, edges).
///
/// It relies on the inheriting class `T` providing `getLocalBounds()`.
///
////////////////////////////////////////////////////////////
template <typename T>
struct LocalAnchorPointMixin
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto getLocalAnchorPoint(const Vec2f factors) const
    {
        return static_cast<const T&>(*this).getLocalBounds().getAnchorPoint(factors);
    }


////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_GETTER(name, ...)                                                  \
    /** \brief Get the position of the name anchor point */                                       \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto name() const \
    {                                                                                             \
        return this->getLocalAnchorPoint(__VA_ARGS__);                                            \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getLocalBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_MIXIN_GETTER


    ////////////////////////////////////////////////////////////
    /// \brief Get the world X coordinate of the left edge
    ///
    /// \return Left edge X coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLocalLeft() const
    {
        return static_cast<const T&>(*this).getLocalBounds().getLeft();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world X coordinate of the right edge
    ///
    /// \return Right edge X coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLocalRight() const
    {
        return static_cast<const T&>(*this).getLocalBounds().getRight();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world Y coordinate of the top edge
    ///
    /// \return Top edge Y coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLocalTop() const
    {
        return static_cast<const T&>(*this).getLocalBounds().getTop();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world Y coordinate of the bottom edge
    ///
    /// \return Bottom edge Y coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLocalBottom() const
    {
        return static_cast<const T&>(*this).getLocalBounds().getBottom();
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::LocalAnchorPointMixin
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
/// `struct MyObject : public sf::Transformable, public sf::LocalAnchorPointMixin<MyObject>`
/// The template argument `T` must be the type of the inheriting class itself.
/// The inheriting class must provide `getLocalBounds()`.
///
/// \see `sf::Transformable`
///
////////////////////////////////////////////////////////////
