#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf
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
Vec2f RectangleShape::getPoint(base::SizeT index) const
{
    SFML_BASE_ASSERT(index < 4u && "Index is out of bounds");
    return ShapeUtils::computeRectanglePoint(index, m_size);
}


////////////////////////////////////////////////////////////
Vec2f RectangleShape::getGeometricCenter() const
{
    return m_size / 2.f;
}

} // namespace sf
