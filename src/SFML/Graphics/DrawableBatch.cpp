#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/SizeT.hpp"


namespace sf
{

////////////////////////////////////////////////////////////
DrawableBatch::IndexType* DrawableBatch::reserveMoreIndicesAndGetPtr(RenderTarget& rt, base::SizeT count) const
{
    return static_cast<IndexType*>(rt.getIndicesPtr(sizeof(IndexType) * (m_nIdxs + count))) + m_nIdxs;
}


// TODO P0: cleanup
////////////////////////////////////////////////////////////
void DrawableBatch::add(RenderTarget& rt, const Text& text)
{
    const auto [data, size] = text.getVertices();
    SFML_BASE_ASSERT(size % 6u == 0);

    const base::SizeT numQuads = size / 6u;

    // Indices
    {
        const auto nextIndex = static_cast<IndexType>(m_nVerts);
        IndexType* indexPtr  = reserveMoreIndicesAndGetPtr(rt, 6u * numQuads);

        for (base::SizeT i = 0u; i < numQuads; ++i)
        {
            const auto idx = static_cast<IndexType>(nextIndex + (i * 4u));

            // Triangle strip: triangle #0
            *indexPtr++ = idx + 0u;
            *indexPtr++ = idx + 1u;
            *indexPtr++ = idx + 2u;

            // Triangle strip: triangle #1
            *indexPtr++ = idx + 1u;
            *indexPtr++ = idx + 2u;
            *indexPtr++ = idx + 3u;
        }

        m_nIdxs += 6u * numQuads;
    }

    // Vertices
    {
        const auto transform = text.getTransform();
        Vertex*    vertexPtr = reserveMoreVerticesAndGetPtr(rt, 4u * numQuads);

        for (base::SizeT i = 0u; i < numQuads; ++i)
        {
            const auto idx = i * 6u;

            const Vertex& a = data[idx + 0u];
            const Vertex& b = data[idx + 1u];
            const Vertex& c = data[idx + 2u];
            const Vertex& d = data[idx + 5u];

            *vertexPtr++ = {transform.transformPoint(a.position), a.color, a.texCoords};
            *vertexPtr++ = {transform.transformPoint(b.position), b.color, b.texCoords};
            *vertexPtr++ = {transform.transformPoint(c.position), c.color, c.texCoords};
            *vertexPtr++ = {transform.transformPoint(d.position), d.color, d.texCoords};
        }

        m_nVerts += 4u * numQuads;
    }
}


////////////////////////////////////////////////////////////
Vertex* DrawableBatch::reserveMoreVerticesAndGetPtr(RenderTarget& rt, base::SizeT count) const
{
    return static_cast<Vertex*>(rt.getVerticesPtr(sizeof(Vertex) * (m_nVerts + count))) + m_nVerts;
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(RenderTarget& rt, const Sprite& sprite)
{
    const auto nextIndex = static_cast<IndexType>(m_nVerts);
    IndexType* indexPtr  = reserveMoreIndicesAndGetPtr(rt, 6u);

    // Triangle strip: triangle #0
    *indexPtr++ = static_cast<IndexType>(nextIndex + 0u);
    *indexPtr++ = static_cast<IndexType>(nextIndex + 1u);
    *indexPtr++ = static_cast<IndexType>(nextIndex + 2u);

    // Triangle strip: triangle #1
    *indexPtr++ = static_cast<IndexType>(nextIndex + 1u);
    *indexPtr++ = static_cast<IndexType>(nextIndex + 2u);
    *indexPtr++ = static_cast<IndexType>(nextIndex + 3u);

    m_nIdxs += 6u;

    Vertex* vertices = reserveMoreVerticesAndGetPtr(rt, 4u);
    priv::spriteToVertices(sprite, vertices);
    m_nVerts += 4u;
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(RenderTarget& rt, const Shape& shape)
{
#if 0
    // Triangle fan
    if (const auto [fillData, fillSize] = shape.getFillVertices(); fillSize > 2u)
    {
        const auto  nextFillIndex = static_cast<IndexType>(m_vertices.size());
        IndexType*& indexPtr      = m_indices.reserveMore(fillSize * 3u);

        for (IndexType i = 1u; i < fillSize - 1; ++i)
        {
            *indexPtr++ = nextFillIndex;
            *indexPtr++ = nextFillIndex + i;
            *indexPtr++ = nextFillIndex + i + 1u;
        }

        appendPreTransformedVertices(fillData, fillSize, shape.getTransform());
    }

    // Triangle strip
    if (const auto [outlineData, outlineSize] = shape.getOutlineVertices(); outlineSize > 2u)
    {
        const auto  nextOutlineIndex = static_cast<IndexType>(m_vertices.size());
        IndexType*& indexPtr         = m_indices.reserveMore(outlineSize * 3u);

        for (IndexType i = 0u; i < outlineSize - 2; ++i)
        {
            *indexPtr++ = nextOutlineIndex + i + 0u;
            *indexPtr++ = nextOutlineIndex + i + 1u;
            *indexPtr++ = nextOutlineIndex + i + 2u;
        }

        appendPreTransformedVertices(outlineData, outlineSize, shape.getTransform());
    }
#endif
}


////////////////////////////////////////////////////////////
void DrawableBatch::addSubsequentIndices(RenderTarget& rt, base::SizeT count)
{
    const auto nextIndex = static_cast<IndexType>(m_nVerts);
    IndexType* indexPtr  = reserveMoreIndicesAndGetPtr(rt, count);

    for (IndexType i = 0u; i < static_cast<IndexType>(count); ++i)
        *indexPtr++ = static_cast<IndexType>(nextIndex + i);

    m_nIdxs += count;
}


////////////////////////////////////////////////////////////
void DrawableBatch::clear()
{
    m_nVerts = 0u;
    m_nIdxs  = 0u;
}

} // namespace sf
