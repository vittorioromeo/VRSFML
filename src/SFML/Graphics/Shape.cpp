////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/System/Vector2.hpp>

#include <cassert>
#include <cstddef>

namespace
{
////////////////////////////////////////////////////////////
// Compute the normal of a segment
[[nodiscard]] sf::Vector2f computeNormal(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
    sf::Vector2f normal = (p2 - p1).perpendicular();
    const float  length = normal.length();
    if (length != 0.f)
        normal /= length;
    return normal;
}

////////////////////////////////////////////////////////////
// Get bounds of a vertex range
[[nodiscard]] sf::FloatRect getVertexRangeBounds(const std::vector<sf::Vertex>& data)
{
    if (data.empty())
    {
        return {};
    }

    float left   = data[0].position.x;
    float top    = data[0].position.y;
    float right  = data[0].position.x;
    float bottom = data[0].position.y;

    for (std::size_t i = 1; i < data.size(); ++i)
    {
        const sf::Vector2f position = data[i].position;

        // Update left and right
        if (position.x < left)
            left = position.x;
        else if (position.x > right)
            right = position.x;

        // Update top and bottom
        if (position.y < top)
            top = position.y;
        else if (position.y > bottom)
            bottom = position.y;
    }

    return {{left, top}, {right - left, bottom - top}};
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
void Shape::setTextureRect(const IntRect& rect)
{
    m_textureRect = rect;
    updateTexCoords();
}


////////////////////////////////////////////////////////////
const IntRect& Shape::getTextureRect() const
{
    return m_textureRect;
}


////////////////////////////////////////////////////////////
void Shape::setFillColor(const Color& color)
{
    m_fillColor = color;
    updateFillColors();
}


////////////////////////////////////////////////////////////
const Color& Shape::getFillColor() const
{
    return m_fillColor;
}


////////////////////////////////////////////////////////////
void Shape::setOutlineColor(const Color& color)
{
    m_outlineColor = color;
    updateOutlineColors();
}


////////////////////////////////////////////////////////////
const Color& Shape::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
void Shape::setOutlineThickness(float thickness)
{
    m_outlineThickness = thickness;

    const std::size_t pointCount = m_vertices.size() - 2;

    std::vector<Vector2f> points;
    points.reserve(pointCount);

    for (std::size_t i = 0; i < pointCount; ++i)
        points.push_back(m_vertices[i + 1].position);

    update(points.data(), pointCount); // recompute everything because the whole shape must be offset
}


////////////////////////////////////////////////////////////
float Shape::getOutlineThickness() const
{
    return m_outlineThickness;
}


////////////////////////////////////////////////////////////
FloatRect Shape::getLocalBounds() const
{
    return m_bounds;
}


////////////////////////////////////////////////////////////
FloatRect Shape::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
void Shape::update(const sf::Vector2f* points, const std::size_t pointCount)
{
    // Get the total number of points of the shape
    if (pointCount < 3)
    {
        m_vertices.resize(0);
        m_outlineVertices.resize(0);
        return;
    }

    m_vertices.resize(pointCount + 2); // + 2 for center and repeated first point

    // Position
    for (std::size_t i = 0; i < pointCount; ++i)
        m_vertices[i + 1].position = points[i];

    m_vertices[pointCount + 1].position = m_vertices[1].position;

    // Update the bounding rectangle
    m_vertices[0]  = m_vertices[1]; // so that the result of getBounds() is correct
    m_insideBounds = getVertexRangeBounds(m_vertices);

    // Compute the center and make it the first vertex
    m_vertices[0].position = m_insideBounds.getCenter();

    // Color
    updateFillColors();

    // Texture coordinates
    updateTexCoords();

    // Outline
    updateOutline();
}


////////////////////////////////////////////////////////////
void Shape::updateFillColors()
{
    for (Vertex& vertex : m_vertices)
        vertex.color = m_fillColor;
}


////////////////////////////////////////////////////////////
void Shape::updateTexCoords()
{
    const FloatRect convertedTextureRect(m_textureRect);

    // Make sure not to divide by zero when the points are aligned on a vertical or horizontal line
    const Vector2f safeInsideSize(m_insideBounds.size.x > 0 ? m_insideBounds.size.x : 1.f,
                                  m_insideBounds.size.y > 0 ? m_insideBounds.size.y : 1.f);

    for (Vertex& vertex : m_vertices)
    {
        const Vector2f ratio = (vertex.position - m_insideBounds.position).cwiseDiv(safeInsideSize);
        vertex.texCoords     = convertedTextureRect.position + convertedTextureRect.size.cwiseMul(ratio);
    }
}


////////////////////////////////////////////////////////////
void Shape::updateOutline()
{
    // Return if there is no outline
    if (m_outlineThickness == 0.f)
    {
        m_outlineVertices.clear();
        m_bounds = m_insideBounds;
        return;
    }

    const std::size_t count = m_vertices.size() - 2;
    m_outlineVertices.resize((count + 1) * 2);

    for (std::size_t i = 0; i < count; ++i)
    {
        const std::size_t index = i + 1;

        // Get the two segments shared by the current point
        const Vector2f p0 = (i == 0) ? m_vertices[count].position : m_vertices[index - 1].position;
        const Vector2f p1 = m_vertices[index].position;
        const Vector2f p2 = m_vertices[index + 1].position;

        // Compute their normal
        Vector2f n1 = computeNormal(p0, p1);
        Vector2f n2 = computeNormal(p1, p2);

        // Make sure that the normals point towards the outside of the shape
        // (this depends on the order in which the points were defined)
        if (n1.dot(m_vertices[0].position - p1) > 0)
            n1 = -n1;
        if (n2.dot(m_vertices[0].position - p1) > 0)
            n2 = -n2;

        // Combine them to get the extrusion direction
        const float    factor = 1.f + (n1.x * n2.x + n1.y * n2.y);
        const Vector2f normal = (n1 + n2) / factor;

        // Update the outline points
        m_outlineVertices[i * 2 + 0].position = p1;
        m_outlineVertices[i * 2 + 1].position = p1 + normal * m_outlineThickness;
    }

    // Duplicate the first point at the end, to close the outline
    m_outlineVertices[count * 2 + 0].position = m_outlineVertices[0].position;
    m_outlineVertices[count * 2 + 1].position = m_outlineVertices[1].position;

    // Update outline colors
    updateOutlineColors();

    // Update the shape's bounds
    m_bounds = getVertexRangeBounds(m_outlineVertices);
}


////////////////////////////////////////////////////////////
void Shape::updateOutlineColors()
{
    for (Vertex& outlineVertex : m_outlineVertices)
        outlineVertex.color = m_outlineColor;
}

} // namespace sf
