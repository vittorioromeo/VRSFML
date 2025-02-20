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
[[nodiscard, gnu::always_inline, gnu::const]] inline sf::Vector2f computeNormal(sf::Vector2f p1, sf::Vector2f p2) noexcept
{
    const sf::Vector2f normal = (p2 - p1).perpendicular();
    const float        length = normal.length();

    return length != 0.f ? normal / length : normal;
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
                                        const base::SizeT count) noexcept
{
    for (base::SizeT i = 0u; i < count; ++i)
    {
        const base::SizeT index = i + 1u;

        // Get the two segments shared by the current point
        const Vector2f p0 = (i == 0u) ? vertices[count].position : vertices[index - 1].position;
        const Vector2f p1 = vertices[index].position;
        const Vector2f p2 = vertices[index + 1].position;

        // Compute their normal
        Vector2f n1 = computeNormal(p0, p1);
        Vector2f n2 = computeNormal(p1, p2);

        // Make sure that the normals point towards the outside of the shape
        // (this depends on the order in which the points were defined)
        if (n1.dot(vertices[0].position - p1) > 0.f)
            n1 = -n1;

        if (n2.dot(vertices[0].position - p1) > 0.f)
            n2 = -n2;

        // Combine them to get the extrusion direction
        const float    factor = 1.f + (n1.x * n2.x + n1.y * n2.y);
        const Vector2f normal = (n1 + n2) / factor;

        // Update the outline points
        vertices[verticesEndIndex + (i * 2 + 0)].position = p1;
        vertices[verticesEndIndex + (i * 2 + 1)].position = p1 + normal * outlineThickness;
    }

    // Duplicate the first point at the end, to close the outline
    vertices[verticesEndIndex + (count * 2 + 0)].position = vertices[verticesEndIndex + 0].position;
    vertices[verticesEndIndex + (count * 2 + 1)].position = vertices[verticesEndIndex + 1].position;
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
