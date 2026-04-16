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
/// \brief Data required to draw a rectangle shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RectangleShapeData :
    LocalAnchorPointMixin<RectangleShapeData>,
    GlobalAnchorPointMixin<RectangleShapeData>
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

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

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::RectangleShapeData
/// \ingroup graphics
///
/// `sf::RectangleShapeData` is the plain-data description of an
/// axis-aligned rectangle. It holds the rectangle's `size` together
/// with the standard transformable members (`position`, `scale`,
/// `origin`, `rotation`) and the standard shape appearance members
/// (`textureRect`, `outlineTextureRect`, `fillColor`,
/// `outlineColor`, `outlineThickness`, `miterLimit`).
///
/// Use it as input for `sf::ShapeUtils`, `sf::RectangleShape`, or a
/// drawable batch when you want to render a rectangle without
/// instantiating a full shape object.
///
/// \see `sf::Shape`, `sf::RectangleShape`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
