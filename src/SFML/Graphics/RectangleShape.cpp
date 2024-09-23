#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RectangleShape.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
RectangleShape::RectangleShape(const Settings& settings) : Shape(priv::toShapeSettings(settings))
{
    setSize(settings.size);
}


////////////////////////////////////////////////////////////
void RectangleShape::setSize(Vector2f size)
{
    m_size = size;

    m_points[0] = {0, 0};
    m_points[1] = {m_size.x, 0};
    m_points[2] = {m_size.x, m_size.y};
    m_points[3] = {0, m_size.y};

    update(m_points, /* pointCount */ 4);
}


////////////////////////////////////////////////////////////
Vector2f RectangleShape::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
base::SizeT RectangleShape::getPointCount() const
{
    return 4;
}


////////////////////////////////////////////////////////////
Vector2f RectangleShape::getPoint(base::SizeT index) const
{
    SFML_BASE_ASSERT(index < 4u && "Index is out of bounds");
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vector2f RectangleShape::getGeometricCenter() const
{
    return m_size / 2.f;
}

} // namespace sf
