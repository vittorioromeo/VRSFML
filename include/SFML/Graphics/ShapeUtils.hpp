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
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr sf::Vector2f computeCirclePointFromAngleStep(
    const sf::base::SizeT index,
    const float           angleStep,
    const float           radius)
{
    const float radians       = static_cast<float>(index) * angleStep;
    const auto [sine, cosine] = sf::base::fastSinCos(radians);

    SFML_BASE_ASSUME(sine >= 0.f && sine <= 1.f);
    SFML_BASE_ASSUME(cosine >= 0.f && cosine <= 1.f);

    return {radius * (1.f + sine), radius * (1.f + cosine)};
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float computeAngleStep(const unsigned int pointCount)
{
    return sf::base::tau / static_cast<float>(pointCount);
}

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr sf::Vector2f computeCirclePoint(
    const sf::base::SizeT index,
    const unsigned int    pointCount,
    const float           radius) noexcept
{
    return computeCirclePointFromAngleStep(index, computeAngleStep(pointCount), radius);
}

////////////////////////////////////////////////////////////
/// \brief Compute the normal of a segment
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline sf::Vector2f computeSegmentNormal(const sf::Vector2f p1,
                                                                                       const sf::Vector2f p2) noexcept
{
    // Compute the difference and its perpendicular.
    const sf::Vector2f diff   = p2 - p1;
    const sf::Vector2f normal = {-diff.y, diff.x};

    // Compute squared length of the normal.
    const float lenSq = normal.x * normal.x + normal.y * normal.y;

    // By computing the squared length (lenSq), we can avoid an extra temporary if the squared length is zero.
    // Only if nonzero do we compute the square root and the reciprocal.

    // Avoid division by zero.
    if (lenSq == 0.f)
        return normal;

    const float invLen = 1.f / sf::base::sqrt(lenSq);
    return {normal.x * invLen, normal.y * invLen};
}

////////////////////////////////////////////////////////////
/// \brief Get bounds of a vertex range
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline constexpr sf::FloatRect getVertexRangeBounds(const sf::Vertex* data, const sf::base::SizeT nVertices) noexcept
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
/// \brief TODO P1: docs
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
