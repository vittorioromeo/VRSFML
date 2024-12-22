#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transformable.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TrivialVector.hpp"


namespace
{
////////////////////////////////////////////////////////////
// Compute the normal of a segment
[[nodiscard, gnu::always_inline, gnu::const]] inline sf::Vector2f computeNormal(sf::Vector2f p1, sf::Vector2f p2)
{
    const sf::Vector2f normal = (p2 - p1).perpendicular();
    const float        length = normal.length();

    return length != 0.f ? normal / length : normal;
}

////////////////////////////////////////////////////////////
// Get bounds of a vertex range
[[nodiscard]] sf::FloatRect getVertexRangeBounds(const sf::base::TrivialVector<sf::Vertex>& data)
{
    if (data.empty())
        return {};

    float left   = data[0].position.x;
    float top    = data[0].position.y;
    float right  = data[0].position.x;
    float bottom = data[0].position.y;

    for (sf::base::SizeT i = 1; i < data.size(); ++i)
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
Shape::Shape(const Settings& settings) :
Transformable{.position = settings.position, .scale = settings.scale, .origin = settings.origin, .rotation = settings.rotation},
m_textureRect{settings.textureRect},
m_outlineTextureRect{settings.outlineTextureRect},
m_fillColor{settings.fillColor},
m_outlineColor{settings.outlineColor},
m_outlineThickness{settings.outlineThickness}
{
}


////////////////////////////////////////////////////////////
void Shape::setTextureRect(const FloatRect& rect)
{
    m_textureRect = rect;
    updateTexCoords(m_vertices);
}


////////////////////////////////////////////////////////////
void Shape::setOutlineTextureRect(const FloatRect& rect)
{
    m_outlineTextureRect = rect;
    updateOutlineTexCoords(m_outlineVertices);
}


////////////////////////////////////////////////////////////
const FloatRect& Shape::getTextureRect() const
{
    return m_textureRect;
}


////////////////////////////////////////////////////////////
const FloatRect& Shape::getOutlineTextureRect() const
{
    return m_outlineTextureRect;
}


////////////////////////////////////////////////////////////
void Shape::setFillColor(Color color)
{
    m_fillColor = color;
    updateFillColors(m_vertices);
}


////////////////////////////////////////////////////////////
Color Shape::getFillColor() const
{
    return m_fillColor;
}


////////////////////////////////////////////////////////////
void Shape::setOutlineColor(Color color)
{
    m_outlineColor = color;
    updateOutlineColors(m_outlineVertices);
}


////////////////////////////////////////////////////////////
Color Shape::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
void Shape::setOutlineThickness(float thickness)
{
    m_outlineThickness = thickness;

    const base::SizeT pointCount = m_vertices.size() - 2;

    base::TrivialVector<Vector2f> points;
    points.reserve(pointCount);

    for (base::SizeT i = 0; i < pointCount; ++i)
        points.unsafeEmplaceBack(m_vertices[i + 1].position);

    update(points.data(), pointCount); // recompute everything because the whole shape must be offset
}


////////////////////////////////////////////////////////////
float Shape::getOutlineThickness() const
{
    return m_outlineThickness;
}


////////////////////////////////////////////////////////////
const FloatRect& Shape::getLocalBounds() const
{
    return m_bounds;
}


////////////////////////////////////////////////////////////
FloatRect Shape::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
[[nodiscard]] base::Span<const Vertex> Shape::getFillVertices() const
{
    return {m_vertices.data(), m_vertices.size()};
}


////////////////////////////////////////////////////////////
[[nodiscard]] base::Span<const Vertex> Shape::getOutlineVertices() const
{
    return {m_outlineVertices.data(), m_outlineVertices.size()};
}


////////////////////////////////////////////////////////////
void Shape::update(const sf::Vector2f* points, const base::SizeT pointCount)
{
    if (!updateImplResizeVerticesVector(pointCount))
        return;

    // Position
    for (base::SizeT i = 0; i < pointCount; ++i)
        m_vertices[i + 1].position = points[i];

    updateImplFromVerticesPositions(pointCount);
}


////////////////////////////////////////////////////////////
bool Shape::updateImplResizeVerticesVector(const base::SizeT pointCount)
{
    if (pointCount < 3u)
    {
        m_vertices.clear();
        m_outlineVertices.clear();
        return false;
    }

    m_vertices.resize(pointCount + 2u); // + 2 for center and repeated first point
    return true;
}


////////////////////////////////////////////////////////////
void Shape::updateImplFromVerticesPositions(const base::SizeT pointCount)
{
    m_vertices[pointCount + 1].position = m_vertices[1].position;

    // Update the bounding rectangle
    m_vertices[0]  = m_vertices[1]; // so that the result of getBounds() is correct
    m_insideBounds = getVertexRangeBounds(m_vertices);

    // Compute the center and make it the first vertex
    m_vertices[0].position = m_insideBounds.getCenter();

    // Updates
    updateFillColors(m_vertices);
    updateTexCoords(m_vertices);
    updateOutline(m_outlineVertices, m_vertices);
    updateOutlineTexCoords(m_outlineVertices);
}


////////////////////////////////////////////////////////////
void Shape::drawOnto(RenderTarget& renderTarget, const Texture* texture, RenderStates states) const
{
    states.transform *= getTransform();
    states.coordinateType = CoordinateType::Pixels;
    states.texture        = texture;

    // Render the inside
    renderTarget.draw(m_vertices, PrimitiveType::TriangleFan, states);

    // Render the outline
    if (m_outlineThickness != 0)
        renderTarget.draw(m_outlineVertices, PrimitiveType::TriangleStrip, states);
}


////////////////////////////////////////////////////////////
void Shape::updateFillColors(base::TrivialVector<Vertex>& vertices) const
{
    for (Vertex& vertex : vertices)
        vertex.color = m_fillColor;
}


////////////////////////////////////////////////////////////
void Shape::updateTexCoords(base::TrivialVector<Vertex>& vertices) const
{
    // Make sure not to divide by zero when the points are aligned on a vertical or horizontal line
    const Vector2f safeInsideSize(m_insideBounds.size.x > 0 ? m_insideBounds.size.x : 1.f,
                                  m_insideBounds.size.y > 0 ? m_insideBounds.size.y : 1.f);

    for (Vertex& vertex : vertices)
    {
        const Vector2f ratio = (vertex.position - m_insideBounds.position).componentWiseDiv(safeInsideSize);
        vertex.texCoords     = m_textureRect.position + m_textureRect.size.componentWiseMul(ratio);
    }
}


////////////////////////////////////////////////////////////
void Shape::updateOutlineTexCoords(base::TrivialVector<Vertex>& outlineVertices) const
{
    // TODO P0:
    for (Vertex& vertex : outlineVertices)
        vertex.texCoords = m_outlineTextureRect.position;
}


////////////////////////////////////////////////////////////
void Shape::updateOutline(base::TrivialVector<Vertex>& outlineVertices, const base::TrivialVector<Vertex>& vertices)
{
    // Return if there is no outline
    if (m_outlineThickness == 0.f)
    {
        outlineVertices.clear();
        m_bounds = m_insideBounds;
        return;
    }

    const base::SizeT count = vertices.size() - 2;
    outlineVertices.resize((count + 1) * 2);

    for (base::SizeT i = 0; i < count; ++i)
    {
        const base::SizeT index = i + 1;

        // Get the two segments shared by the current point
        const Vector2f p0 = (i == 0) ? vertices[count].position : vertices[index - 1].position;
        const Vector2f p1 = vertices[index].position;
        const Vector2f p2 = vertices[index + 1].position;

        // Compute their normal
        Vector2f n1 = computeNormal(p0, p1);
        Vector2f n2 = computeNormal(p1, p2);

        // Make sure that the normals point towards the outside of the shape
        // (this depends on the order in which the points were defined)
        if (n1.dot(vertices[0].position - p1) > 0)
            n1 = -n1;
        if (n2.dot(vertices[0].position - p1) > 0)
            n2 = -n2;

        // Combine them to get the extrusion direction
        const float    factor = 1.f + (n1.x * n2.x + n1.y * n2.y);
        const Vector2f normal = (n1 + n2) / factor;

        // Update the outline points
        outlineVertices[i * 2 + 0].position = p1;
        outlineVertices[i * 2 + 1].position = p1 + normal * m_outlineThickness;
    }

    // Duplicate the first point at the end, to close the outline
    outlineVertices[count * 2 + 0].position = outlineVertices[0].position;
    outlineVertices[count * 2 + 1].position = outlineVertices[1].position;

    // Update outline colors
    updateOutlineColors(outlineVertices);

    // Update the shape's bounds
    m_bounds = getVertexRangeBounds(outlineVertices);
}


////////////////////////////////////////////////////////////
void Shape::updateOutlineColors(base::TrivialVector<Vertex>& outlineVertices) const
{
    for (Vertex& outlineVertex : outlineVertices)
        outlineVertex.color = m_outlineColor;
}

} // namespace sf
