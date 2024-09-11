#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShape.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Vector2.hpp"


namespace
{
////////////////////////////////////////////////////////////
sf::Vector2f computeCirclePoint(sf::base::SizeT index, sf::base::SizeT pointCount, float radius)
{
    const sf::Angle angle = static_cast<float>(index) / static_cast<float>(pointCount) * sf::degrees(360.f) -
                            sf::degrees(90.f);

    return sf::Vector2f{radius, radius}.movedTowards(radius, angle);
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
CircleShape::CircleShape(float radius, base::SizeT pointCount) : m_radius{radius}
{
    update(radius, pointCount);
}


////////////////////////////////////////////////////////////
void CircleShape::setRadius(float radius)
{
    m_radius = radius;
    update(radius, m_points.size());
}


////////////////////////////////////////////////////////////
float CircleShape::getRadius() const
{
    return m_radius;
}


////////////////////////////////////////////////////////////
void CircleShape::setPointCount(base::SizeT count)
{
    update(m_radius, count);
}

////////////////////////////////////////////////////////////
base::SizeT CircleShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getPoint(base::SizeT index) const
{
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getGeometricCenter() const
{
    return {m_radius, m_radius};
}


////////////////////////////////////////////////////////////
void CircleShape::update(float radius, base::SizeT pointCount)
{
    m_points.resize(pointCount);

    for (base::SizeT i = 0; i < pointCount; ++i)
        m_points[i] = computeCirclePoint(i, pointCount, radius);

    Shape::update(m_points.data(), m_points.size());
}

} // namespace sf
