#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/GlobalAnchorPointMixin.hpp"
#include "SFML/System/LocalAnchorPointMixin.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Defines the geometric properties of an ellipse shape
///
/// `sf::EllipseShapeData` is used to specify the characteristics
/// of an ellipse, such as its horizontal and vertical radii,
/// and the number of points used to approximate its contour.
/// This data is then typically used by a rendering system or a
/// shape class to draw the actual ellipse.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API EllipseShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        horizontalRadius{0.f}; //!< Horizontal radius of the ellipse
    float        verticalRadius{0.f};   //!< Vertical radius of the ellipse
    Angle        startAngle{};          //!< Starting angle of ellipse point generation
    unsigned int pointCount{30u};       //!< Number of points composing the ellipse


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the ellipse would actually render anything.
    ///
    /// An ellipse with non-positive `horizontalRadius` / `verticalRadius`
    /// or `pointCount < 3` produces no geometry and is treated as an
    /// empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return horizontalRadius > 0.f && verticalRadius > 0.f && pointCount >= 3u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the ellipse.
    ///
    /// Returns an empty bbox when `hasVisibleGeometry()` is `false`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2f getLocalBounds() const noexcept
    {
        if (!hasVisibleGeometry()) [[unlikely]]
            return {};

        return {{0.f, 0.f}, {2.f * horizontalRadius, 2.f * verticalRadius}};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the ellipse.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- closed-form rotated-ellipse formula
    /// (one trig lookup, two `sqrt` calls, one matrix build, one
    /// point transform). Independent of `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the ellipse.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- two multiplies. Returns the ellipse's
    /// center, which coincides with `getLocalBounds().getCenter()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return {horizontalRadius, verticalRadius};
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::EllipseShapeData
/// \ingroup graphics
///
/// `sf::EllipseShapeData` is a plain struct that holds the geometric
/// definition of an ellipse: horizontal and vertical radii, the
/// starting angle of the contour, and the number of points used to
/// approximate the curve.
///
/// This struct is typically used together with `sf::ShapeUtils` to
/// produce vertex data, with `sf::Shape` for a drawable wrapper, or
/// directly with a batching system that consumes shape data.
///
/// Like all `*ShapeData` types, it also embeds the common
/// transformable members (`position`, `scale`, `origin`, `rotation`)
/// and the common shape appearance members (`textureRect`,
/// `outlineTextureRect`, `fillColor`, `outlineColor`,
/// `outlineThickness`, `miterLimit`).
///
/// Example usage:
/// \code
/// const sf::EllipseShapeData ellipseData{
///     .fillColor        = sf::Color::Cyan,
///     .outlineColor     = sf::Color::Blue,
///     .outlineThickness = 2.f,
///     .horizontalRadius = 50.f,
///     .verticalRadius   = 30.f,
///     .pointCount       = 40, // more points = smoother ellipse
/// };
/// \endcode
///
/// \see `sf::Shape`, `sf::ShapeUtils`, `sf::CircleShapeData`, `sf::RectangleShapeData`
///
////////////////////////////////////////////////////////////
