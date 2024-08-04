#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/CircleShape.hpp>

#include <SFML/System/Angle.hpp>

#include <vector>


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
struct CircleShape::Impl
{
    float                 radius; //!< Radius of the circle
    std::vector<Vector2f> points; //!< Points composing the circle
};


////////////////////////////////////////////////////////////
CircleShape::CircleShape(float radius, std::size_t pointCount) : m_impl(radius)
{
    update(radius, pointCount);
}


////////////////////////////////////////////////////////////
CircleShape::~CircleShape() = default;


////////////////////////////////////////////////////////////
CircleShape::CircleShape(const CircleShape& rhs) = default;


////////////////////////////////////////////////////////////
CircleShape& CircleShape::operator=(const CircleShape&) = default;


////////////////////////////////////////////////////////////
CircleShape::CircleShape(CircleShape&&) noexcept = default;


////////////////////////////////////////////////////////////
CircleShape& CircleShape::operator=(CircleShape&&) noexcept = default;


////////////////////////////////////////////////////////////
void CircleShape::setRadius(float radius)
{
    m_impl->radius = radius;
    update(radius, m_impl->points.size());
}


////////////////////////////////////////////////////////////
float CircleShape::getRadius() const
{
    return m_impl->radius;
}


////////////////////////////////////////////////////////////
void CircleShape::setPointCount(std::size_t count)
{
    update(m_impl->radius, count);
}

////////////////////////////////////////////////////////////
std::size_t CircleShape::getPointCount() const
{
    return m_impl->points.size();
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getPoint(std::size_t index) const
{
    return m_impl->points[index];
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getGeometricCenter() const
{
    return {m_impl->radius, m_impl->radius};
}


////////////////////////////////////////////////////////////
void CircleShape::update(float radius, std::size_t pointCount)
{
    m_impl->points.resize(pointCount);

    for (std::size_t i = 0; i < pointCount; ++i)
        m_impl->points[i] = computeCirclePoint(i, pointCount, radius);

    Shape::update(m_impl->points.data(), m_impl->points.size());
}

} // namespace sf
