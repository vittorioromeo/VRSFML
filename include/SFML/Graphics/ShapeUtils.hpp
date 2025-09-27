#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtins/Restrict.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FastSinCos.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::ShapeUtils
{
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeEllipsePointFromAngleStep(
    const base::SizeT index,
    const float       startRadians,
    const float       angleStep,
    const float       hRadius,
    const float       vRadius) noexcept
{
    const float wrappedAngle = base::positiveRemainder(startRadians + static_cast<float>(index) * angleStep, base::tau);
    const auto [sine, cosine] = base::fastSinCos(wrappedAngle);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeEllipsePoint(
    const base::SizeT  index,
    const float        startRadians,
    const unsigned int pointCount,
    const float        hRadius,
    const float        vRadius) noexcept
{
    return computeEllipsePointFromAngleStep(index, startRadians, base::tau / static_cast<float>(pointCount), hRadius, vRadius);
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeCirclePointFromAngleStep(
    const base::SizeT index,
    const float       startRadians,
    const float       angleStep,
    const float       radius) noexcept
{
    return computeEllipsePointFromAngleStep(index, startRadians, angleStep, radius, radius);
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeCirclePoint(
    const base::SizeT  index,
    const float        startRadians,
    const unsigned int pointCount,
    const float        radius) noexcept
{
    return computeCirclePointFromAngleStep(index, startRadians, base::tau / static_cast<float>(pointCount), radius);
}

////////////////////////////////////////////////////////////
/// \brief Computes a vertex point for a rectangle's boundary.
///
/// This function calculates the 2D position of one of the four
/// vertices defining a rectangle. The vertices are ordered as
/// follows: top-left (0), top-right (1), bottom-right (2),
/// and bottom-left (3).
///
/// \param index The index of the vertex to compute (0 to 3).
/// \param size The width and height of the rectangle.
///
/// \return The computed 2D position of the boundary vertex.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeRectanglePoint(
    const base::SizeT index,
    const Vec2f       size) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(index <= 4u);

    const Vec2f points[]{{0.f, 0.f}, {size.x, 0.f}, {size.x, size.y}, {0.f, size.y}};
    return points[index];
}

////////////////////////////////////////////////////////////
/// \brief Computes a vertex point for a rounded rectangle's boundary.
///
/// This function calculates the 2D position of a vertex on the boundary
/// of a rounded rectangle. The vertices are generated by iterating
/// through each of the four corners, drawing an arc for each.
///
/// \param index The index of the vertex to compute. This iterates
///              sequentially through the points of each corner.
/// \param size The overall width and height of the rectangle.
/// \param cornerRadius The radius of each rounded corner.
/// \param cornerPointCount The number of vertices used to define each
///                         individual rounded corner.
///
/// \return The computed 2D position of the boundary vertex.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeRoundedRectanglePoint(
    const base::SizeT  index,
    const Vec2f        size,
    const float        cornerRadius,
    const unsigned int cornerPointCount) noexcept
{
    [[maybe_unused]] const auto totalNumPoints = cornerPointCount * 4u;
    SFML_BASE_ASSERT_AND_ASSUME(index < totalNumPoints);

    const base::SizeT centerIndex = index / cornerPointCount;
    SFML_BASE_ASSERT_AND_ASSUME(centerIndex <= 3u);

    const float deltaAngle = (base::halfPi) / static_cast<float>(cornerPointCount - 1u);

    const Vec2f center{(centerIndex == 0 || centerIndex == 3) ? size.x - cornerRadius : cornerRadius,
                       (centerIndex < 2) ? cornerRadius : size.y - cornerRadius};

    const auto [sine, cosine] = base::fastSinCos(deltaAngle * static_cast<float>(index - centerIndex));
    return center + Vec2f{cornerRadius * cosine, -cornerRadius * sine};
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeArrowPoint(
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

    const Vec2f points[7u] = {
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
/// \brief Computes the angular step per point for a pie slice's arc.
///
/// This helper function calculates the constant angular increment
/// between successive points along the curved edge of a pie slice.
/// It's used to distribute the `pointCount` vertices (minus the center
/// and one endpoint, as the angle is between segments) evenly across
/// the `sweepAngle`.
///
/// \param sweepAngle The total angular extent of the pie slice's arc, in radians.
/// \param pointCount The total number of vertices defining the pie slice,
///                   including the center point and all points along the arc.
///                   Must be >= 3.
///
/// \return The computed angle step in radians between points on the arc.
///
/// \see computePieSlicePoint, computePieSlicePointFromArcAngleStep
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computePieSlicePointFromArcAngleStep(
    const base::SizeT index,
    const float       radius,
    const float       arcAngleStep,
    const float       startAngle) noexcept
{

    SFML_BASE_ASSERT_AND_ASSUME(radius >= 0.f); // Radius should be non-negative

    // Vertex `0` is the center of the arc/circle, acting as the hub for the triangle fan.

    if (index == 0u)
        return {radius, radius};

    // Vertices `1` to `pointCount - 1` are on the arc's curved edge.
    // There are `(pointCount - 1)` points on the arc edge.
    // These points span `(pointCount - 2)` segments along the arc.

    const auto  arcPointStep  = static_cast<float>(index - 1u);
    const float wrappedAngle  = base::positiveRemainder(startAngle + arcPointStep * arcAngleStep, base::tau);
    const auto [sine, cosine] = base::fastSinCos(wrappedAngle);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computePieSlicePoint(
    const base::SizeT  index,
    const float        radius,
    const float        startAngle,
    const float        sweepAngle,
    const unsigned int pointCount) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(pointCount >= 3u); // Need center + at least start/end points on arc
    SFML_BASE_ASSERT_AND_ASSUME(sweepAngle > 0.f); // Sweep angle must be positive
    SFML_BASE_ASSERT_AND_ASSUME(index < pointCount);

    return computePieSlicePointFromArcAngleStep(index, radius, computePieSliceArcAngleStep(sweepAngle, pointCount), startAngle);
}

////////////////////////////////////////////////////////////
/// \see computeStarPoint
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeStarPointFromAngleStep(
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr Vec2f computeStarPoint(
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
/// \brief Computes the outer and inner points of a ring.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr auto computeRingPointsFromAngleStep(
    const base::SizeT index,
    const float       startRadians,
    const float       angleStep,
    const float       outerRadius,
    const float       innerRadius)
{
    SFML_BASE_ASSERT_AND_ASSUME(outerRadius >= 0.f);
    SFML_BASE_ASSERT_AND_ASSUME(innerRadius >= 0.f);

    const float wrappedAngle = base::positiveRemainder(startRadians + static_cast<float>(index) * angleStep, base::tau);
    const auto [sine, cosine] = base::fastSinCos(wrappedAngle);

    SFML_BASE_ASSERT_AND_ASSUME(sine >= -1.f && sine <= 1.f);
    SFML_BASE_ASSERT_AND_ASSUME(cosine >= -1.f && cosine <= 1.f);

    // Local center of the ring
    const Vec2f localCenter{outerRadius, outerRadius};

    // Calculate offset from `localCenter`
    const Vec2f outerOffset{outerRadius * cosine, outerRadius * sine};
    const Vec2f innerOffset{innerRadius * cosine, innerRadius * sine};

    struct RingPoints
    {
        Vec2f outerPoint;
        Vec2f innerPoint;
    };

    // Add offset to `localCenter` to get final local position
    return RingPoints{localCenter + outerOffset, localCenter + innerOffset};
}

////////////////////////////////////////////////////////////
/// \brief Computes the normalized perpendicular of a segment.
///
/// Given two points defining a line segment, this function calculates the unit vec2
/// perpendicular to that segment.
///
/// \param p1 The starting point of the segment.
/// \param p2 The ending point of the segment.
///
/// \return The normal vec2 (normalized).
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline Vec2f computeSegmentNormal(const Vec2f p1, const Vec2f p2) noexcept
{
    // Compute the difference and its perpendicular.
    const Vec2f diff   = p2 - p1;
    const Vec2f normal = {-diff.y, diff.x};

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
/// \brief Internal helper to compute vertices for a shape's outline.
///
/// This template function calculates the vertices needed to draw an outline
/// of a given thickness around a shape defined by a set of fill points.
/// It generates a triangle strip, with two vertices (inner and outer)
/// for each point of the base shape, plus two additional vertices to
/// close the outline loop.
///
/// \param outlineThickness The desired thickness of the outline.
/// \param fillPositionFn   A function or lambda that, when called with an index
///                         `i` (from `0` to `pointCount - 1`), returns the
///                         `Vec2f` position of the i-th fill point of the shape.
/// \param outlineVertices  Pointer to the vertex array where the computed
///                         outline vertices will be stored. This array must be
///                         large enough to hold `(pointCount + 1) * 2` vertices.
/// \param pointCount       The number of unique points defining the base shape's
///                         fill area (e.g., for a triangle, this is 3).
/// \param miterLimit       The miter limit, used to control the appearance of
///                         sharp corners. If the miter length (distance from
///                         the inner to the outer corner point) exceeds this
///                         limit, a bevel join will be used instead of a miter join.
///
////////////////////////////////////////////////////////////
inline constexpr void updateOutlineImpl(const float                      outlineThickness,
                                        auto&&                           fillPositionFn,
                                        Vertex* const SFML_BASE_RESTRICT outlineVertices,
                                        const base::SizeT                pointCount,
                                        const float                      miterLimit) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(outlineVertices != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(pointCount >= 3u);
    SFML_BASE_ASSERT_AND_ASSUME(miterLimit > 0.f);

    const float miterLimitSq = miterLimit * miterLimit; // Precompute squared limit

    for (base::SizeT i = 0u; i < pointCount; ++i)
    {
        const Vec2f& p1 = fillPositionFn(i);
        const Vec2f& p0 = fillPositionFn((i == 0u) ? (pointCount - 1u) : (i - 1u)); // Previous point
        const Vec2f& p2 = fillPositionFn((i + 1u) % pointCount);                    // Next point

        const Vec2f n1 = computeSegmentNormal(p0, p1);
        const Vec2f n2 = computeSegmentNormal(p1, p2);

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
                                       : ((n1 + n2) / factor).clampMaxLengthSquared(miterLimitSq); // miter vec2 direction with miter limit

        const auto offsetVec2 = outlineNormal * outlineThickness;

        // Store the outline vertex positions
        const base::SizeT outIdx = i << 1u; // i * 2

        outlineVertices[outIdx + 0u].position = p1;              // Inner vertex
        outlineVertices[outIdx + 1u].position = p1 + offsetVec2; // Outer vertex
    }

    // Duplicate the first outline vertex pair at the end to close the outline loop.
    const base::SizeT dupIndex = pointCount << 1u; // pointCount * 2

    outlineVertices[dupIndex + 0u].position = outlineVertices[0].position;
    outlineVertices[dupIndex + 1u].position = outlineVertices[1].position;
}

////////////////////////////////////////////////////////////
/// \brief Computes outline vertices for a shape with a triangle fan fill.
///
/// This function is a convenience wrapper around `updateOutlineImpl`.
/// It is specifically designed for shapes whose fill is represented by
/// a triangle fan structure (e.g., a central point followed by points
/// tracing the perimeter). It extracts the fill point positions directly
/// from the `fillVertices` array to pass to `updateOutlineImpl`.
///
/// \param outlineThickness The desired thickness of the outline.
/// \param fillVertices     Pointer to the vertex array defining the shape's
///                         fill as a triangle fan. The positions from these
///                         vertices are used as the base for the outline.
/// \param outlineVertices  Pointer to the vertex array where the computed
///                         outline vertices will be stored. This array must be
///                         large enough to hold `(pointCount + 1) * 2` vertices.
/// \param pointCount       The number of points defining the perimeter of the
///                         triangle fan (i.e., total fill vertices minus the
///                         central hub point, if one exists and is part of the count
///                         passed to the underlying `updateOutlineImpl`).
/// \param miterLimit       The miter limit, used to control the appearance of
///                         sharp corners.
///
/// \see updateOutlineImpl
///
////////////////////////////////////////////////////////////
inline constexpr void updateOutlineFromTriangleFanFill(
    const float                            outlineThickness,
    const Vertex* const SFML_BASE_RESTRICT fillVertices,
    Vertex* const SFML_BASE_RESTRICT       outlineVertices,
    const base::SizeT                      pointCount,
    const float                            miterLimit) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(fillVertices != nullptr);

    updateOutlineImpl(outlineThickness,
                      [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return fillVertices[i].position; },
                      outlineVertices,
                      pointCount,
                      miterLimit);
}

////////////////////////////////////////////////////////////
/// \brief Computes the geometric center (centroid) of a convex polygon.
///
/// This function calculates the 2D coordinates of the centroid for a
/// convex polygon defined by an ordered list of vertices.
/// The calculation is based on the formula for the centroid of a polygon,
/// which involves summing the cross products of adjacent vertices.
///
/// Preconditions:
/// - The polygon must be convex.
/// - The vertices in the `points` array must be ordered (e.g., clockwise or counter-clockwise).
/// - `pointCount` must be greater than 0.
///
/// Edge cases:
/// - If `pointCount` is 1, the single point is returned.
/// - If `pointCount` is 2, the midpoint of the segment is returned.
/// - If the polygon has zero area (e.g., collinear points), the center
///   of its bounding box is returned as a fallback.
///
/// \param points Pointer to an array of `Vec2f` representing the vertices
///               of the convex polygon.
/// \param pointCount The number of vertices in the polygon.
///
/// \return `Vec2f` representing the coordinates of the geometric center.
///
////////////////////////////////////////////////////////////
inline constexpr Vec2f computeConvexShapeGeometricCenter(const Vec2f* points, const base::SizeT pointCount)
{
    SFML_BASE_ASSERT(pointCount > 0u && "Cannot calculate geometric center of shape with no points");

    if (pointCount == 1u)
        return points[0];

    if (pointCount == 2u)
        return (points[0] + points[1]) / 2.f;

    Vec2f centroid{};
    float twiceArea = 0;

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
    Vec2f minPoint = points[0];
    Vec2f maxPoint = minPoint;

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
/// \namespace sf::ShapeUtils
/// \ingroup graphics
///
/// \brief Utility functions for geometric calculations related to shapes.
///
/// This namespace contains a collection of helper functions for performing
/// various geometric computations. These include calculating vertex points
/// for primitive shapes like circles, ellipses, rectangles (standard and rounded),
/// arrows, pie slices, and stars. It also provides utilities for generating
/// vertices for shape outlines and for finding the geometric center of
/// convex polygons.
///
/// While these functions are primarily used internally by SFML's shape
/// classes (e.g., `sf::Shape`, `sf::CircleShape`, `sf::RectangleShape`),
/// they may also be useful for advanced users who need to perform custom
/// geometric operations or create new types of drawable shapes.
///
/// \see sf::Shape, sf::CircleShape, sf::RectangleShape, sf::ConvexShape
///
////////////////////////////////////////////////////////////
