#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Builtins/Assume.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FastSinCos.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Computes the angular step between vertices.
///
/// Given the number of points for a circle or ellipse, this function
/// calculates the angle (in radians) between consecutive vertices.
///
/// \param pointCount Number of points/vertices.
///
/// \return The angular step in radians.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float computeAngleStep(
    const unsigned int pointCount) noexcept
{
    return base::tau / static_cast<float>(pointCount);
}

////////////////////////////////////////////////////////////
/// \brief Computes a point on an ellipse using a precomputed angle step.
///
/// This function calculates the 2D position of a vertex on an ellipse
/// given its horizontal and vertical radii. It uses a precomputed angular
/// step multiplied by the vertex index.
///
/// \param index The index of the vertex.
/// \param angleStep The angular increment between vertices (in radians).
/// \param hRadius The horizontal radius of the ellipse.
/// \param vRadius The vertical radius of the ellipse.
///
/// \return The computed 2D position.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeEllipsePointFromAngleStep(
    const base::SizeT index,
    const float       angleStep,
    const float       hRadius,
    const float       vRadius) noexcept
{
    const auto [sine, cosine] = base::fastSinCos(static_cast<float>(index) * angleStep);

    SFML_BASE_ASSUME(sine >= -1.f && sine <= 1.f);
    SFML_BASE_ASSUME(cosine >= -1.f && cosine <= 1.f);

    return {hRadius * (1.f + sine), vRadius * (1.f + cosine)};
}

////////////////////////////////////////////////////////////
/// \brief Computes a point on an ellipse.
///
/// Given the vertex index and total number of points defining the ellipse,
/// this function computes the vertex position by first calculating the angle step.
///
/// \param index The index of the vertex.
/// \param pointCount The total number of vertices.
/// \param hRadius The horizontal radius of the ellipse.
/// \param vRadius The vertical radius of the ellipse.
///
/// \return The computed 2D position.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeEllipsePoint(
    const base::SizeT  index,
    const unsigned int pointCount,
    const float        hRadius,
    const float        vRadius) noexcept
{
    return computeEllipsePointFromAngleStep(index, computeAngleStep(pointCount), hRadius, vRadius);
}

////////////////////////////////////////////////////////////
/// \brief Computes a point on a circle using a precomputed angle step.
///
/// This function calculates the position of a vertex on a circle, reusing the
/// ellipse computation by passing the same value for both radii.
///
/// \param index The index of the vertex.
/// \param angleStep The angular increment between vertices (in radians).
/// \param radius The radius of the circle.
///
/// \return The computed 2D position.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeCirclePointFromAngleStep(
    const base::SizeT index,
    const float       angleStep,
    const float       radius) noexcept
{
    return computeEllipsePointFromAngleStep(index, angleStep, radius, radius);
}

