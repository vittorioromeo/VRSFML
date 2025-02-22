#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/ConvexShape.hpp"
#include "SFML/Graphics/Shape.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/MinMax.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(const Settings& settings) : Shape(priv::toShapeSettings(settings))
{
    setPointCount(settings.pointCount);
}


////////////////////////////////////////////////////////////
void ConvexShape::setPointCount(base::SizeT count)
{
    m_points.resize(count);
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
base::SizeT ConvexShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
void ConvexShape::setPoint(base::SizeT index, Vector2f point)
{
    SFML_BASE_ASSERT(index < m_points.size() && "Index is out of bounds");
    m_points[index] = point;
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
Vector2f ConvexShape::getPoint(base::SizeT index) const
{
    SFML_BASE_ASSERT(index < m_points.size() && "Index is out of bounds");
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vector2f ConvexShape::getGeometricCenter() const
{
    const auto pointCount = m_points.size();

    if (pointCount == 0)
    {
        SFML_BASE_ASSERT(false && "Cannot calculate geometric center of shape with no points");
        return Vector2f{};
    }

    if (pointCount == 1)
        return m_points[0];

    if (pointCount == 2)
        return (m_points[0] + m_points[1]) / 2.f;

    Vector2f centroid{};
    float    twiceArea = 0;

    auto previousPoint = m_points[pointCount - 1];
    for (base::SizeT i = 0; i < pointCount; ++i)
    {
        const auto  currentPoint = m_points[i];
        const float product      = previousPoint.cross(currentPoint);
        twiceArea += product;
        centroid += (currentPoint + previousPoint) * product;

        previousPoint = currentPoint;
    }

    if (twiceArea != 0.f)
        return centroid / 3.f / twiceArea;

    // Fallback for no area - find the center of the bounding box
    Vector2f minPoint = m_points[0];
    Vector2f maxPoint = minPoint;

    for (base::SizeT i = 1; i < pointCount; ++i)
    {
        const auto currentPoint = m_points[i];

        minPoint.x = base::min(minPoint.x, currentPoint.x);
        maxPoint.x = base::max(maxPoint.x, currentPoint.x);
        minPoint.y = base::min(minPoint.y, currentPoint.y);
        maxPoint.y = base::max(maxPoint.y, currentPoint.y);
    }

    return (maxPoint + minPoint) / 2.f;
}

} // namespace sf
