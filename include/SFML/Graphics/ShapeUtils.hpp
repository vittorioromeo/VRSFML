#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtins/Restrict.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FastSinCos.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::ShapeUtils
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

    SFML_BASE_ASSERT_AND_ASSUME(sine >= -1.f && sine <= 1.f);
    SFML_BASE_ASSERT_AND_ASSUME(cosine >= -1.f && cosine <= 1.f);

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
    SFML_BASE_ASSERT_AND_ASSUME(index <= 4u);

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
    [[maybe_unused]] const auto totalNumPoints = cornerPointCount * 4u;
    SFML_BASE_ASSERT_AND_ASSUME(index < totalNumPoints);

    const base::SizeT centerIndex = index / cornerPointCount;
    SFML_BASE_ASSERT_AND_ASSUME(centerIndex <= 3u);

    const float deltaAngle = (base::halfPi) / static_cast<float>(cornerPointCount - 1u);

    const Vector2f center{(centerIndex == 0 || centerIndex == 3) ? size.x - cornerRadius : cornerRadius,
                          (centerIndex < 2) ? cornerRadius : size.y - cornerRadius};

    const auto [sine, cosine] = base::fastSinCos(deltaAngle * static_cast<float>(index - centerIndex));
    return center + Vector2f{cornerRadius * cosine, -cornerRadius * sine};
}

////////////////////////////////////////////////////////////
/// \brief Computes a vertex point for an arrow shape's boundary.
///
/// Defines an arrow pointing along the positive X-axis, with its
/// tail base centered at the local origin `(0,0)`. The arrow boundary
/// is defined by 7 distinct vertices in counter-clockwise order:
///
/// 0: Bottom-left tail corner
/// 1: Bottom-right shaft corner (at shaft end)
/// 2: Bottom barb corner (inner corner of head)
/// 3: Tip of the arrowhead
/// 4: Top barb corner (inner corner of head)
/// 5: Top-right shaft corner (at shaft end)
/// 6: Top-left tail corner
///
/// \param index       The index of the vertex to compute `(0 <= index < 7)`.
/// \param shaftLength Length of the arrow's shaft section.
/// \param shaftWidth  Full width of the arrow's shaft.
/// \param headLength  Length of the arrowhead section (from shaft end to tip).
/// \param headWidth   Full width of the arrowhead at its base (barbs).
///
/// \return The computed 2D position of the boundary vertex.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeArrowPoint(
    const base::SizeT index,
    const float       shaftLength,
    const float       shaftWidth,
    const float       headLength,
    const float       headWidth) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(index < 7u);
    SFML_BASE_ASSERT_AND_ASSUME(shaftLength >= 0.f);
    SFML_BASE_ASSERT_AND_ASSUME(shaftWidth >= 0.f);
    SFML_BASE_ASSERT_AND_ASSUME(headLength >= 0.f);
    SFML_BASE_ASSERT_AND_ASSUME(headWidth >= 0.f);

    const float halfShaftW = shaftWidth / 2.f;
    const float halfHeadW  = headWidth / 2.f;
    const float tipX       = shaftLength + headLength;

    const Vector2f points[7u] = {
        {0.f, -halfShaftW},         // 0: Bottom-left tail
        {shaftLength, -halfShaftW}, // 1: Bottom-right shaft
        {shaftLength, -halfHeadW},  // 2: Bottom barb
        {tipX, 0.f},                // 3: Tip
        {shaftLength, halfHeadW},   // 4: Top barb
        {shaftLength, halfShaftW},  // 5: Top-right shaft
        {0.f, halfShaftW}           // 6: Top-left tail
    };

    return points[index];
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float computePieSliceArcAngleStep(
    const float        sweepAngle,
    const unsigned int pointCount) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(pointCount >= 3u);
    return sweepAngle / static_cast<float>(pointCount - 2u);
}

////////////////////////////////////////////////////////////
/// \see computePieSlicePoint
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computePieSlicePointFromArcAngleStep(
    const base::SizeT  index,
    const float        radius,
    const float        arcAngleStep,
    const float        startAngle,
    const float        sweepAngle,
    const unsigned int pointCount) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(pointCount >= 3u);  // Need center + at least start/end points on arc
    SFML_BASE_ASSERT_AND_ASSUME(sweepAngle != 0.f); // Sweep angle must be non-zero
    SFML_BASE_ASSERT_AND_ASSUME(index < pointCount);
    SFML_BASE_ASSERT_AND_ASSUME(radius >= 0.f); // Radius should be non-negative

    // Vertex `0` is the center of the arc/circle, acting as the hub for the triangle fan.

    if (index == 0u)
        return {radius, radius};

    // Vertices `1` to `pointCount - 1` are on the arc's curved edge.
    // There are `(pointCount - 1)` points on the arc edge.
    // These points span `(pointCount - 2)` segments along the arc.

    const auto arcPointStep   = static_cast<float>(index - 1u);
    const auto [sine, cosine] = base::fastSinCos(startAngle + arcPointStep * arcAngleStep);

    SFML_BASE_ASSERT_AND_ASSUME(sine >= -1.f && sine <= 1.f);
    SFML_BASE_ASSERT_AND_ASSUME(cosine >= -1.f && cosine <= 1.f);

    return {radius * (1.f + sine), radius * (1.f + cosine)};
}

