#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/GlobalAnchorPointMixin.hpp"
#include "SFML/System/LocalAnchorPointMixin.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a rounded rectangle shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RoundedRectangleShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f        size{};                //!< Size of the rounded rectangle
    float        cornerRadius{0.f};     //!< Radius of the corners of the rounded rectangle
    unsigned int cornerPointCount{16u}; //!< Number of points used to draw the corners of the rounded rectangle


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the rounded rectangle would actually render anything.
    ///
    /// A rounded rectangle with non-positive `size.x` / `size.y`,
    /// negative `cornerRadius`, or `cornerPointCount < 1` produces
    /// no geometry and is treated as an empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return size.x > 0.f && size.y > 0.f && cornerRadius >= 0.f && cornerPointCount >= 1u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the rounded rectangle.
    ///
    /// Returns an empty bbox when `hasVisibleGeometry()` is `false`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2f getLocalBounds() const noexcept
    {
        if (!hasVisibleGeometry()) [[unlikely]]
            return {};

        return {{0.f, 0.f}, size};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the rounded rectangle.
    ///
    /// Tight up to corner-arc tessellation: each of the
    /// `4*cornerPointCount` corner-arc vertices is folded through
    /// the shape transform.
    ///
    /// \par Cost
    /// **Vertex iteration, O(cornerPointCount)** -- transforms
    /// `4 * cornerPointCount` corner-arc samples and folds them
    /// into an AABB. Each iteration performs one trig lookup and
    /// one point transform. Cost grows linearly with
    /// `cornerPointCount`; the default `cornerPointCount = 16`
    /// means 64 vertex transforms per call.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the rounded rectangle.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- two multiplies. The rounded rectangle
    /// has two axes of symmetry, so the centroid coincides with
    /// `getLocalBounds().getCenter()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return size * 0.5f;
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::RoundedRectangleShapeData
/// \ingroup graphics
///
/// `sf::RoundedRectangleShapeData` describes an axis-aligned
/// rectangle whose corners are rounded with a quarter-circle of
/// radius `cornerRadius`. The smoothness of each rounded corner is
/// controlled by `cornerPointCount`.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::RectangleShapeData`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