////////////////////////////////////////////////////////////
/// \brief Computes a point on a circle.
///
/// Given the vertex index and total number of vertices for a circle,
/// this function computes the position of the vertex.
///
/// \param index The index of the vertex.
/// \param pointCount The total number of vertices.
/// \param radius The radius of the circle.
///
/// \return The computed 2D position.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeCirclePoint(
    const base::SizeT  index,
    const unsigned int pointCount,
    const float        radius) noexcept
{
    return computeCirclePointFromAngleStep(index, computeAngleStep(pointCount), radius);
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeRectanglePoint(
    const base::SizeT index,
    const Vector2f    size) noexcept
{
    SFML_BASE_ASSERT(index <= 4u);
    SFML_BASE_ASSUME(index <= 4u);

    const Vector2f points[]{{0.f, 0.f}, {size.x, 0.f}, {size.x, size.y}, {0.f, size.y}};
    return points[index];
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeRoundedRectanglePoint(
    const base::SizeT  index,
    const Vector2f     size,
    const float        cornerRadius,
    const unsigned int cornerPointCount) noexcept
{
    const auto totalNumPoints = cornerPointCount * 4u;

    SFML_BASE_ASSERT(index < totalNumPoints);
    SFML_BASE_ASSUME(index < totalNumPoints);

    const base::SizeT centerIndex = index / cornerPointCount;
    SFML_BASE_ASSERT(centerIndex >= 0u && centerIndex <= 3u);
    SFML_BASE_ASSUME(centerIndex >= 0u && centerIndex <= 3u);

    const float deltaAngle = (base::halfPi) / static_cast<float>(cornerPointCount - 1u);

    const Vector2f center{(centerIndex == 0 || centerIndex == 3) ? size.x - cornerRadius : cornerRadius,
                          (centerIndex < 2) ? cornerRadius : size.y - cornerRadius};

    const auto [sine, cosine] = base::fastSinCos(deltaAngle * static_cast<float>(index - centerIndex));
    return center + sf::Vector2f{cornerRadius * cosine, -cornerRadius * sine};
}

////////////////////////////////////////////////////////////
/// \brief Computes the normalized perpendicular of a segment.
///
/// Given two points defining a line segment, this function calculates the unit vector
/// perpendicular to that segment.
///
/// \param p1 The starting point of the segment.
/// \param p2 The ending point of the segment.
///
/// \return The normal vector (normalized).
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline Vector2f computeSegmentNormal(const Vector2f p1, const Vector2f p2) noexcept
{
    // Compute the difference and its perpendicular.
    const Vector2f diff   = p2 - p1;
    const Vector2f normal = {-diff.y, diff.x};

    // Compute squared length of the normal.
    const float lenSq = normal.x * normal.x + normal.y * normal.y;

    // By computing the squared length (lenSq), we can avoid an extra temporary if the squared length is zero.
    // Only if nonzero do we compute the square root and the reciprocal.

    // Avoid division by zero.
    if (lenSq == 0.f)
        return normal;

    const float invLen = 1.f / base::sqrt(lenSq);
    return {normal.x * invLen, normal.y * invLen};
}

////////////////////////////////////////////////////////////
/// \brief Get bounds of a vertex range
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline constexpr FloatRect getVertexRangeBounds(const Vertex* data, const base::SizeT nVertices) noexcept
{
    if (nVertices == 0u)
        return {};

    float left   = data[0].position.x;
    float top    = data[0].position.y;
    float right  = data[0].position.x;
    float bottom = data[0].position.y;

    const Vertex* end = data + nVertices;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        left   = base::min(left, v->position.x);
        right  = base::max(right, v->position.x);
        top    = base::min(top, v->position.y);
        bottom = base::max(bottom, v->position.y);
    }

    return {{left, top}, {right - left, bottom - top}};
}

////////////////////////////////////////////////////////////
/// \brief Computes the bounding rectangle of a range of vertices.
///
/// This function iterates through a sequence of vertices and determines the
/// smallest axis-aligned rectangle that encloses all the vertices.
///
/// \param data Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return A rect representing the bounding rectangle.
///
////////////////////////////////////////////////////////////
inline constexpr void updateOutlineImpl(const float       outlineThickness,
                                        const base::SizeT verticesEndIndex,
                                        Vertex*           vertices,
                                        const base::SizeT count)
{
    // Cache the center (`vertices[0]` is the center of the shape)
    const Vector2f    center      = vertices[0].position;
    const base::SizeT outlineBase = verticesEndIndex;

    // Lambda that computes and writes the outline for a single segment
    // - `i` is the logical index (used to compute the output position)
    // - `p0`, `p1`, `p2` are the three consecutive positions used to compute the normals
    const auto updateSegment = [&](const base::SizeT i, const Vector2f p0, const Vector2f p1, const Vector2f p2)
                                   __attribute__((always_inline, flatten))
    {
        Vector2f n1 = computeSegmentNormal(p0, p1);
        Vector2f n2 = computeSegmentNormal(p1, p2);

        const Vector2f diff = center - p1;

        if (n1.dot(diff) > 0.f)
            n1 = -n1;

        if (n2.dot(diff) > 0.f)
            n2 = -n2;

        const float factor = 1.f + (n1.x * n2.x + n1.y * n2.y);

        const Vector2f    normal = (n1 + n2) / factor;
        const base::SizeT outIdx = outlineBase + (i << 1u); // Shift is equivalent to `i * 2`

        vertices[outIdx + 0u].position = p1;
        vertices[outIdx + 1u].position = p1 + normal * outlineThickness;
    };

    // Handle the first segment (wrap-around):
    // `p0` is from the last vertex (wrap-around), `p1` is the first point (after center),
    // and `p2` is the second point.
    updateSegment(0u, vertices[count].position, vertices[1].position, vertices[2].position);

    // Process the remaining segments.
    for (base::SizeT i = 1; i < count; ++i)
    {
        // For `i >= 1`, the three points are consecutive:
        // `p0` is `vertices[i]`, `p1` is `vertices[i + 1]`, `p2` is `vertices[i + 2]`.
        updateSegment(i, vertices[i].position, vertices[i + 1].position, vertices[i + 2].position);
    }

    // Duplicate the first outline vertex at the end to close the outline loop.
    const base::SizeT dupIndex = outlineBase + (count << 1u); // Shift is equivalent to `i * 2`

    vertices[dupIndex + 0].position = vertices[outlineBase + 0].position;
    vertices[dupIndex + 1].position = vertices[outlineBase + 1].position;
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::ShapeUtils
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
