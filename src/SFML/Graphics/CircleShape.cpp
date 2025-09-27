// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/LambdaMacros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
CircleShape::CircleShape(const Data& data) :
    Shape(priv::toShapeData(data)),
    m_radius{data.radius},
    m_pointCount{data.pointCount}
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
Vec2f CircleShape::getPoint(base::SizeT index) const
{
    SFML_BASE_ASSERT(index < m_pointCount && "Index is out of bounds");
    return ShapeUtils::computeCirclePoint(index, /* startAngle */ 0.f, m_pointCount, m_radius);
}


////////////////////////////////////////////////////////////
Vec2f CircleShape::getGeometricCenter() const
{
    return {m_radius, m_radius};
}


////////////////////////////////////////////////////////////
void CircleShape::updateCircleGeometry()
{
    const float angleStep = sf::base::tau / static_cast<float>(m_pointCount);

    updateFromFunc([&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return ShapeUtils::computeCirclePointFromAngleStep(i, /* startAngle */ 0.f, angleStep, m_radius); },
                   m_pointCount);
}

} // namespace sf
