#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Export.hpp"

#include "Zancle/Graphics/Priv/ShapeMacros.hpp"
#include "Zancle/Graphics/Priv/TransformableMacros.hpp"

#include "Zancle/System/GlobalAnchorPointMixin.hpp"
#include "Zancle/System/LocalAnchorPointMixin.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/RequireDesignatedInitializers.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a rectangle shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] ZA_GRAPHICS_API RectangleShapeData : LocalAnchorPointMixin, GlobalAnchorPointMixin
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f size{}; //!< Size of the rectangle


    ////////////////////////////////////////////////////////////
    /// \brief `true` when the rectangle would actually render anything.
    ///
    /// A rectangle with non-positive `size.x` or `size.y` produces
    /// no geometry and is treated as an empty shape.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool hasVisibleGeometry() const noexcept
    {
        return size.x > 0.f && size.y > 0.f;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Tight local-space AABB of the rectangle.
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
    /// \brief Tight world-space AABB of the rectangle.
    ///
    /// Tight under any scale + rotation because the rectangle's four
    /// vertices coincide with the corners of the local bounding box.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- transforms the 4 corners of the
    /// local bounds rectangle and folds them into an AABB. Effectively
    /// constant time.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const noexcept;


    ////////////////////////////////////////////////////////////
    /// \brief Local-space geometric centroid of the rectangle.
    ///
    /// \par Cost
    /// **Analytical, O(1)** -- two multiplies. Coincides with
    /// `getLocalBounds().getCenter()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getCentroid() const noexcept
    {
        return size * 0.5f;
    }
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::RectangleShapeData
/// \ingroup graphics
///
/// `za::RectangleShapeData` is the plain-data description of an
/// axis-aligned rectangle. It holds the rectangle's `size` together
/// with the standard transformable members (`position`, `scale`,
/// `origin`, `rotation`) and the standard shape appearance members
/// (`textureRect`, `outlineTextureRect`, `fillColor`,
/// `outlineColor`, `outlineThickness`, `miterLimit`).
///
/// Use it as input for `za::ShapeUtils`, `za::RectangleShape`, or a
/// drawable batch when you want to render a rectangle without
/// instantiating a full shape object.
///
/// \see `za::Shape`, `za::RectangleShape`, `za::ShapeUtils`
///
////////////////////////////////////////////////////////////
