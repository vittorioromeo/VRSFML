#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RectangleShape.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
RectangleShape::RectangleShape(const Shape::Settings& settings, Vector2f size) : Shape(settings)
{
    setSize(size);
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
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vector2f RectangleShape::getGeometricCenter() const
{
    return m_size / 2.f;
}

} // namespace sf