////////////////////////////////////////////////////////////
/// \brief Computes a vertex point for a filled convex arc (pie slice).
///
/// A filled arc is defined by a center point, a radius, a starting
/// angle, and an angular extent (sweep angle). The shape consists
/// of the center point (vertex `0`) and a series of points along the
/// circular arc segment (vertices `1` to `pointCount - 1`), suitable for
/// rendering as a triangle fan originating from the center.
///
/// The arc is generated using the same coordinate system and angle
/// convention as `computeCirclePoint`: the implicit center of the circle
/// from which the arc is derived is at `(radius, radius)`. An angle of
/// `0` radians corresponds to the top point `(radius, 2 * radius)`,
/// `pi / 2` radians to the right point `(2 * radius, radius)`, `pi`
/// radians to the bottom point `(radius, 0)`, and `3 * pi / 2` radians
/// to the left point `(0, radius)`.
///
/// The resulting shape must be convex, which requires the absolute value
/// of `sweepAngle` to be greater than `0` and less than or equal to `pi` radians
/// (`180` degrees).
///
/// Vertex `0` is the center point, located at `(radius, radius)`.
/// Vertices `1` to `pointCount - 1` trace the curved edge of the arc.
///
/// \param index      The index of the vertex to compute `(0 <= index < pointCount)`.
/// \param radius     The radius of the arc.
/// \param startAngle The starting angle of the arc in radians (using the described convention).
/// \param sweepAngle The angular extent of the arc in radians (must be > `0` and <= `pi`).
///                   Positive sweeps counter-clockwise, negative sweeps clockwise.
/// \param pointCount Total number of vertices for the shape (must be >= `3`).
///                   Includes the center point and (`pointCount - 1`) points on the arc.
///
/// \return The computed 2D position of the vertex.
///
/// \see computeCirclePoint
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computePieSlicePoint(
    const base::SizeT  index,
    const float        radius,
    const float        startAngle,
    const float        sweepAngle,
    const unsigned int pointCount) noexcept
{
    return computePieSlicePointFromArcAngleStep(index,
                                                radius,
                                                computePieSliceArcAngleStep(sweepAngle, pointCount),
                                                startAngle,
                                                sweepAngle,
                                                pointCount);
}

////////////////////////////////////////////////////////////
/// \see computeStarPoint
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeStarPointFromAngleStep(
    const base::SizeT index,
    const float       angleStep,
    const float       outerRadius,
    const float       innerRadius) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(outerRadius >= 0.f);
    SFML_BASE_ASSERT_AND_ASSUME(innerRadius >= 0.f);

    float angle = static_cast<float>(index) * angleStep - base::halfPi; // Start from top (-pi/2)

    if (angle < 0.f)
        angle += base::tau;

    const float radius = (index % 2u == 0u) ? outerRadius : innerRadius;

    const auto [sine, cosine] = base::fastSinCos(angle);

    SFML_BASE_ASSERT_AND_ASSUME(sine >= -1.f && sine <= 1.f);
    SFML_BASE_ASSERT_AND_ASSUME(cosine >= -1.f && cosine <= 1.f);

    // Use outerRadius for centering consistently, like circle/ellipse
    return {outerRadius + radius * cosine, outerRadius + radius * sine};
}

