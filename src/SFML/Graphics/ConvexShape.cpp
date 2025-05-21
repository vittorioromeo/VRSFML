#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/ConvexShape.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(const Data& data) : Shape(priv::toShapeData(data))
{
    setPointCount(data.pointCount);
}


////////////////////////////////////////////////////////////
void ConvexShape::setPointCount(const base::SizeT count)
{
    m_points.resize(count);
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
base::SizeT ConvexShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
void ConvexShape::setPoint(const base::SizeT index, const Vec2f point)
{
    SFML_BASE_ASSERT(index < m_points.size() && "Index is out of bounds");
    m_points[index] = point;
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
Vec2f ConvexShape::getPoint(const base::SizeT index) const
{
    SFML_BASE_ASSERT(index < m_points.size() && "Index is out of bounds");
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vec2f ConvexShape::getGeometricCenter() const
{
    return ShapeUtils::computeConvexShapeGeometricCenter(m_points.data(), m_points.size());
}

} // namespace sf
