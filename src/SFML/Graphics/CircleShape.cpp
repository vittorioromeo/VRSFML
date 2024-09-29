#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShape.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FastSinCos.hpp"
#include "SFML/Base/TrivialVector.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr sf::Vector2f computeCirclePoint(
    const sf::base::SizeT index,
    const unsigned int    pointCount,
    const float           radius)
{
    const float radians = static_cast<float>(index) / static_cast<float>(pointCount) * sf::base::tau - sf::base::halfPi;
    const auto [sine, cosine] = sf::base::fastSinCos(radians);
    return {radius * (1.f + sine), radius * (1.f + cosine)};
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
CircleShape::CircleShape(const Settings& settings) :
Shape(priv::toShapeSettings(settings)),
m_radius{settings.radius},
m_pointCount{settings.pointCount}
{
    updateCircleGeometry();
}


////////////////////////////////////////////////////////////
void CircleShape::setRadius(float radius)
{
    if (radius == m_radius)
        return;

    m_radius = radius;
    updateCircleGeometry();
}


////////////////////////////////////////////////////////////
float CircleShape::getRadius() const
{
    return m_radius;
}


////////////////////////////////////////////////////////////
void CircleShape::setPointCount(unsigned int pointCount)
{
    if (pointCount == m_pointCount)
        return;

    m_pointCount = pointCount;
    updateCircleGeometry();
}


////////////////////////////////////////////////////////////
unsigned int CircleShape::getPointCount() const
{
    return m_pointCount;
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getPoint(base::SizeT index) const
{
    SFML_BASE_ASSERT(index < m_pointCount && "Index is out of bounds");
    return computeCirclePoint(index, m_pointCount, m_radius);
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getGeometricCenter() const
{
    return {m_radius, m_radius};
}


////////////////////////////////////////////////////////////
void CircleShape::updateCircleGeometry()
{
    thread_local base::TrivialVector<Vector2f> points;
    points.resize(m_pointCount);

    for (unsigned int i = 0u; i < m_pointCount; ++i)
        points[i] = computeCirclePoint(i, m_pointCount, m_radius);

    Shape::update(points.data(), points.size());
}

} // namespace sf
