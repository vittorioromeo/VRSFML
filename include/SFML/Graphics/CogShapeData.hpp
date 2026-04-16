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
/// \brief Data required to draw a cog (gear) shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API CogShapeData : LocalAnchorPointMixin<CogShapeData>, GlobalAnchorPointMixin<CogShapeData>
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Distance from the center to the tip of each tooth
    float        innerRadius{0.f}; //!< Distance from the center to the root (base) of each tooth
    unsigned int toothCount{8u};   //!< Number of teeth around the cog
    float toothWidthRatio{0.5f};   //!< Fraction of each angular sector occupied by a tooth (0 = no tooth, 1 = no gap)


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the cog would actually render anything.
    ///
    /// A cog with non-positive `outerRadius`, negative
    /// `innerRadius`, `toothCount < 3`, or `toothWidthRatio`
    /// outside `(0, 1)` produces no geometry and is treated as an
    /// empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return outerRadius > 0.f && innerRadius >= 0.f && toothCount >= 3u && toothWidthRatio > 0.f && toothWidthRatio < 1.f;
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
    /// \brief Tight world-space AABB of the cog.
    ///
    /// Folds all 4*toothCount tooth-tip and root vertices through
    /// the shape transform.
    ///
    /// \par Cost
    /// **Vertex iteration, O(toothCount)** -- transforms
    /// `4 * toothCount` cog vertices and folds them into an AABB.
    /// Each iteration performs one trig lookup and one point
    /// transform. Cost grows linearly with `toothCount`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the cog.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- one multiply. A cog is rotationally
    /// symmetric about its center (every tooth contributes equally),
    /// so the centroid coincides with `getLocalBounds().getCenter()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return {outerRadius, outerRadius};
    }
};

} // namespace sf
