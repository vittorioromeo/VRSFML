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
/// \brief Defines the geometric properties of a circle shape
///
/// `za::CircleShapeData` is used to specify the characteristics
/// of a circle, such as its radius and the number of points
/// used to approximate its contour. This data is consumed by
/// `za::ShapeUtils`, `za::Shape`, `za::CircleShape`, and the
/// drawable batching system.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] ZA_GRAPHICS_API CircleShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        radius{0.f};     //!< Radius of the circle
    Angle        startAngle{};    //!< Starting angle of the first generated point on the contour
    unsigned int pointCount{30u}; //!< Number of points composing the circle (higher = smoother contour)


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the circle would actually render anything.
    ///
    /// A circle with non-positive `radius` or `pointCount < 3`
    /// produces no geometry and is treated as an empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return radius > 0.f && pointCount >= 3u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the circle.
    ///
    /// Returns an empty bbox when `hasVisibleGeometry()` is `false`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2f getLocalBounds() const noexcept
    {
        if (!hasVisibleGeometry()) [[unlikely]]
            return {};

        return {{0.f, 0.f}, {2.f * radius, 2.f * radius}};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the circle (rotation-invariant under uniform scale).
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- closed-form rotated-disk formula
    /// (one trig lookup, two `sqrt` calls, one matrix build, one
    /// point transform). Independent of `pointCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the circle.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- one multiply. Returns the disk's
    /// center, which coincides with `getLocalBounds().getCenter()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return {radius, radius};
    }
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::CircleShapeData
/// \ingroup graphics
///
/// `za::CircleShapeData` is the plain-data description of a circle.
/// It holds the radius and tessellation parameters together with
/// the standard transformable members (`position`, `scale`,
/// `origin`, `rotation`) and the standard shape appearance members
/// (`textureRect`, `outlineTextureRect`, `fillColor`,
/// `outlineColor`, `outlineThickness`, `miterLimit`).
///
/// It is intentionally a flat struct so that it can be passed by
/// value, stored in arrays, and consumed by free functions.
///
/// Example usage:
/// \code
/// const za::CircleShapeData data{
///     .position  = {100.f, 100.f},
///     .fillColor = za::Color::Yellow,
///     .radius    = 32.f,
/// };
/// \endcode
///
/// \see `za::Shape`, `za::CircleShape`, `za::ShapeUtils`
///
////////////////////////////////////////////////////////////
