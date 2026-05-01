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
/// \brief Data required to draw a ring pie slice shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RingPieSliceShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Radius of the outer circle defining the boundary
    float        innerRadius{0.f}; //!< Radius of the inner circle defining the hole
    Angle        startAngle{};     //!< Starting angle of the pie slice
    Angle        sweepAngle{};     //!< Sweep angle of the pie slice
    unsigned int pointCount{30u};  //!< Number of points composing the pie slice


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the ring pie slice would actually render anything.
    ///
    /// A ring pie slice with `outerRadius <= 0`, `innerRadius < 0`,
    /// `innerRadius >= outerRadius`, `pointCount < 3`, or zero
    /// `sweepAngle` produces no geometry and is treated as an empty
    /// shape. Negative sweeps are accepted and sweep in the opposite
    /// direction (matching `CurvedArrowShapeData`).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return outerRadius > 0.f && innerRadius >= 0.f && innerRadius < outerRadius && pointCount >= 3u &&
               sweepAngle.asRadians() != 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the ring pie slice.
    ///
    /// Takes the swept angle into account: narrow sweeps produce
    /// correspondingly narrow bounding boxes. The inner arc and the
    /// radial connector segments are also considered, so the result
    /// is tight for any valid combination of inner/outer radii.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- folds outer + inner arc endpoints
    /// and up to four in-sweep cardinal extrema (a handful of trig
    /// lookups). Independent of `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getLocalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the ring pie slice.
    ///
    /// Tight up to arc tessellation: every rendered outer- and
    /// inner-arc sample is folded through the shape transform.
    /// The analytical cardinal-extrema trick used by
    /// `getLocalBounds` can't be reused here because rotation
    /// (and non-uniform scale) move the world extrema off the
    /// local cardinals.
    ///
    /// \par Cost
    /// **Vertex iteration, O(pointCount)** -- transforms
    /// `2 * pointCount` vertices (outer arc + inner arc) and folds
    /// them into an AABB. Each iteration performs one trig lookup
    /// and one point transform. Cost grows linearly with `pointCount`;
    /// the default `pointCount = 30` means 60 vertex transforms per
    /// call.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the ring pie slice.
    ///
    /// For an annulus sector with outer radius `R`, inner radius `r`,
    /// half-sweep `alpha = sweepAngle / 2`, the centroid lies on the
    /// bisector at distance
    /// `d = (4 * sin(alpha) * (R^3 - r^3)) / (3 * sweepAngle * (R^2 - r^2))`
    /// from the ring's geometric center `(R, R)`.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- one trig lookup for the bisector
    /// direction, one `sin` for the distance, plus a handful of
    /// multiplies and two divisions. Independent of `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Vec2f getCentroid() const noexcept;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::RingPieSliceShapeData
/// \ingroup graphics
///
/// `sf::RingPieSliceShapeData` describes a slice of a ring -- the
/// intersection of a `sf::RingShapeData` and a `sf::PieSliceShapeData`.
/// It is defined by an `outerRadius`, an `innerRadius`, a `startAngle`,
/// and a `sweepAngle`. `pointCount` controls the smoothness of the
/// arcs.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::RingShapeData`, `sf::PieSliceShapeData`, `sf::CurvedArrowShapeData`
///
////////////////////////////////////////////////////////////
