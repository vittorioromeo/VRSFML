#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Sprite.hpp"

#include "SFML/Base/Math/Fabs.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Sprite::Sprite(const IntRect& rectangle) : m_textureRect(rectangle)
{
}


////////////////////////////////////////////////////////////
void Sprite::setTextureRect(const IntRect& rectangle)
{
    m_textureRect = rectangle;
}


////////////////////////////////////////////////////////////
void Sprite::setColor(Color color)
{
    m_color = color;
}


////////////////////////////////////////////////////////////
const IntRect& Sprite::getTextureRect() const
{
    return m_textureRect;
}


////////////////////////////////////////////////////////////
Color Sprite::getColor() const
{
    return m_color;
}


////////////////////////////////////////////////////////////
FloatRect Sprite::getLocalBounds() const
{
    return {{0.f, 0.f},
            {base::fabs(static_cast<float>(m_textureRect.position.x)),
             base::fabs(static_cast<float>(m_textureRect.position.y))}};
}


////////////////////////////////////////////////////////////
FloatRect Sprite::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
void Sprite::getPreTransformedVertices(Vertex* target) const
{
    const auto [position, size] = m_textureRect.to<FloatRect>();

    // Absolute value is used to support negative texture rect sizes
    const Vector2f absSize(base::fabs(size.x), base::fabs(size.y));

    // Update positions
    getTransform().transformSpritePoints(target[0].position, target[1].position, target[2].position, target[3].position, absSize);

    // Update color
    target[0].color = m_color;
    target[1].color = m_color;
    target[2].color = m_color;
    target[3].color = m_color;

    // Update texture coordinates
    target[0].texCoords = position;
    target[1].texCoords = position + Vector2f{0.f, size.y};
    target[2].texCoords = position + Vector2f{size.x, 0.f};
    target[3].texCoords = position + size;
}

} // namespace sf
