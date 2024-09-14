#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Vertex.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
void DrawableBatch::add(RenderTarget& rt, const Sprite& sprite)
{
    const auto nextIndex = static_cast<IndexType>(m_nVerts);

    // m_indices.reserveMore(6u);
    auto* indices = static_cast<IndexType*>(rt.getIndicesPtr(sizeof(IndexType) * (m_nIdxs + 6u))) + m_nIdxs;

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
    auto* vertices = static_cast<Vertex*>(rt.getVerticesPtr(sizeof(Vertex) * (m_nVerts + 4u))) + m_nVerts;
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
        const auto nextFillIndex = static_cast<IndexType>(m_nVerts);

    auto* indices = static_cast<IndexType*>(rt.getIndicesPtr(m_nIdxs + count)) + count;
        m_indices.reserveMore(fillSize * 3u);

        for (IndexType i = 1u; i < fillSize - 1; ++i)
            m_indices.unsafePushBackMultiple(static_cast<IndexType>(nextFillIndex),
                                             static_cast<IndexType>(nextFillIndex + i),
                                             static_cast<IndexType>(nextFillIndex + i + 1u));

        appendPreTransformedVertices(fillData, fillSize, shape.getTransform());
    }

    // Triangle strip
    if (const auto [outlineData, outlineSize] = shape.getOutlineVertices(); outlineSize > 2u)
    {
        const auto nextOutlineIndex = static_cast<IndexType>(m_nVerts);

        m_indices.reserveMore(outlineSize * 3u);

        for (IndexType i = 0u; i < outlineSize - 2; ++i)
            m_indices.unsafePushBackMultiple(static_cast<IndexType>(nextOutlineIndex + i),
                                             static_cast<IndexType>(nextOutlineIndex + i + 1u),
                                             static_cast<IndexType>(nextOutlineIndex + i + 2u));

        appendPreTransformedVertices(outlineData, outlineSize, shape.getTransform());
    }
#endif
}


////////////////////////////////////////////////////////////
void DrawableBatch::addSubsequentIndices(RenderTarget& rt, base::SizeT count)
{
    // m_indices.reserveMore(count);
    auto* indices = static_cast<IndexType*>(rt.getIndicesPtr(sizeof(IndexType) * (m_nIdxs + count))) + m_nIdxs;

    const auto nextIndex = static_cast<IndexType>(m_nVerts);

    for (IndexType i = 0u; i < static_cast<IndexType>(count); ++i)
        *indices++ = static_cast<IndexType>(nextIndex + i);

    m_nIdxs += count;

}


////////////////////////////////////////////////////////////
void DrawableBatch::clear()
{
    // m_vertices.clear();
    m_nVerts = 0u;

    // m_indices.clear();
    m_nIdxs = 0u;
}

} // namespace sf
