// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/RectangleShape.hpp"

#include "Zancle/Graphics/Shape.hpp"
#include "Zancle/Graphics/ShapeUtils.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Base/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
RectangleShape::RectangleShape(const Data& data) : Shape(priv::toShapeData(data))
{
    setSize(data.size);
}


////////////////////////////////////////////////////////////
void RectangleShape::setSize(Vec2f size)
{
    m_size = size;

    const Vec2f points[]{{0.f, 0.f}, {m_size.x, 0.f}, {m_size.x, m_size.y}, {0.f, m_size.y}};
    update(points, /* pointCount */ 4u);
}


////////////////////////////////////////////////////////////
Vec2f RectangleShape::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
Vec2f RectangleShape::getPoint(za::SizeT index) const
{
    ZA_ASSERT(index < 4u && "Index is out of bounds");
    return ShapeUtils::computeRectanglePoint(index, m_size);
}


////////////////////////////////////////////////////////////
Vec2f RectangleShape::getGeometricCenter() const
{
    return m_size / 2.f;
}

} // namespace za
