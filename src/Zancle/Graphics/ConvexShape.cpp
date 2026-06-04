// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/ConvexShape.hpp"

#include "Zancle/Graphics/Shape.hpp"
#include "Zancle/Graphics/ShapeUtils.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(const Data& data) : Shape(priv::toShapeData(data))
{
    setPointCount(data.pointCount);
}


////////////////////////////////////////////////////////////
void ConvexShape::setPointCount(const zb::SizeT count)
{
    m_points.resize(count);
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
zb::SizeT ConvexShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
void ConvexShape::setPoint(const zb::SizeT index, const Vec2f point)
{
    ZB_ASSERT(index < m_points.size() && "Index is out of bounds");
    m_points[index] = point;
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
Vec2f ConvexShape::getPoint(const zb::SizeT index) const
{
    ZB_ASSERT(index < m_points.size() && "Index is out of bounds");
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vec2f ConvexShape::getGeometricCenter() const
{
    return ShapeUtils::computeConvexShapeGeometricCenter(m_points.data(), m_points.size());
}

} // namespace za
