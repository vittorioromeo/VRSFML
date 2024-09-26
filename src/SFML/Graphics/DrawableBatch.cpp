#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/SizeT.hpp"


namespace
{
////////////////////////////////////////////////////////////
using IndexType = sf::DrawableBatch::IndexType;


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr IndexType* appendTriangleIndices(
    IndexType*      indexPtr,
    const IndexType startIndex) noexcept
{
    *indexPtr++ = startIndex + 0u;
    *indexPtr++ = startIndex + 1u;
    *indexPtr++ = startIndex + 2u;

    return indexPtr;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr IndexType* appendTriangleFanIndices(
    IndexType*      indexPtr,
    const IndexType startIndex,
    const IndexType i) noexcept
{
    *indexPtr++ = startIndex;
    *indexPtr++ = startIndex + i;
    *indexPtr++ = startIndex + i + 1u;

    return indexPtr;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr IndexType* appendQuadIndices(IndexType* indexPtr,
                                                                                              const IndexType startIndex) noexcept
{
    indexPtr = appendTriangleIndices(indexPtr, startIndex);     // Triangle strip: triangle #0
    indexPtr = appendTriangleIndices(indexPtr, startIndex + 1); // Triangle strip: triangle #1

    return indexPtr;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr sf::Vertex* appendPreTransformedQuadVertices(
    sf::Vertex*          vertexPtr,
    const sf::Transform& transform,
    const sf::Vertex&    a,
    const sf::Vertex&    b,
    const sf::Vertex&    c,
    const sf::Vertex&    d) noexcept
{
    *vertexPtr++ = {transform.transformPoint(a.position), a.color, a.texCoords};
    *vertexPtr++ = {transform.transformPoint(b.position), b.color, b.texCoords};
    *vertexPtr++ = {transform.transformPoint(c.position), c.color, c.texCoords};
    *vertexPtr++ = {transform.transformPoint(d.position), d.color, d.texCoords};

    return vertexPtr;
}

} // namespace


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
            (void)appendQuadIndices(indexPtr, static_cast<IndexType>(nextIndex + (i * 4u)));

        m_nIdxs += 6u * numQuads;
    }

    // Vertices
    {
        const auto transform = text.getTransform();
        Vertex*    vertexPtr = reserveMoreVerticesAndGetPtr(rt, 4u * numQuads);

        for (base::SizeT i = 0u; i < numQuads; ++i)
        {
            const auto idx = i * 6u;

            vertexPtr = appendPreTransformedQuadVertices(vertexPtr,
                                                         transform,
                                                         data[idx + 0u],
                                                         data[idx + 1u],
                                                         data[idx + 2u],
                                                         data[idx + 5u]);
        }

        m_nVerts += 4u * numQuads;
    }
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(RenderTarget& rt, const Sprite& sprite)
{
    // Indices
    {
        const auto nextIndex = static_cast<IndexType>(m_nVerts);
        IndexType* indexPtr  = reserveMoreIndicesAndGetPtr(rt, 6u);

        (void)appendQuadIndices(indexPtr, nextIndex);

        m_nIdxs += 6u;
    }

    // Vertices
    {
        Vertex* vertexPtr = reserveMoreVerticesAndGetPtr(rt, 4u);
        priv::spriteToVertices(sprite, vertexPtr); // does not take a reference
        m_nVerts += 4u;
    }
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
            indexPtr = appendTriangleFanIndices(indexPtr, nextFillIndex, i);

        appendPreTransformedVertices(fillData, fillSize, shape.getTransform());
    }

    // Triangle strip
    if (const auto [outlineData, outlineSize] = shape.getOutlineVertices(); outlineSize > 2u)
    {
        const auto  nextOutlineIndex = static_cast<IndexType>(m_vertices.size());
        IndexType*& indexPtr         = m_indices.reserveMore(outlineSize * 3u);

        for (IndexType i = 0u; i < outlineSize - 2; ++i)
            indexPtr = appendTriangleIndices(indexPtr, nextOutlineIndex + i);

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
