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

#include "SFML/System/Angle.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
class [[nodiscard]] LightningBolt
{
private:
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] BoltSegment
    {
        sf::Vec2f    start;
        sf::Vec2f    end;
        float        thickness;
        int          depth;
        sf::base::U8 alpha;
    };

public:
    ////////////////////////////////////////////////////////////
    explicit LightningBolt(auto&&          rng,
                           const sf::Vec2f start,
                           const sf::Vec2f end,
                           const sf::Color color = sf::Color(200, 220, 255)) :
        m_color(color),
        m_duration(sf::seconds(0.27f))
    {
        constexpr float mainBoltJaggedness   = 0.35f;
        constexpr float mainBoltThickness    = 3.0f;
        constexpr float branchChance         = 0.9f;
        constexpr int   maxBranchDepth       = 8;
        constexpr float branchLengthMin      = 0.4f;
        constexpr float branchLengthMax      = 0.7f;
        constexpr float branchThicknessDecay = 0.45f;
        constexpr int   jaggednessDepth      = 6;
        constexpr float branchAlphaDecay     = 0.45f;


        sf::base::Vector<BoltSegment> segmentsToProcess{{start, end, mainBoltThickness, 0, 255u}};
        sf::base::Vector<sf::Vec2f>   pointChainBuffer;

        while (!segmentsToProcess.empty())
        {
            const BoltSegment current = segmentsToProcess.back();
            segmentsToProcess.popBack();

            // Step 1: Generate the final point chain directly. No more intermediate segments.
            pointChainBuffer.clear();
            generatePointChain(pointChainBuffer, rng, current.start, current.end, mainBoltJaggedness, jaggednessDepth);

            // Step 2: Decide if we should create new branches from this point chain.
            if (current.depth < maxBranchDepth && rng.getF(0.f, 1.f) < branchChance)
            {
                const sf::Vec2f dir = current.end - current.start;

                const auto startPointIdx = rng.template getI<sf::base::SizeT>(1u, (pointChainBuffer.size() - 2u) / 2u);
                const sf::Vec2f branchStart = pointChainBuffer[startPointIdx];

                // Create the new branch and add it to the queue.
                const float     branchLength = dir.length() * rng.getF(branchLengthMin, branchLengthMax);
                const sf::Angle angle        = dir.angle() + sf::radians(rng.getF(-0.5f, 0.5f));
                const sf::Vec2f branchEnd    = branchStart + sf::Vec2f::fromAngle(branchLength, angle);

                const auto decayedAlpha = static_cast<sf::base::U8>(static_cast<float>(current.alpha) * branchAlphaDecay);

                segmentsToProcess.emplaceBack(branchStart,
                                              branchEnd,
                                              current.thickness * branchThicknessDecay,
                                              current.depth + 1,
                                              decayedAlpha);
            }

            // Step 3: Convert the final point chain into vertices.
            createVertexArray(pointChainBuffer, current.thickness, m_color.withAlpha(current.alpha));
        }
    }

    ////////////////////////////////////////////////////////////
    void update(const sf::Time dt)
    {
        m_lifetime += dt;

        const float progress = m_lifetime.asSeconds() / m_duration.asSeconds();
        const float fade     = 1.f - progress;

        if (progress > 1.f)
            return;

        for (sf::base::SizeT i = 0u; i < m_verticesCore.size(); ++i)
            m_verticesCore[i].color.a = static_cast<sf::base::U8>(static_cast<float>(m_originalCoreAlphas[i]) * fade);

        for (sf::base::SizeT i = 0u; i < m_verticesGlow.size(); ++i)
            m_verticesGlow[i].color.a = static_cast<sf::base::U8>(static_cast<float>(m_originalGlowAlphas[i]) * fade);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isFinished() const
    {
        return m_lifetime >= m_duration;
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& target, const sf::RenderStates states) const
    {
        if (isFinished())
            return;

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

private:
    ////////////////////////////////////////////////////////////
    void generatePointChain(sf::base::Vector<sf::Vec2f>& out,
                            auto&&                       rng,
                            const sf::Vec2f              start,
                            const sf::Vec2f              end,
                            const float                  jaggedness,
                            const int                    depth)
    {
        if (depth <= 0)
        {
            out.resize(2);

            out[0] = start;
            out[1] = end;

            return;
        }

        // 1. Calculate the final size and perform a single allocation.
        const sf::base::SizeT finalPointCount = (1u << depth) + 1u;

        out.resize(finalPointCount);

        out[0] = start;
        out[1] = end;

        // 2. The outer loop replaces recursion, running 'depth' times.
        for (int i = 0; i < depth; ++i)
        {
            // Number of points that were valid in the *previous* pass.
            const sf::base::SizeT pointsInPrevPass = (1u << i) + 1u;

            // 3. Iterate backwards over the segments of the previous pass.
            //    This is the key to making the in-place update safe.
            //    We use a signed int for the loop to safely terminate at j=-1.
            for (int k = static_cast<int>(pointsInPrevPass) - 2; k >= 0; --k)
            {
                const auto j = static_cast<sf::base::SizeT>(k);

                const sf::Vec2f p1 = out[j];
                const sf::Vec2f p2 = out[j + 1u];

                sf::Vec2f midpoint = (p1 + p2) * 0.5f;

                if ((p2 - p1).length() >= 1.f)
                {
                    const sf::Vec2f dir    = (p2 - p1).normalized();
                    const sf::Vec2f normal = dir.perpendicular();
                    const float     offset = rng.getF(-1.f, 1.f) * (p2 - p1).length() * jaggedness;

                    midpoint += normal * offset;
                }

                // 4. Place the points into their new positions for the *next* pass.
                //    p2 (the end of the segment) moves to its new, further location.
                //    The new midpoint is inserted between p1 and p2's new locations.
                //    p1 (at points[2*j]) is already in the correct place from a previous step's write.
                out[j * 2u + 2u] = p2;
                out[j * 2u + 1u] = midpoint;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    void createVertexArray(const sf::base::Vector<sf::Vec2f>& points, const float thickness, const sf::Color color)
    {
        if (points.size() < 2)
            return;

        const float glowThickness = thickness * 4.f;

        // Step 2: Iterate through the segments defined by the points list and build connected quads.
        for (sf::base::SizeT i = 0u; i < points.size() - 1u; ++i)
        {
            const sf::Vec2f p1 = points[i];
            const sf::Vec2f p2 = points[i + 1];

            // Calculate the normal for the current segment
            const sf::Vec2f dir    = (p2 - p1).normalized();
            const sf::Vec2f normal = dir.perpendicular();

            // For smooth joins, we need to calculate the miter normal at each vertex.
            // This normal bisects the angle between the incoming and outgoing segments.
            sf::Vec2f normalP1 = normal;
            sf::Vec2f normalP2 = normal;

            // Calculate miter for the start point of the segment (p1)
            if (i > 0)
            {
                const sf::Vec2f prevDir = (p1 - points[i - 1]).normalized();
                const sf::Vec2f tangent = (dir + prevDir).normalized();

                normalP1 = tangent.perpendicular();
            }

            // Calculate miter for the end point of the segment (p2)
            if (i < points.size() - 2)
            {
                const sf::Vec2f nextDir = (points[i + 2] - p2).normalized();
                const sf::Vec2f tangent = (nextDir + dir).normalized();

                normalP2 = tangent.perpendicular();
            }

            const auto baseIndex = static_cast<sf::IndexType>(m_verticesCore.size());

            // Core vertices
            m_verticesCore.emplaceBack(p1 - normalP1 * thickness * 0.5f, color);
            m_verticesCore.emplaceBack(p2 - normalP2 * thickness * 0.5f, color);
            m_verticesCore.emplaceBack(p2 + normalP2 * thickness * 0.5f, color);
            m_verticesCore.emplaceBack(p1 + normalP1 * thickness * 0.5f, color);
            m_originalCoreAlphas.pushBackMultiple(color.a, color.a, color.a, color.a);

            // Glow vertices
            const auto glowColor = color.withAlpha(color.a / 3u);

            m_verticesGlow.emplaceBack(p1 - normalP1 * glowThickness * 0.5f, glowColor);
            m_verticesGlow.emplaceBack(p2 - normalP2 * glowThickness * 0.5f, glowColor);
            m_verticesGlow.emplaceBack(p2 + normalP2 * glowThickness * 0.5f, glowColor);
            m_verticesGlow.emplaceBack(p1 + normalP1 * glowThickness * 0.5f, glowColor);
            m_originalGlowAlphas.pushBackMultiple(glowColor.a, glowColor.a, glowColor.a, glowColor.a);

            // Indices
            m_indices.pushBackMultiple(baseIndex + 0u, baseIndex + 1u, baseIndex + 2u, baseIndex + 0u, baseIndex + 2u, baseIndex + 3u);
        }
    }

    ////////////////////////////////////////////////////////////
    sf::Color m_color;

    sf::Time m_lifetime;
    sf::Time m_duration;

    sf::base::Vector<sf::Vertex> m_verticesCore;
    sf::base::Vector<sf::Vertex> m_verticesGlow;

    sf::base::Vector<sf::base::U8> m_originalCoreAlphas;
    sf::base::Vector<sf::base::U8> m_originalGlowAlphas;

    sf::base::Vector<sf::IndexType> m_indices;
};

} // namespace tsurv
