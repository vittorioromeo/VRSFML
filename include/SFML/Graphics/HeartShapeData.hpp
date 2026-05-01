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
/// \brief Data required to draw a heart shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API HeartShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f        size{};          //!< Overall bounding size of the heart (width x height)
    unsigned int pointCount{64u}; //!< Number of points composing the heart contour (higher = smoother)


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the heart would actually render anything.
    ///
    /// A heart with non-positive `size.x` / `size.y`, or
    /// `pointCount < 3`, produces no geometry and is treated as an
    /// empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return size.x > 0.f && size.y > 0.f && pointCount >= 3u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the heart.
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
    /// \brief Tight world-space AABB of the heart.
    ///
    /// Tight up to tessellation: the sampled perimeter points are
    /// folded individually through the shape transform.
    ///
    /// \par Cost
    /// **Vertex iteration, O(pointCount)** -- transforms `pointCount`
    /// sampled heart-curve points and folds them into an AABB.
    /// Each iteration performs one trig lookup, several multiplies
    /// for the parametric formula, and one point transform. Cost
    /// grows linearly with `pointCount`; the default `pointCount = 64`
    /// is the dominant per-call cost for this shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the heart.
    ///
    /// The heart's parametric `sin^3` curve has no pleasant closed
    /// form for its centroid, so this is computed numerically by
    /// applying the polygon shoelace/first-moment formula to the
    /// same sampled perimeter used for rendering.
    ///
    /// \par Cost
    /// **Vertex iteration, O(pointCount)** -- samples `pointCount`
    /// heart-curve points and folds them via two-pass shoelace to
    /// compute the signed area and first moments. Each iteration
    /// performs one trig lookup plus several multiplies. Cost grows
    /// linearly with `pointCount`; the default `pointCount = 64` is
    /// the dominant per-call cost for this shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Vec2f getCentroid() const noexcept;
};

} // namespace sf
