#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Export.hpp"

#include "Zancle/Graphics/Priv/ShapeMacros.hpp"
#include "Zancle/Graphics/Priv/TransformableMacros.hpp"

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/GlobalAnchorPointMixin.hpp"
#include "Zancle/System/LocalAnchorPointMixin.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/RequireDesignatedInitializers.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a pie slice shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] ZA_GRAPHICS_API PieSliceShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

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

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::PieSliceShapeData
/// \ingroup graphics
///
/// `za::PieSliceShapeData` describes a pie slice (a filled angular
/// sector of a circle) defined by a `radius`, a `startAngle`, and a
/// `sweepAngle`. The contour is approximated by `pointCount` points
/// distributed along the arc.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `za::ShapeUtils` and `za::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `za::Shape`, `za::CircleShape`, `za::RingPieSliceShapeData`, `za::ShapeUtils`
///
////////////////////////////////////////////////////////////
