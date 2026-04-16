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
/// \brief Data required to draw a pie slice shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API PieSliceShapeData :
    LocalAnchorPointMixin<PieSliceShapeData>,
    GlobalAnchorPointMixin<PieSliceShapeData>
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        radius{0.f};     //!< Radius of the pie slice
    Angle        startAngle{};    //!< Starting angle of the pie slice
    Angle        sweepAngle{};    //!< Sweep angle of the pie slice
    unsigned int pointCount{30u}; //!< Number of points composing the pie slice


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the pie slice would actually render anything.
    ///
    /// A pie slice with non-positive `radius`, `pointCount < 3`, or
    /// zero `sweepAngle` produces no geometry and is treated as an
    /// empty shape. Negative sweeps are accepted and sweep in the
    /// opposite direction (matching `CurvedArrowShapeData`).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return radius > 0.f && pointCount >= 3u && sweepAngle.asRadians() != 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the pie slice.
    ///
    /// Depends on the swept angular range: the bbox tightens for
    /// partial sweeps and degenerates to the full-disk bbox when
    /// the slice covers the whole circle.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- folds the hub, both arc endpoints,
    /// and up to four in-sweep cardinal extrema (a handful of trig
    /// lookups). Independent of `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getLocalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the pie slice.
    ///
    /// Tight up to arc tessellation: the hub plus every rendered
    /// arc sample is folded through the shape transform. The
    /// analytical cardinal-extrema trick used by `getLocalBounds`
    /// can't be reused here because rotation (and non-uniform
    /// scale) move the world extrema off the local cardinals.
    ///
    /// \par Cost
    /// **Vertex iteration, O(pointCount)** -- transforms `pointCount`
    /// vertices (1 hub + `pointCount - 1` arc samples) and folds
    /// them into an AABB. Each iteration performs one trig lookup
    /// and one point transform. Cost grows linearly with `pointCount`;
    /// the default `pointCount = 30` means 30 vertex transforms per
    /// call.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the pie slice.
    ///
    /// For a sector of half-sweep `alpha = sweepAngle / 2`, the
    /// centroid lies on the bisector at distance
    /// `d = (4 * radius * sin(alpha)) / (3 * sweepAngle)` from the
    /// hub `(radius, radius)`.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- one trig lookup for the bisector
    /// direction, one `sin` for the distance, plus a handful of
    /// multiplies and one division. Independent of `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Vec2f getCentroid() const noexcept;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::PieSliceShapeData
/// \ingroup graphics
///
/// `sf::PieSliceShapeData` describes a pie slice (a filled angular
/// sector of a circle) defined by a `radius`, a `startAngle`, and a
/// `sweepAngle`. The contour is approximated by `pointCount` points
/// distributed along the arc.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::CircleShape`, `sf::RingPieSliceShapeData`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
