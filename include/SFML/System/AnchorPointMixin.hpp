#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility mixin providing anchor point functions for positioning
///
/// This template class can be inherited by classes that have position
/// and bounds (like `sf::Transformable` based classes or `sf::WindowBase`)
/// to add convenient functions for getting and setting the object's position
/// based on common anchor points (corners, centers, edges).
///
/// It relies on the inheriting class `T` providing `getGlobalBounds()`
/// (or `getSize()` for `sf::WindowBase`) and having a `position` member.
///
////////////////////////////////////////////////////////////
template <typename T>
struct AnchorPointMixin
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
    /** \brief Get the position of the name anchor point */                                       \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto name() const \
    {                                                                                             \
        return this->getAnchorPoint(__VA_ARGS__);                                                 \
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the top-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the center-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-left anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-center anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Get the world position of the bottom-right anchor point
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_GETTER(getBottomRight, {1.f, 1.f});

#undef SFML_PRIV_DEFINE_MIXIN_GETTER

    ////////////////////////////////////////////////////////////
    /// \brief Get the world X coordinate of the left edge
    ///
    /// \return Left edge X coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLeft() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getLeft();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the world X coordinate of the right edge
    ///
    /// \return Right edge X coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getRight() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getRight();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the world Y coordinate of the top edge
    ///
    /// \return Top edge Y coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getTop() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getTop();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the world Y coordinate of the bottom edge
    ///
    /// \return Bottom edge Y coordinate
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getBottom() const
    {
        return static_cast<const T&>(*this).getGlobalBounds().getBottom();
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
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setAnchorPoint(const Vector2f factors, const Vector2f newPosition)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position += newPosition - bounds.position + bounds.getAnchorPointOffset(factors);
    }

////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_MIXIN_SETTER(name, ...)                                                \
    /** \brief Set the position based on the name anchor point */                               \
    [[gnu::always_inline, gnu::flatten]] inline constexpr void name(const Vector2f newPosition) \
    {                                                                                           \
        this->setAnchorPoint(__VA_ARGS__, newPosition);                                         \
    }

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopLeft, {0.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopCenter, {0.5f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the top-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setTopRight, {1.f, 0.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenterLeft, {0.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenter, {0.5f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the center-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setCenterRight, {1.f, 0.5f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-left anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomLeft, {0.f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-center anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomCenter, {0.5f, 1.f});

    ////////////////////////////////////////////////////////////
    /// \brief Set the position based on the bottom-right anchor
    ///
    /// \param newPosition Target world coordinates for the anchor
    ///
    ////////////////////////////////////////////////////////////
    SFML_PRIV_DEFINE_MIXIN_SETTER(setBottomRight, {1.f, 1.f});

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
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setLeft(const float newCoordinate)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position.x += newCoordinate - bounds.position.x;
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
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setRight(const float newCoordinate)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position.x += newCoordinate - bounds.position.x - bounds.size.x;
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
    [[gnu::always_inline, gnu::flatten]] inline constexpr void setTop(const float newCoordinate)
    {
        const auto& bounds = static_cast<const T&>(*this).getGlobalBounds();
        static_cast<T&>(*this).position.y += newCoordinate - bounds.position.y;
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
/// `struct MyObject : public sf::Transformable, public sf::AnchorPointMixin<MyObject>`
/// The template argument `T` must be the type of the inheriting class itself.
/// The inheriting class must provide `getGlobalBounds()` (or `getSize()` if
/// bounds are not applicable, like for `sf::WindowBase`) and have a public
/// `position` member of type `sf::Vector2f`.
///
/// \see `sf::Transformable`, `sf::WindowBase`
///
////////////////////////////////////////////////////////////
