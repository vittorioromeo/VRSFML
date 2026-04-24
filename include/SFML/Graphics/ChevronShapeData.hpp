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
/// \brief Data required to draw a chevron shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API ChevronShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f size{};         //!< Overall bounding size of the chevron (width x height)
    float thickness{0.f}; //!< Stroke thickness measured vertically at the open (back) end


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the chevron would actually render anything.
    ///
    /// A chevron with non-positive `size.x`/`size.y` or negative
    /// `thickness` produces no geometry and is treated as an empty
    /// shape. (A `thickness` at or beyond `size.y / 2` cleanly
    /// degenerates to a filled triangle -- still considered visible.)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return size.x > 0.f && size.y > 0.f && thickness >= 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the chevron.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2f getLocalBounds() const noexcept
    {
        if (!hasVisibleGeometry()) [[unlikely]]
            return {};

        return {{0.f, 0.f}, size};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the chevron.
    ///
    /// \par Cost
    /// **Vertex iteration, O(1)** -- the chevron has a fixed 6
    /// vertices, so six point transforms and an AABB fold. Cheap
    /// in absolute terms, but technically a per-vertex loop rather
    /// than a closed-form formula.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the chevron.
    ///
    /// The chevron is symmetric about `y = size.y / 2`, so `Cy`
    /// lies on that axis. With `s = thickness / size.y` and
    /// `w = size.x`, direct shoelace on the 6 perimeter vertices
    /// gives the closed form
    /// `Cx = w * (1 - (1 - 2s)^3) / (12 * s * (1 - s))`
    /// for `0 < s < 0.5`. When `s` reaches or exceeds `0.5`, the
    /// chevron cleanly degenerates to a filled triangle with
    /// vertices `(0, 0)`, `(w, size.y / 2)`, `(0, size.y)`, whose
    /// centroid is `(w / 3, size.y / 2)`.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- a handful of multiplies and one
    /// division.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        const float w = size.x;
        const float h = size.y;

        if (!hasVisibleGeometry() || h <= 0.f) [[unlikely]]
            return {w * 0.5f, h * 0.5f};

        // Degenerate filled-triangle fallback when the inner tip collapses to x = 0.
        if (thickness * 2.f >= h) [[unlikely]]
            return {w * (1.f / 3.f), h * 0.5f};

        const float s          = thickness / h;
        const float oneMinus2s = 1.f - 2.f * s;
        const float denom      = 12.f * s * (1.f - s);

        if (denom <= 0.f) [[unlikely]]
            return {w * 0.5f, h * 0.5f};

        return {w * (1.f - oneMinus2s * oneMinus2s * oneMinus2s) / denom, h * 0.5f};
    }
};

} // namespace sf
