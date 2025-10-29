#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/FastSinCos.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
/// \brief A drawable, wobbling laser beam effect
///
/// This class creates the geometry for a laser beam between two points.
/// It consists of a bright inner core and a wider, softer outer glow.
/// The beam's shape is animated in the `update` method to create a
/// dynamic "wobble" effect, reminiscent of the Ghostbusters' proton streams.
///
////////////////////////////////////////////////////////////
class LaserBeam
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the laser beam
    ///
    /// \param start          The starting point of the beam
    /// \param end            The ending point of the beam
    /// \param color          The color of the beam's core
    /// \param coreThickness  The thickness of the bright inner core
    ///
    ////////////////////////////////////////////////////////////
    explicit LaserBeam(const sf::Vec2f start,
                       const sf::Vec2f end,
                       const sf::Color color         = sf::Color(255, 25, 25),
                       const float     coreThickness = 1.5f) :
        start(start),
        end(end),
        m_color(color),
        m_coreThickness(coreThickness),
        m_glowThickness(coreThickness * 6.0f)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Update the beam's animation
    ///
    /// This recalculates the wobble of the beam based on elapsed time.
    /// \param dt Time elapsed since the last frame
    ///
    ////////////////////////////////////////////////////////////
    void update(const sf::Time dt)
    {
        m_lifetime += dt;

        // --- Primary Wave (large, slow wobble) ---
        constexpr float wobbleAmplitude1  = 1.5f;  // The main size of the wobble
        constexpr float wobbleSpeed1      = 25.0f; // How fast the main wave pattern moves
        constexpr float spatialFrequency1 = 0.12f; // How many wiggles per pixel. Smaller = longer waves.

        // --- Detail Wave (small, fast jitter) ---
        constexpr float wobbleAmplitude2  = 0.7f;  // The size of the smaller, faster ripples
        constexpr float wobbleSpeed2      = 10.0f; // The detail wave should move at a different (usually faster) speed
        constexpr float spatialFrequency2 = 0.25f; // The detail wave should have a higher frequency (more wiggles)

        constexpr float pixelsPerSegment = 10.0f; // How long each segment of the beam is

        const sf::Vec2f beamVector = end - start;
        const float     beamLength = beamVector.length();

        if (beamLength == 0.f)
            return;

        const auto numSegments = static_cast<sf::base::SizeT>(sf::base::ceil(beamLength / pixelsPerSegment));

        if (numSegments == 0u)
            return;

        const sf::Vec2f direction = beamVector / beamLength; // Normalized direction
        const sf::Vec2f normal    = direction.perpendicular();

        // Calculate the current wobbled points
        const auto nBasePoints = numSegments + 1u;

        m_currentPoints.clear();
        m_currentPoints.reserve(nBasePoints);

        const float timeOffset1 = m_lifetime.asSeconds() * wobbleSpeed1;
        const float timeOffset2 = m_lifetime.asSeconds() * wobbleSpeed2;

        for (sf::base::SizeT i = 0u; i < nBasePoints; ++i)
        {
            const float progress = static_cast<float>(i) / static_cast<float>(numSegments);
            const float distance = beamLength * progress;

            // Calculate the displacement from the first (primary) wave
            const float sineInput1 = sf::base::positiveRemainder(timeOffset1 + distance * spatialFrequency1, sf::base::tau);
            const float displacement1 = sf::base::fastSin(sineInput1) * wobbleAmplitude1;

            // Calculate the displacement from the second (detail) wave
            const float sineInput2 = sf::base::positiveRemainder(timeOffset2 + distance * spatialFrequency2, sf::base::tau);
            const float displacement2 = sf::base::fastSin(sineInput2) * wobbleAmplitude2;

            // The final displacement is the sum of both waves
            const float totalDisplacement = displacement1 + displacement2;

            const sf::Vec2f basePoint = start + direction * distance;
            m_currentPoints.pushBack(basePoint + normal * totalDisplacement);
        }

        // The start and end points should not wobble
        m_currentPoints.front() = start;
        m_currentPoints.back()  = end;

        // Regenerate the vertex geometry based on the new wobbled points
        updateVertexGeometry(m_currentPoints);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Draw the laser beam to a render target
    ///
    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& target, const sf::RenderStates states) const
    {
        const auto drawVertices = [&](const sf::base::Vector<sf::Vertex>& vertices)
        {
            target.drawIndexedVertices({
                .vertexData    = vertices.data(),
                .vertexCount   = vertices.size(),
                .indexData     = m_indices.data(),
                .indexCount    = m_indices.size(),
                .primitiveType = sf::PrimitiveType::Triangles,
                .renderStates  = states,
            });
        };

        drawVertices(m_verticesGlow);
        drawVertices(m_verticesCore);
    }

    ////////////////////////////////////////////////////////////
    sf::Vec2f start;
    sf::Vec2f end;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Recalculates all vertex positions based on a list of points
    ///
    /// This is the core geometry generation function, separated so it can be
    /// called by the update loop every frame.
    ///
    ////////////////////////////////////////////////////////////
    void updateVertexGeometry(const sf::base::Vector<sf::Vec2f>& points)
    {
        if (points.size() < 2)
            return;

        const auto numQuads    = static_cast<sf::base::SizeT>(points.size() - 1u);
        const auto vertexCount = numQuads * 4u;
        const auto indexCount  = numQuads * 6u;

        if (m_verticesCore.size() != vertexCount)
        {
            m_verticesCore.resize(vertexCount);
            m_verticesGlow.resize(vertexCount);
            m_indices.resize(indexCount);
        }

        for (sf::base::SizeT i = 0u; i < numQuads; ++i)
        {
            const auto baseIndex = static_cast<sf::IndexType>(i * 4u);
            const auto idxOffset = i * 6u;

            m_indices[idxOffset + 0u] = baseIndex + 0u;
            m_indices[idxOffset + 1u] = baseIndex + 1u;
            m_indices[idxOffset + 2u] = baseIndex + 2u;
            m_indices[idxOffset + 3u] = baseIndex + 0u;
            m_indices[idxOffset + 4u] = baseIndex + 2u;
            m_indices[idxOffset + 5u] = baseIndex + 3u;
        }

        const auto glowColor = m_color.withAlpha(m_color.a / 4u);

        for (sf::base::SizeT i = 0u; i < points.size() - 1u; ++i)
        {
            const sf::Vec2f p1 = points[i];
            const sf::Vec2f p2 = points[i + 1];

            // Calculate the normal for the current segment
            const sf::Vec2f dir    = (p2 - p1).normalized();
            const sf::Vec2f normal = dir.perpendicular();

            // Calculate miter normals for smooth joins between segments
            sf::Vec2f normalP1 = normal;
            if (i > 0)
            {
                const sf::Vec2f prevDir = (p1 - points[i - 1]).normalized();
                normalP1                = (dir + prevDir).normalized().perpendicular();
            }

            sf::Vec2f normalP2 = normal;
            if (i < points.size() - 2)
            {
                const sf::Vec2f nextDir = (points[i + 2] - p2).normalized();
                normalP2                = (nextDir + dir).normalized().perpendicular();
            }

            const auto baseVertexIdx = i * 4u;

            // Update core vertices
            m_verticesCore[baseVertexIdx + 0u] = {{p1 - normalP1 * m_coreThickness * 0.5f}, m_color};
            m_verticesCore[baseVertexIdx + 1u] = {{p2 - normalP2 * m_coreThickness * 0.5f}, m_color};
            m_verticesCore[baseVertexIdx + 2u] = {{p2 + normalP2 * m_coreThickness * 0.5f}, m_color};
            m_verticesCore[baseVertexIdx + 3u] = {{p1 + normalP1 * m_coreThickness * 0.5f}, m_color};

            // Update glow vertices
            m_verticesGlow[baseVertexIdx + 0u] = {{p1 - normalP1 * m_glowThickness * 0.5f}, glowColor};
            m_verticesGlow[baseVertexIdx + 1u] = {{p2 - normalP2 * m_glowThickness * 0.5f}, glowColor};
            m_verticesGlow[baseVertexIdx + 2u] = {{p2 + normalP2 * m_glowThickness * 0.5f}, glowColor};
            m_verticesGlow[baseVertexIdx + 3u] = {{p1 + normalP1 * m_glowThickness * 0.5f}, glowColor};
        }
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    sf::Color m_color;
    float     m_coreThickness;
    float     m_glowThickness;
    sf::Time  m_lifetime;

    sf::base::Vector<sf::Vec2f> m_currentPoints;

    sf::base::Vector<sf::Vertex>    m_verticesCore;
    sf::base::Vector<sf::Vertex>    m_verticesGlow;
    sf::base::Vector<sf::IndexType> m_indices;
};

} // namespace tsurv