////////////////////////////////////////////////////////////
/// \brief Computes a vertex point for a star shape.
///
/// A star is defined by alternating outer and inner points around a center.
/// Vertices are generated by alternating between outerRadius and innerRadius
/// at regular angular steps. The total number of vertices generated is
/// `2 * pointCount`. Vertex indices `0, 2, 4, ...` correspond to outer points,
/// and indices `1, 3, 5, ...` correspond to inner points.
///
/// Assumes the center for angle calculations is `(outerRadius, outerRadius)`.
///
/// \param index        The index of the vertex to compute `(0 <= index < 2 * pointCount)`.
/// \param pointCount   The number of points the star has (e.g., 5 for a 5-pointed star, must be >= 2).
/// \param outerRadius  Distance from the center to the outer points.
/// \param innerRadius  Distance from the center to the inner points.
///
/// \return The computed 2D position of the vertex.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vector2f computeStarPoint(
    const base::SizeT  index,
    const unsigned int pointCount,
    const float        outerRadius,
    const float        innerRadius) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(pointCount >= 2u);
    SFML_BASE_ASSERT_AND_ASSUME(index < 2u * pointCount);

    const float angleStep = base::tau / static_cast<float>(2u * pointCount);

    return computeStarPointFromAngleStep(index, angleStep, outerRadius, innerRadius);
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

    const float invLen = 1.f / SFML_BASE_MATH_SQRTF(lenSq);
    return {normal.x * invLen, normal.y * invLen};
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
inline constexpr void updateOutlineImpl(const float                            outlineThickness,
                                        const Vertex* const SFML_BASE_RESTRICT fillVertices,
                                        Vertex* const SFML_BASE_RESTRICT       outlineVertices,
                                        const base::SizeT                      pointCount,
                                        const float                            miterLimit = 4.f) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(outlineThickness > 0.f);
    SFML_BASE_ASSERT_AND_ASSUME(outlineVertices != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(fillVertices != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(pointCount >= 3u);
    SFML_BASE_ASSERT_AND_ASSUME(miterLimit > 0.f);

    const float miterLimitSq = miterLimit * miterLimit; // Precompute squared limit

    for (base::SizeT i = 0u; i < pointCount; ++i)
    {
        const Vector2f& p1 = fillVertices[i].position;
        const Vector2f& p0 = fillVertices[(i == 0) ? (pointCount - 1) : (i - 1)].position; // Previous point
        const Vector2f& p2 = fillVertices[(i + 1) % pointCount].position;                  // Next point

        const Vector2f n1 = computeSegmentNormal(p0, p1);
        const Vector2f n2 = computeSegmentNormal(p1, p2);

        // Calculate the factor `1 + n1.dot(n2)`. This is related to the angle.
        // factor = 1 + cos(angle_between_normals)
        // If angle is near 180 degrees (segments nearly collinear, sharp internal angle), factor is near 0.
        // If angle is near 0 degrees (segments nearly collinear, flat internal angle), factor is near 2.
        const float factor = 1.f + n1.dot(n2);

        // Handle near-collinear segments (factor close to 0) to avoid division issues.
        // Use a simple normal offset in this case (effectively a bevel/butt cap).
        // Threshold chosen somewhat arbitrarily, needs to be small but non-zero.
        constexpr float collinearThreshold = 1e-5f;

        const auto outlineNormal = (SFML_BASE_MATH_FABSF(factor) < collinearThreshold)
                                       ? n1 // Segments are almost parallel/anti-parallel, use one normal
                                       : ((n1 + n2) / factor).clampMaxLengthSquared(miterLimitSq); // miter vector direction with miter limit

        const auto offsetVector = outlineNormal * outlineThickness;

        // Store the outline vertex positions
        const base::SizeT outIdx = i << 1u; // i * 2

        outlineVertices[outIdx + 0u].position = p1;                // Inner vertex
        outlineVertices[outIdx + 1u].position = p1 + offsetVector; // Outer vertex
    }

    // Duplicate the first outline vertex pair at the end to close the outline loop.
    const base::SizeT dupIndex = pointCount << 1u; // pointCount * 2

    outlineVertices[dupIndex + 0u].position = outlineVertices[0].position;
    outlineVertices[dupIndex + 1u].position = outlineVertices[1].position;
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
inline constexpr Vector2f computeConvexShapeGeometricCenter(const Vector2f* points, const base::SizeT pointCount)
{
    SFML_BASE_ASSERT(pointCount > 0u && "Cannot calculate geometric center of shape with no points");

    if (pointCount == 1u)
        return points[0];

    if (pointCount == 2u)
        return (points[0] + points[1]) / 2.f;

    Vector2f centroid{};
    float    twiceArea = 0;

    auto previousPoint = points[pointCount - 1];
    for (base::SizeT i = 0; i < pointCount; ++i)
    {
        const auto  currentPoint = points[i];
        const float product      = previousPoint.cross(currentPoint);
        twiceArea += product;
        centroid += (currentPoint + previousPoint) * product;

        previousPoint = currentPoint;
    }

    if (twiceArea != 0.f)
        return centroid / 3.f / twiceArea;

    // Fallback for no area - find the center of the bounding box
    Vector2f minPoint = points[0];
    Vector2f maxPoint = minPoint;

    for (base::SizeT i = 1; i < pointCount; ++i)
    {
        const auto currentPoint = points[i];

        minPoint.x = SFML_BASE_MIN(minPoint.x, currentPoint.x);
        maxPoint.x = SFML_BASE_MAX(maxPoint.x, currentPoint.x);
        minPoint.y = SFML_BASE_MIN(minPoint.y, currentPoint.y);
        maxPoint.y = SFML_BASE_MAX(maxPoint.y, currentPoint.y);
    }

    return (maxPoint + minPoint) / 2.f;
}

} // namespace sf::ShapeUtils


////////////////////////////////////////////////////////////
/// \class sf::ShapeUtils
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
