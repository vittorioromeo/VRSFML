#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"


namespace sf
{

////////////////////////////////////////////////////////////
DrawableBatch::IndexType* DrawableBatch::reserveMoreIndicesAndGetPtr(RenderTarget& rt, base::SizeT count) const
{
    return static_cast<IndexType*>(rt.getIndicesPtr(sizeof(IndexType) * (m_nIdxs + count))) + m_nIdxs;
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

    // m_indices.reserveMore(6u);
    auto* indices = reserveMoreIndicesAndGetPtr(rt, 6u);

    // Triangle strip: triangle #0
    *indices++ = static_cast<IndexType>(nextIndex + 0u);
    *indices++ = static_cast<IndexType>(nextIndex + 1u);
    *indices++ = static_cast<IndexType>(nextIndex + 2u);

    // Triangle strip: triangle #1
    *indices++ = static_cast<IndexType>(nextIndex + 1u);
    *indices++ = static_cast<IndexType>(nextIndex + 2u);
    *indices++ = static_cast<IndexType>(nextIndex + 3u);

    m_nIdxs += 6u;

    // m_vertices.reserveMore(4u);
    auto* vertices = reserveMoreVerticesAndGetPtr(rt, 4u);
    sprite.getPreTransformedVertices(vertices);
    m_nVerts += 4;
    // m_vertices.unsafeEmplaceRangeFromFunc([&](Vertex* target) { sprite.getPreTransformedVertices(target); }, 4u);
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
