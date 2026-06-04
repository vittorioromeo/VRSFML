#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/DrawIndexedVerticesSettings.hpp"
#include "Zancle/Graphics/IndexType.hpp"
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/RenderStates.hpp"
#include "Zancle/Graphics/RenderTarget.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/System/Time.hpp"
#include "Zancle/System/Vec2.hpp"

#include "ZancleBase/Constants.hpp"
#include "ZancleBase/Math/Ceil.hpp"
#include "ZancleBase/Remainder.hpp"
#include "ZancleBase/SinCosLookup.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"


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
class [[nodiscard]] LaserBeam
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
    explicit LaserBeam(const za::Vec2f theStart,
                       const za::Vec2f theEnd,
                       const za::Color color         = za::Color(255, 25, 25),
                       const float     coreThickness = 1.5f) :
        start(theStart),
        end(theEnd),
        m_color(color),
        m_coreThickness(coreThickness),
        m_glowThickness(coreThickness * 6.f)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Update the beam's animation
    ///
    /// This recalculates the wobble of the beam based on elapsed time.
    /// \param dt Time elapsed since the last frame
    ///
    ////////////////////////////////////////////////////////////
    void update(const za::Time dt)
    {
        m_lifetime += dt;

        // --- Primary Wave (large, slow wobble) ---
        constexpr float wobbleAmplitude1  = 1.5f;  // The main size of the wobble
        constexpr float wobbleSpeed1      = 25.f;  // How fast the main wave pattern moves
        constexpr float spatialFrequency1 = 0.12f; // How many wiggles per pixel. Smaller = longer waves.

        // --- Detail Wave (small, fast jitter) ---
        constexpr float wobbleAmplitude2  = 0.7f;  // The size of the smaller, faster ripples
        constexpr float wobbleSpeed2      = 10.f;  // The detail wave should move at a different (usually faster) speed
        constexpr float spatialFrequency2 = 0.25f; // The detail wave should have a higher frequency (more wiggles)

        constexpr float pixelsPerSegment = 10.f; // How long each segment of the beam is

        const za::Vec2f beamVector = end - start;
        const float     beamLength = beamVector.length();

        if (beamLength == 0.f)
            return;

        const auto numSegments = static_cast<zb::SizeT>(zb::ceil(beamLength / pixelsPerSegment));

        if (numSegments == 0u)
            return;

        const za::Vec2f direction = beamVector / beamLength; // Normalized direction
        const za::Vec2f normal    = direction.perpendicular();

        // Calculate the current wobbled points
        const auto nBasePoints = numSegments + 1u;

        m_currentPoints.clear();
        m_currentPoints.reserve(nBasePoints);

        const float timeOffset1 = m_lifetime.asSeconds() * wobbleSpeed1;
        const float timeOffset2 = m_lifetime.asSeconds() * wobbleSpeed2;

        for (zb::SizeT i = 0u; i < nBasePoints; ++i)
        {
            const float progress = static_cast<float>(i) / static_cast<float>(numSegments);
            const float distance = beamLength * progress;

            // Calculate the displacement from the first (primary) wave
            const float sineInput1 = zb::positiveRemainder(timeOffset1 + distance * spatialFrequency1, zb::tau);
            const float displacement1 = zb::sinLookup(sineInput1) * wobbleAmplitude1;

            // Calculate the displacement from the second (detail) wave
            const float sineInput2 = zb::positiveRemainder(timeOffset2 + distance * spatialFrequency2, zb::tau);
            const float displacement2 = zb::sinLookup(sineInput2) * wobbleAmplitude2;

            // The final displacement is the sum of both waves
            const float totalDisplacement = displacement1 + displacement2;

            const za::Vec2f basePoint = start + direction * distance;
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
    void draw(za::RenderTarget& target, const za::RenderStates states) const
    {
        const auto drawVertices = [&](const zb::Vector<za::Vertex>& vertices)
        {
            target.drawIndexedVertices(
                za::DrawIndexedVerticesSettings{
                    .vertexSpan    = vertices,
                    .indexSpan     = m_indices,
                    .primitiveType = za::PrimitiveType::Triangles,
                },
                states);
        };

        drawVertices(m_verticesGlow);
        drawVertices(m_verticesCore);
    }

    ////////////////////////////////////////////////////////////
    za::Vec2f start;
    za::Vec2f end;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Recalculates all vertex positions based on a list of points
    ///
    /// This is the core geometry generation function, separated so it can be
    /// called by the update loop every frame.
    ///
    ////////////////////////////////////////////////////////////
    void updateVertexGeometry(const zb::Vector<za::Vec2f>& points)
    {
        if (points.size() < 2)
            return;

        const auto numQuads    = static_cast<zb::SizeT>(points.size() - 1u);
        const auto vertexCount = numQuads * 4u;
        const auto indexCount  = numQuads * 6u;

        if (m_verticesCore.size() != vertexCount)
        {
            m_verticesCore.resize(vertexCount);
            m_verticesGlow.resize(vertexCount);
            m_indices.resize(indexCount);
        }

        for (zb::SizeT i = 0u; i < numQuads; ++i)
        {
            const auto baseIndex = static_cast<za::IndexType>(i * 4u);
            const auto idxOffset = i * 6u;

            m_indices[idxOffset + 0u] = baseIndex + 0u;
            m_indices[idxOffset + 1u] = baseIndex + 1u;
            m_indices[idxOffset + 2u] = baseIndex + 2u;
            m_indices[idxOffset + 3u] = baseIndex + 0u;
            m_indices[idxOffset + 4u] = baseIndex + 2u;
            m_indices[idxOffset + 5u] = baseIndex + 3u;
        }

        const auto glowColor = m_color.withAlpha(m_color.a / 4u);

        for (zb::SizeT i = 0u; i < points.size() - 1u; ++i)
        {
            const za::Vec2f p1 = points[i];
            const za::Vec2f p2 = points[i + 1];

            // Calculate the normal for the current segment
            const za::Vec2f dir    = (p2 - p1).normalized();
            const za::Vec2f normal = dir.perpendicular();

            // Calculate miter normals for smooth joins between segments
            za::Vec2f normalP1 = normal;
            if (i > 0)
            {
                const za::Vec2f prevDir = (p1 - points[i - 1]).normalized();
                normalP1                = (dir + prevDir).normalized().perpendicular();
            }

            za::Vec2f normalP2 = normal;
            if (i < points.size() - 2)
            {
                const za::Vec2f nextDir = (points[i + 2] - p2).normalized();
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
    za::Color m_color;
    float     m_coreThickness;
    float     m_glowThickness;
    za::Time  m_lifetime;

    zb::Vector<za::Vec2f> m_currentPoints;

    zb::Vector<za::Vertex>    m_verticesCore;
    zb::Vector<za::Vertex>    m_verticesGlow;
    zb::Vector<za::IndexType> m_indices;
};

} // namespace tsurv
