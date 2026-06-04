#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
/// \brief Data required to draw a ring shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] ZA_GRAPHICS_API RingShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Distance from the center to the outer points
    float        innerRadius{0.f}; //!< Distance from the center to the inner points
    Angle        startAngle{};     //!< Starting angle of the ring point generation
    unsigned int pointCount{30u};  //!< Number of points composing the ring


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the ring would actually render anything.
    ///
    /// A ring with `outerRadius <= 0`, `innerRadius < 0`,
    /// `innerRadius >= outerRadius`, or `pointCount < 3` produces no
    /// geometry and is treated as an empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return outerRadius > 0.f && innerRadius >= 0.f && innerRadius < outerRadius && pointCount >= 3u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the ring.
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
    /// \brief Tight world-space AABB of the ring.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- closed-form rotated-disk formula on
    /// the outer radius (one trig lookup, two `sqrt` calls, one
    /// matrix build, one point transform). Independent of
    /// `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the ring.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- one multiply. The annulus is
    /// centrally symmetric, so the centroid coincides with the
    /// ring's geometric center.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return {outerRadius, outerRadius};
    }
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::RingShapeData
/// \ingroup graphics
///
/// `za::RingShapeData` describes a ring (an annulus): a closed band
/// between two concentric circles defined by `outerRadius` and
/// `innerRadius`. The contour is approximated by `pointCount` points
/// per circle, and `startAngle` shifts where on the circle the
/// tessellation begins.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `za::ShapeUtils` and `za::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `za::Shape`, `za::CircleShape`, `za::RingPieSliceShapeData`, `za::ShapeUtils`
///
////////////////////////////////////////////////////////////
