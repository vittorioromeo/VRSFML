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
/// \brief Data required to draw a star shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API StarShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Distance from the center to the outer points
    float        innerRadius{0.f}; //!< Distance from the center to the inner points
    unsigned int pointCount{5u};   //!< Number of points of the star (e.g., 5 for a 5-pointed star)


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the star would actually render anything.
    ///
    /// A star with non-positive `outerRadius`, negative `innerRadius`,
    /// or `pointCount < 2` produces no geometry and is treated as an
    /// empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return outerRadius > 0.f && innerRadius >= 0.f && pointCount >= 2u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Local-space AABB (enclosing outer-tip disk).
    ///
    /// Returns an empty bbox when `hasVisibleGeometry()` is `false`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2f getLocalBounds() const noexcept
    {
        if (!hasVisibleGeometry()) [[unlikely]]
            return {};

        return {{0.f, 0.f}, {2.f * outerRadius, 2.f * outerRadius}};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the star.
    ///
    /// Folds the 2*pointCount actual tip and notch vertices through
    /// the shape transform (outer tips drive the bbox in general).
    ///
    /// \par Cost
    /// **Vertex iteration, O(pointCount)** -- transforms `2 * pointCount`
    /// star vertices and folds them into an AABB. Each iteration
    /// performs one trig lookup and one point transform. Cost grows
    /// linearly with `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the star.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- one multiply. A regular star polygon
    /// has `pointCount`-fold rotational symmetry about its center,
    /// so the centroid coincides with `getLocalBounds().getCenter()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return {outerRadius, outerRadius};
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::StarShapeData
/// \ingroup graphics
///
/// `sf::StarShapeData` describes a regular star polygon. It has
/// `pointCount` outer "points" and `pointCount` inner concave
/// vertices. `outerRadius` is the distance from the center to the
/// tips, and `innerRadius` is the distance from the center to the
/// concave vertices between them. The ratio between the two
/// determines how "spiky" the star looks.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
