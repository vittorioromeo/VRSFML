#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"

#include "SFML/System/AnchorPointMixin.hpp"
#include "SFML/System/Rect.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Geometry that can render a texture, with its
///        own transformations, color, etc.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API Sprite : TransformableMixin<Sprite>, AnchorPointMixin<Sprite>
{
    ////////////////////////////////////////////////////////////
    /// \brief Get the local bounding rectangle of the entity
    ///
    /// The returned rectangle is in local coordinates, which means
    /// that it ignores the transformations (translation, rotation,
    /// scale, ...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// entity in the entity's coordinate system.
    ///
    /// \return Local bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] FloatRect getLocalBounds() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the global bounding rectangle of the entity
    ///
    /// The returned rectangle is in global coordinates, which means
    /// that it takes into account the transformations (translation,
    /// rotation, scale, ...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// sprite in the global 2D world's coordinate system.
    ///
    /// \return Global bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] FloatRect getGlobalBounds() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_TRANSFORMABLE_DATA_MEMBERS;
    FloatRect textureRect;         //!< Rectangle defining the area of the source texture to display
    Color     color{Color::White}; //!< Color of the sprite
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Sprite
/// \ingroup graphics
///
/// `sf::Sprite` is a drawable class that allows to easily display
/// a texture (or a part of it) on a render target.
///
/// It inherits all the functions from `sf::Transformable`:
/// position, rotation, scale, origin. It also adds sprite-specific
/// properties such as the texture to use, the part of it to display,
/// and some convenience functions to change the overall color of the
/// sprite, or to get its bounding rectangle.
///
/// `sf::Sprite` works in combination with the `sf::Texture` class, which
/// loads and provides the pixel data of a given texture.
///
/// The separation of `sf::Sprite` and `sf::Texture` allows more flexibility
/// and better performances: indeed a `sf::Texture` is a heavy resource,
/// and any operation on it is slow (often too slow for real-time
/// applications). On the other side, a `sf::Sprite` is a lightweight
/// object which can use the pixel data of a `sf::Texture` and draw
/// it with its own transformation/color/blending attributes.
///
/// It is important to note that the `sf::Sprite` instance doesn't
/// even keep a reference to the `sf::Texture` it uses, the texture
/// must be provided prior to drawing the sprite via `sf::RenderStates`.
///
/// See also the note on coordinates and undistorted rendering in `sf::Transformable`.
///
/// Usage example:
/// \code
/// // Load a texture
/// const auto texture = sf::Texture::loadFromFile("texture.png").value();
///
/// // Create a sprite
/// sprite.textureRect = {{10, 10}, {50, 30}};
/// sprite.color = {255, 255, 255, 200};
/// sprite.position = {100.f, 25.f};
///
/// // Draw it
/// window.draw(sprite, texture);
/// \endcode
///
/// \see `sf::Texture`, `sf::Transformable`
///
////////////////////////////////////////////////////////////
