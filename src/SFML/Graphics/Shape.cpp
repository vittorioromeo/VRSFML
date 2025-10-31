// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Shape.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/VertexUtils.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Shape::Shape(const Data& data) :
    m_textureRect{data.textureRect},
    m_outlineTextureRect{data.outlineTextureRect},
    m_outlineThickness{data.outlineThickness},
    m_fillColor{data.fillColor},
    m_outlineColor{data.outlineColor},
    position{data.position},
    scale{data.scale},
    origin{data.origin},
    rotation{data.rotation}
{
}


////////////////////////////////////////////////////////////
void Shape::setTextureRect(const Rect2f& rect)
{
    if (m_textureRect == rect)
        return;

    m_textureRect = rect;
    updateTexCoords();
}


////////////////////////////////////////////////////////////
void Shape::setOutlineTextureRect(const Rect2f& rect)
{
    if (m_outlineTextureRect == rect)
        return;

    m_outlineTextureRect = rect;
    updateOutlineTexCoords();
}


////////////////////////////////////////////////////////////
const Rect2f& Shape::getTextureRect() const
{
    return m_textureRect;
}


////////////////////////////////////////////////////////////
const Rect2f& Shape::getOutlineTextureRect() const
{
    return m_outlineTextureRect;
}


////////////////////////////////////////////////////////////
void Shape::setFillColor(Color color)
{
    if (m_fillColor == color)
        return;

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
    if (m_outlineColor == color)
        return;

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

    const base::SizeT pointCount = m_verticesEndIndex - 2u;

    m_vertices.resize(pointCount + 2u); // +2 for center and repeated first point

    updateOutline();
    updateOutlineTexCoords();
}


////////////////////////////////////////////////////////////
void Shape::setMiterLimit(float miterLimit)
{
    m_miterLimit = miterLimit;
    updateOutline();
}


////////////////////////////////////////////////////////////
float Shape::getMiterLimit() const
{
    return m_miterLimit;
}


////////////////////////////////////////////////////////////
float Shape::getOutlineThickness() const
{
    return m_outlineThickness;
}


////////////////////////////////////////////////////////////
const Rect2f& Shape::getLocalBounds() const
{
    return m_bounds;
}


////////////////////////////////////////////////////////////
Rect2f Shape::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
void Shape::update(const sf::Vec2f* points, const base::SizeT pointCount)
{
    updateFromFunc([&points](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { return points[i]; }, pointCount);
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
    if (m_insideBounds.size.x == 0 || m_insideBounds.size.y == 0)
        return;

    const Vertex* end = m_vertices.data() + m_verticesEndIndex;
    for (Vertex* vertex = m_vertices.data(); vertex != end; ++vertex)
    {
        const Vec2f ratio = (vertex->position - m_insideBounds.position).componentWiseDiv(m_insideBounds.size);
        vertex->texCoords = m_textureRect.position + m_textureRect.size.componentWiseMul(ratio);
    }
}


////////////////////////////////////////////////////////////
void Shape::updateOutlineTexCoords()
{
    // Make sure not to divide by zero when the points are aligned on a vertical or horizontal line
    if (m_bounds.size.x == 0 || m_bounds.size.y == 0)
        return;

    const auto* end = m_vertices.data() + m_vertices.size();
    for (Vertex* vertex = m_vertices.data() + m_verticesEndIndex; vertex != end; ++vertex)
    {
        const Vec2f ratio = (vertex->position - m_bounds.position).componentWiseDiv(m_bounds.size);
        vertex->texCoords = m_outlineTextureRect.position + m_outlineTextureRect.size.componentWiseMul(ratio);
    }
}


////////////////////////////////////////////////////////////
void Shape::updateOutline()
{
    // Return if there is no outline or no vertices
    if (m_outlineThickness == 0.f || m_vertices.size() < 2)
    {
        m_verticesEndIndex = m_vertices.size();
        m_bounds           = m_insideBounds;

        return;
    }

    const base::SizeT count = m_vertices.size() - 2u;
    m_vertices.resize(m_verticesEndIndex + (count + 1u) * 2u);

    ShapeUtils::updateOutlineFromTriangleFanFill(m_outlineThickness,
                                                 m_vertices.data() + 1u, // Skip the first vertex (center point)
                                                 m_vertices.data() + m_verticesEndIndex,
                                                 count,
                                                 m_miterLimit);

    // Update outline colors
    updateOutlineColors();

    // Update the shape's bounds from outline vertices
    m_bounds = VertexUtils::getVertexRangeBounds(m_vertices.data() + m_verticesEndIndex,
                                                 m_vertices.size() - m_verticesEndIndex);
}


////////////////////////////////////////////////////////////
void Shape::updateOutlineColors()
{
    const auto* end = m_vertices.data() + m_vertices.size();
    for (Vertex* vertex = m_vertices.data() + m_verticesEndIndex; vertex != end; ++vertex)
        vertex->color = m_outlineColor;
}

} // namespace sf
