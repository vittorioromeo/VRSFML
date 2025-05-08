#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/VertexUtils.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Shape::Shape(const Settings& settings) :
m_textureRect{settings.textureRect},
m_outlineTextureRect{settings.outlineTextureRect},
m_outlineThickness{settings.outlineThickness},
m_fillColor{settings.fillColor},
m_outlineColor{settings.outlineColor},
position{settings.position},
scale{settings.scale},
origin{settings.origin},
rotation{settings.rotation}
{
}


////////////////////////////////////////////////////////////
void Shape::setTextureRect(const FloatRect& rect)
{
    if (m_textureRect == rect)
        return;

    m_textureRect = rect;
    updateTexCoords();
}


////////////////////////////////////////////////////////////
void Shape::setOutlineTextureRect(const FloatRect& rect)
{
    if (m_outlineTextureRect == rect)
        return;

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
    // Return if there is no outline
    if (m_outlineThickness == 0.f)
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
                                                 count);

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
