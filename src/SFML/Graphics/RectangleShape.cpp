#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RectangleShape.hpp"

#include "SFML/System/Vector2.hpp"

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

    const Vector2f points[]{{0.f, 0.f}, {m_size.x, 0.f}, {m_size.x, m_size.y}, {0.f, m_size.y}};
    update(points, /* pointCount */ 4u);
}


////////////////////////////////////////////////////////////
Vector2f RectangleShape::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
Vector2f RectangleShape::getPoint(base::SizeT index) const
{
    SFML_BASE_ASSERT(index < 4u && "Index is out of bounds");

    const Vector2f points[]{{0.f, 0.f}, {m_size.x, 0.f}, {m_size.x, m_size.y}, {0.f, m_size.y}};
    return points[index];
}


////////////////////////////////////////////////////////////
Vector2f RectangleShape::getGeometricCenter() const
{
    return m_size / 2.f;
}

} // namespace sf
