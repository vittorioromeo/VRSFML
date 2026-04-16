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
/// \brief Data required to draw a cross (plus) shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API CrossShapeData :
    LocalAnchorPointMixin<CrossShapeData>,
    GlobalAnchorPointMixin<CrossShapeData>
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f size{};            //!< Overall bounding size of the cross (width x height)
    float armThickness{0.f}; //!< Thickness of the horizontal and vertical arms


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the cross would actually render anything.
    ///
    /// A cross with non-positive `size.x` or `size.y`, or negative
    /// `armThickness`, produces no geometry and is treated as an
    /// empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return size.x > 0.f && size.y > 0.f && armThickness >= 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the cross.
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
    /// \brief Tight world-space AABB of the cross.
    ///
    /// \par Cost
    /// **Vertex iteration, O(1)** -- the cross has a fixed 12
    /// vertices, so twelve point transforms and an AABB fold.
    /// Cheap in absolute terms, but technically a per-vertex loop
    /// rather than a closed-form formula.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the cross.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- two multiplies. The "plus" cross has
    /// two axes of symmetry, so the centroid coincides with
    /// `getLocalBounds().getCenter()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return size * 0.5f;
    }
};

} // namespace sf
