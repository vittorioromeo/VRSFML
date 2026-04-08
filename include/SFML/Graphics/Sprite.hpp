#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"

#include "SFML/System/GlobalAnchorPointMixin.hpp"
#include "SFML/System/LocalAnchorPointMixin.hpp"
#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Geometry that can render a texture, with its
///        own transformations, color, etc.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API Sprite : TransformableMixin<Sprite>, GlobalAnchorPointMixin<Sprite>, LocalAnchorPointMixin<Sprite>
{
    ////////////////////////////////////////////////////////////
    /// \brief Get the local bounding rectangle of the entity
    ///
    /// The returned rectangle is in local coordinates, which means
    /// that it ignores the transformations (translation, rotation,
    /// scale, etc...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// entity in the entity's coordinate system.
    ///
    /// \return Local bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getLocalBounds() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the global bounding rectangle of the entity
    ///
    /// The returned rectangle is in global coordinates, which means
    /// that it takes into account the transformations (translation,
    /// rotation, scale, etc...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// sprite in the global 2D world's coordinate system.
    ///
    /// \return Global bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;
    Rect2f textureRect;         //!< Rectangle defining the area of the source texture to display
    Color  color{Color::White}; //!< Color of the sprite
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::Sprite
/// \ingroup graphics
///
/// `sf::Sprite` is a lightweight aggregate that knows how to
/// draw a textured quad with its own transform and color tint.
/// It is the building block for displaying images on screen.
///
/// Like all transformable types in VRSFML, `sf::Sprite` exposes
/// the standard public members `position`, `scale`, `origin`,
/// and `rotation` (via `SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS`),
/// plus the sprite-specific `textureRect` (sub-rectangle of the
/// source texture to display) and `color` (modulation tint).
///
/// **Lifetime safety:** unlike upstream SFML, `sf::Sprite` does
/// **not** store a pointer to its texture. The texture is
/// supplied at draw time -- either as an extra argument to
/// `sf::RenderTarget::draw` or via `sf::RenderStates::texture`.
/// This eliminates the classic "white square" problem where a
/// sprite outlives its texture.
///
/// Because `sf::Sprite` is an aggregate, it is best constructed
/// inline with C++20 designated initializers.
///
/// See also the note on coordinates and undistorted rendering
/// in `sf::Transformable`.
///
/// Usage example:
/// \code
/// // Load a texture.
/// const auto texture = sf::Texture::loadFromFile("texture.png").value();
///
/// // Create a sprite that displays a sub-rectangle of the texture
/// // with a custom color tint.
/// const sf::Sprite sprite{
///     .position    = {100.f, 25.f},
///     .textureRect = {{10.f, 10.f}, {50.f, 30.f}},
///     .color       = {255u, 255u, 255u, 200u},
/// };
///
/// // Draw it. The texture is passed at draw time.
/// window.draw(sprite, texture);
/// \endcode
///
/// \see `sf::Texture`, `sf::Transformable`, `sf::RenderStates`
///
////////////////////////////////////////////////////////////
