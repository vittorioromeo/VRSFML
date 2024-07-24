#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/CircleShape.hpp>

#include <SFML/System/Angle.hpp>


namespace
{
////////////////////////////////////////////////////////////
sf::Vector2f computeCirclePoint(std::size_t index, std::size_t pointCount, float radius)
{
    const sf::Angle angle = static_cast<float>(index) / static_cast<float>(pointCount) * sf::degrees(360.f) -
                            sf::degrees(90.f);

    return sf::Vector2f{radius, radius}.movedTowards(radius, angle);
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
CircleShape::CircleShape(float radius, std::size_t pointCount) : m_radius(radius)
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
void CircleShape::setPointCount(std::size_t count)
{
    update(m_radius, count);
}

////////////////////////////////////////////////////////////
std::size_t CircleShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getPoint(std::size_t index) const
{
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getGeometricCenter() const
{
    return {m_radius, m_radius};
}


////////////////////////////////////////////////////////////
void CircleShape::update(float radius, std::size_t pointCount)
{
    m_points.resize(pointCount);

    for (std::size_t i = 0; i < pointCount; ++i)
        m_points[i] = computeCirclePoint(i, pointCount, radius);

    Shape::update(m_points.data(), m_points.size());
}

} // namespace sf
