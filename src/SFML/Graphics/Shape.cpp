#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transformable.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TrivialVector.hpp"


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
    updateTexCoords();
}


////////////////////////////////////////////////////////////
void Shape::setOutlineTextureRect(const FloatRect& rect)
{
    m_outlineTextureRect = rect;
    updateOutlineTexCoords();
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
    updateFillColors();
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
    updateOutlineColors();
}


////////////////////////////////////////////////////////////
Color Shape::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
void Shape::setOutlineThickness(float thickness)
{
    if (m_outlineThickness == thickness)
        return;

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
    return {m_vertices.data(), m_verticesEndIndex};
}


////////////////////////////////////////////////////////////
[[nodiscard]] base::Span<const Vertex> Shape::getOutlineVertices() const
{
    return {m_vertices.data() + m_verticesEndIndex, m_vertices.size() - m_verticesEndIndex};
}


////////////////////////////////////////////////////////////
void Shape::update(const sf::Vector2f* points, const base::SizeT pointCount)
{
    if (!updateImplResizeVerticesVector(pointCount))
        return;

    // Position
    for (base::SizeT i = 0; i < pointCount; ++i)
        m_vertices[i + 1].position = points[i];

    updateImplFromVerticesPositions(pointCount, /* mustUpdateBounds */ true);
}


////////////////////////////////////////////////////////////
bool Shape::updateImplResizeVerticesVector(const base::SizeT pointCount)
{
    if (pointCount < 3u)
    {
        m_vertices.clear();
        m_verticesEndIndex = 0;
        return false;
    }

    m_vertices.resize(pointCount + 2u); // + 2 for center and repeated first point
    m_verticesEndIndex = pointCount + 2u;
    return true;
}


////////////////////////////////////////////////////////////
void Shape::updateImplFromVerticesPositions(const base::SizeT pointCount, const bool mustUpdateBounds)
{
    m_vertices[pointCount + 1].position = m_vertices[1].position;

    // Update the bounding rectangle
    m_vertices[0]  = m_vertices[1]; // so that the result of getBounds() is correct
    m_insideBounds = getVertexRangeBounds(m_vertices.data(), m_verticesEndIndex);

    // Compute the center and make it the first vertex
    m_vertices[0].position = m_insideBounds.getCenter();

    // Updates
    updateFillColors();
    updateTexCoords();
    updateOutline(mustUpdateBounds);
    updateOutlineTexCoords();
}


////////////////////////////////////////////////////////////
void Shape::drawOnto(RenderTarget& renderTarget, const Texture* texture, RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = texture;

    // Render the inside
    renderTarget.drawVertices(m_vertices.data(), m_verticesEndIndex, PrimitiveType::TriangleFan, states);

    // Render the outline
    if (m_outlineThickness != 0.f)
        renderTarget.drawVertices(m_vertices.data() + m_verticesEndIndex,
                                  m_vertices.size() - m_verticesEndIndex,
                                  PrimitiveType::TriangleStrip,
                                  states);
}


////////////////////////////////////////////////////////////
void Shape::updateFillColors()
{
    const auto* end = m_vertices.data() + m_verticesEndIndex;
    for (Vertex* vertex = m_vertices.data(); vertex != end; ++vertex)
        vertex->color = m_fillColor;
}


////////////////////////////////////////////////////////////
void Shape::updateTexCoords()
{
    // Make sure not to divide by zero when the points are aligned on a vertical or horizontal line
    const Vector2f safeInsideSize(m_insideBounds.size.x > 0 ? m_insideBounds.size.x : 1.f,
                                  m_insideBounds.size.y > 0 ? m_insideBounds.size.y : 1.f);

    for (Vertex& vertex : m_vertices)
    {
        const Vector2f ratio = (vertex.position - m_insideBounds.position).componentWiseDiv(safeInsideSize);
        vertex.texCoords     = m_textureRect.position + m_textureRect.size.componentWiseMul(ratio);
    }
}


////////////////////////////////////////////////////////////
void Shape::updateOutlineTexCoords()
{
    // TODO P0:
    const auto* end = m_vertices.data() + m_vertices.size();
    for (Vertex* vertex = m_vertices.data() + m_verticesEndIndex; vertex != end; ++vertex)
        vertex->texCoords = m_outlineTextureRect.position;
}


////////////////////////////////////////////////////////////
void Shape::updateOutline(const bool mustUpdateBounds)
{
    // Return if there is no outline
    if (m_outlineThickness == 0.f)
    {
        m_verticesEndIndex = m_vertices.size();

        if (mustUpdateBounds)
            m_bounds = m_insideBounds;

        return;
    }

    const base::SizeT count = m_vertices.size() - 2;
    m_vertices.resize(m_verticesEndIndex + (count + 1) * 2);

    updateOutlineImpl(m_outlineThickness, m_verticesEndIndex, m_vertices.data(), count);

    // Update outline colors
    updateOutlineColors();

    // Update the shape's bounds from outline vertices
    if (mustUpdateBounds)
        m_bounds = getVertexRangeBounds(m_vertices.data() + m_verticesEndIndex, m_vertices.size() - m_verticesEndIndex);
}


////////////////////////////////////////////////////////////
void Shape::updateOutlineColors()
{
    const auto* end = m_vertices.data() + m_vertices.size();
    for (Vertex* vertex = m_vertices.data() + m_verticesEndIndex; vertex != end; ++vertex)
        vertex->color = m_outlineColor;
}

} // namespace sf
