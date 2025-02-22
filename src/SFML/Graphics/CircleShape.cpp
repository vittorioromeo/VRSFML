#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
CircleShape::CircleShape(const Settings& settings) :
Shape(priv::toShapeSettings(settings)),
m_radius{settings.radius},
m_pointCount{settings.pointCount}
{
    updateCircleGeometry();
}


////////////////////////////////////////////////////////////
void CircleShape::setRadius(float radius)
{
    if (radius == m_radius)
        return;

    m_radius = radius;
    updateCircleGeometry();
}


////////////////////////////////////////////////////////////
float CircleShape::getRadius() const
{
    return m_radius;
}


////////////////////////////////////////////////////////////
void CircleShape::setPointCount(unsigned int pointCount)
{
    if (pointCount == m_pointCount)
        return;

    m_pointCount = pointCount;
    updateCircleGeometry();
}


////////////////////////////////////////////////////////////
unsigned int CircleShape::getPointCount() const
{
    return m_pointCount;
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getPoint(base::SizeT index) const
{
    SFML_BASE_ASSERT(index < m_pointCount && "Index is out of bounds");
    return computeCirclePoint(index, m_pointCount, m_radius);
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getGeometricCenter() const
{
    return {m_radius, m_radius};
}


////////////////////////////////////////////////////////////
void CircleShape::updateCircleGeometry()
{
    if (!Shape::updateImplResizeVerticesVector(m_pointCount)) [[unlikely]]
        return;

    // Position
    const float angleStep = sf::base::tau / static_cast<float>(m_pointCount);

    for (unsigned int i = 0u; i < m_pointCount; ++i)
        m_vertices[i + 1].position = computeCirclePointFromAngleStep(i, angleStep, m_radius);

    Shape::updateImplFromVerticesPositions(m_pointCount, /* mustUpdateBounds */ true);
}

} // namespace sf
