// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/CircleShape.hpp"
#include "Zancle/Graphics/Shape.hpp"
#include "Zancle/Graphics/ShapeUtils.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Constants.hpp"
#include "ZancleBase/SizeT.hpp"


namespace za
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
Vec2f CircleShape::getPoint(zb::SizeT index) const
{
    ZB_ASSERT(index < m_pointCount && "Index is out of bounds");
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
    const float angleStep = zb::tau / static_cast<float>(m_pointCount);

    updateFromFunc([&] [[gnu::always_inline, gnu::flatten]] (const zb::SizeT i) {
        return ShapeUtils::computeCirclePointFromAngleStep(i, /* startAngle */ 0.f, angleStep, m_radius);
    }, m_pointCount);
}

} // namespace za
