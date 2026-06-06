// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/ConvexShape.hpp"

#include "Zancle/Graphics/Shape.hpp"
#include "Zancle/Graphics/ShapeUtils.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Diagnostic/Assert.hpp"

#include "Zancle/Base/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(const Data& data) : Shape(priv::toShapeData(data))
{
    setPointCount(data.pointCount);
}


////////////////////////////////////////////////////////////
void ConvexShape::setPointCount(const za::SizeT count)
{
    m_points.resize(count);
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
za::SizeT ConvexShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
void ConvexShape::setPoint(const za::SizeT index, const Vec2f point)
{
    ZA_ASSERT(index < m_points.size() && "Index is out of bounds");
    m_points[index] = point;
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
Vec2f ConvexShape::getPoint(const za::SizeT index) const
{
    ZA_ASSERT(index < m_points.size() && "Index is out of bounds");
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vec2f ConvexShape::getGeometricCenter() const
{
    return ShapeUtils::computeConvexShapeGeometricCenter(m_points.data(), m_points.size());
}

} // namespace za
