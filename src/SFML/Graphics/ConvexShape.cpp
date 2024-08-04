#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/ConvexShape.hpp>

#include <SFML/Base/Assert.hpp>

#include <vector>


namespace sf
{
////////////////////////////////////////////////////////////
struct ConvexShape::Impl
{
    std::vector<Vector2f> points; //!< Points composing the convex polygon
};


////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(std::size_t pointCount)
{
    setPointCount(pointCount);
}


////////////////////////////////////////////////
ConvexShape::~ConvexShape() = default;


////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(const ConvexShape& rhs) = default;


////////////////////////////////////////////////////////////
ConvexShape& ConvexShape::operator=(const ConvexShape&) = default;


////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(ConvexShape&&) noexcept = default;


////////////////////////////////////////////////////////////
ConvexShape& ConvexShape::operator=(ConvexShape&&) noexcept = default;


////////////////////////////////////////////////////////////
void ConvexShape::setPointCount(std::size_t count)
{
    m_impl->points.resize(count);
    update(m_impl->points.data(), m_impl->points.size());
}


////////////////////////////////////////////////////////////
std::size_t ConvexShape::getPointCount() const
{
    return m_impl->points.size();
}


////////////////////////////////////////////////////////////
void ConvexShape::setPoint(std::size_t index, Vector2f point)
{
    SFML_BASE_ASSERT(index < m_impl->points.size() && "Index is out of bounds");
    m_impl->points[index] = point;
    update(m_impl->points.data(), m_impl->points.size());
}


////////////////////////////////////////////////////////////
Vector2f ConvexShape::getPoint(std::size_t index) const
{
    SFML_BASE_ASSERT(index < m_impl->points.size() && "Index is out of bounds");
    return m_impl->points[index];
}


////////////////////////////////////////////////////////////
Vector2f ConvexShape::getGeometricCenter() const
{
    const auto pointCount = m_impl->points.size();

    switch (pointCount)
    {
        case 0:
            SFML_BASE_ASSERT(false && "Cannot calculate geometric center of shape with no points");
            return Vector2f{};
        case 1:
            return m_impl->points[0];
        case 2:
            return (m_impl->points[0] + m_impl->points[1]) / 2.f;
        default: // more than two points
            Vector2f centroid;
            float    twiceArea = 0;

            auto previousPoint = m_impl->points[pointCount - 1];
            for (std::size_t i = 0; i < pointCount; ++i)
            {
                const auto  currentPoint = m_impl->points[i];
                const float product      = previousPoint.cross(currentPoint);
                twiceArea += product;
                centroid += (currentPoint + previousPoint) * product;

                previousPoint = currentPoint;
            }

            if (twiceArea != 0.f)
            {
                return centroid / 3.f / twiceArea;
            }

            // Fallback for no area - find the center of the bounding box
            auto minPoint = m_impl->points[0];
            auto maxPoint = minPoint;
            for (std::size_t i = 1; i < pointCount; ++i)
            {
                const auto currentPoint = m_impl->points[i];

                minPoint.x = std::min(minPoint.x, currentPoint.x);
                maxPoint.x = std::max(maxPoint.x, currentPoint.x);
                minPoint.y = std::min(minPoint.y, currentPoint.y);
                maxPoint.y = std::max(maxPoint.y, currentPoint.y);
            }
            return (maxPoint + minPoint) / 2.f;
    }
}

} // namespace sf
