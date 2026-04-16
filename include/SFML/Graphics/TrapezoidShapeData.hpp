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

#include "SFML/Base/MinMaxMacros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw an isosceles trapezoid shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API TrapezoidShapeData :
    LocalAnchorPointMixin<TrapezoidShapeData>,
    GlobalAnchorPointMixin<TrapezoidShapeData>
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float topWidth{0.f};    //!< Width of the top edge
    float bottomWidth{0.f}; //!< Width of the bottom edge
    float height{0.f};      //!< Vertical distance between top and bottom edges


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the trapezoid would actually render anything.
    ///
    /// A trapezoid with non-positive `height`, negative widths, or
    /// zero total width (both edges collapsed) produces no geometry
    /// and is treated as an empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return height > 0.f && topWidth >= 0.f && bottomWidth >= 0.f && (topWidth + bottomWidth) > 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the trapezoid.
    ///
    /// Returns an empty bbox when `hasVisibleGeometry()` is `false`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Rect2f getLocalBounds() const noexcept
    {
        if (!hasVisibleGeometry()) [[unlikely]]
            return {};

        return {{0.f, 0.f}, {SFML_BASE_MAX(topWidth, bottomWidth), height}};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight world-space AABB of the trapezoid.
    ///
    /// \par Cost
    /// **Vertex iteration, O(1)** -- the trapezoid has a fixed 4
    /// vertices, so four point transforms and an AABB fold. Cheap
    /// in absolute terms, but technically a per-vertex loop rather
    /// than a closed-form formula.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the trapezoid.
    ///
    /// The trapezoid is symmetric about `x = max(topWidth, bottomWidth)/2`,
    /// so `Cx` lies on that axis. Along `y`, the classic trapezoid
    /// centroid formula with parallel sides `a` (top) and `b`
    /// (bottom) separated by height `h` gives
    /// `Cy = h * (a + 2*b) / (3 * (a + b))`.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- one `max`, a handful of multiplies
    /// and one division.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        const float maxW = SFML_BASE_MAX(topWidth, bottomWidth);
        const float sum  = topWidth + bottomWidth;

        if (sum <= 0.f) [[unlikely]]
            return {maxW * 0.5f, height * 0.5f};

        return {maxW * 0.5f, height * (topWidth + 2.f * bottomWidth) / (3.f * sum)};
    }
};

} // namespace sf
