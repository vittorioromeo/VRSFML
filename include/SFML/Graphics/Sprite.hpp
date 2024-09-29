#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"

#include "SFML/Base/Math/Fabs.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Geometry that can render a texture, with its
///        own transformations, color, etc.
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API Sprite : Transformable
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the sprite from a sub-rectangle of an eventual source texture
    ///
    /// \param rectangle Sub-rectangle of the eventual texture (specified during drawing)
    ///
    /// \see `setTextureRect`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Sprite(const FloatRect& rectangle);

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
    [[nodiscard]] FloatRect getLocalBounds() const;

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
    [[nodiscard]] FloatRect getGlobalBounds() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    FloatRect textureRect;         //!< Rectangle defining the area of the source texture to display
    Color     color{Color::White}; //!< Color of the sprite
};

} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void spriteToVertices(const Sprite& sprite, Vertex* target)
{
    const auto& [position, size] = sprite.textureRect;
    const Vector2f absSize(base::fabs(size.x), base::fabs(size.y)); // TODO P0: consider dropping support for negative UVs

    // Position
    {
        const auto transform = sprite.getTransform();

        target[0].position.x = transform.a02;
        target[0].position.y = transform.a12;

        target[1].position.x = transform.a01 * absSize.y + transform.a02;
        target[1].position.y = transform.a11 * absSize.y + transform.a12;

        target[2].position.x = transform.a00 * absSize.x + transform.a02;
        target[2].position.y = transform.a10 * absSize.x + transform.a12;

        target[3].position = transform.transformPoint(absSize);
    }

    // Color
    {
        target[0].color = sprite.color;
        target[1].color = sprite.color;
        target[2].color = sprite.color;
        target[3].color = sprite.color;
    }

    // Texture Coordinates
    {
        target[0].texCoords = position;
        target[1].texCoords = position + Vector2f{0.f, size.y};
        target[2].texCoords = position + Vector2f{size.x, 0.f};
        target[3].texCoords = position + size;
    }
}

} // namespace sf::priv


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
