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
    updateVertices();
}


////////////////////////////////////////////////////////////
void Sprite::setTextureRect(const IntRect& rectangle)
{
    if (rectangle == m_textureRect)
        return;

    m_textureRect = rectangle;
    updateVertices();
}


////////////////////////////////////////////////////////////
void Sprite::setColor(Color color)
{
    for (Vertex& vertex : m_vertices)
        vertex.color = color;
}


////////////////////////////////////////////////////////////
const IntRect& Sprite::getTextureRect() const
{
    return m_textureRect;
}


////////////////////////////////////////////////////////////
Color Sprite::getColor() const
{
    return m_vertices[0].color;
}


////////////////////////////////////////////////////////////
FloatRect Sprite::getLocalBounds() const
{
    // Last vertex position is equal to texture rect size absolute value
    return {{0.f, 0.f}, m_vertices[3].position};
}


////////////////////////////////////////////////////////////
FloatRect Sprite::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
base::Span<const Vertex> Sprite::getVertices() const
{
    return m_vertices;
}


////////////////////////////////////////////////////////////
void Sprite::updateVertices()
{
    const auto [position, size] = m_textureRect.to<FloatRect>();

    // Absolute value is used to support negative texture rect sizes
    const Vector2f absSize(base::fabs(size.x), base::fabs(size.y));

    // Update positions
    m_vertices[0].position = {0.f, 0.f};
    m_vertices[1].position = {0.f, absSize.y};
    m_vertices[2].position = {absSize.x, 0.f};
    m_vertices[3].position = absSize;

    // Update texture coordinates
    m_vertices[0].texCoords = position;
    m_vertices[1].texCoords = position + Vector2f(0.f, size.y);
    m_vertices[2].texCoords = position + Vector2f(size.x, 0.f);
    m_vertices[3].texCoords = position + size;
}

} // namespace sf
