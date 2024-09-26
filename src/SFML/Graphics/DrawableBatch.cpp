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

#include "SFML/Base/Builtins/Assume.hpp"
#include "SFML/Base/SizeT.hpp"


namespace
{
////////////////////////////////////////////////////////////
using IndexType = sf::DrawableBatch::IndexType;


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendTriangleIndices(IndexType*&     indexPtr,
                                                                                 const IndexType startIndex) noexcept
{
    *indexPtr++ = startIndex + 0u;
    *indexPtr++ = startIndex + 1u;
    *indexPtr++ = startIndex + 2u;
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendTriangleFanIndices(
    IndexType*&     indexPtr,
    const IndexType startIndex,
    const IndexType i) noexcept
{
    *indexPtr++ = startIndex;
    *indexPtr++ = startIndex + i;
    *indexPtr++ = startIndex + i + 1u;
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendQuadIndices(IndexType*& indexPtr, const IndexType startIndex) noexcept
{
    appendTriangleIndices(indexPtr, startIndex);     // Triangle strip: triangle #0
    appendTriangleIndices(indexPtr, startIndex + 1); // Triangle strip: triangle #1
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendPreTransformedQuadVertices(
    sf::Vertex*&         vertexPtr,
    const sf::Transform& transform,
    const sf::Vertex&    a,
    const sf::Vertex&    b,
    const sf::Vertex&    c,
    const sf::Vertex&    d) noexcept
{
    SFML_BASE_ASSUME(a.position.x == c.position.x);
    SFML_BASE_ASSUME(b.position.x == d.position.x);

    SFML_BASE_ASSUME(a.position.y == b.position.y);
    SFML_BASE_ASSUME(c.position.y == d.position.y);

    *vertexPtr++ = {transform.transformPoint(a.position), a.color, a.texCoords};
    *vertexPtr++ = {transform.transformPoint(b.position), b.color, b.texCoords};
    *vertexPtr++ = {transform.transformPoint(c.position), c.color, c.texCoords};
    *vertexPtr++ = {transform.transformPoint(d.position), d.color, d.texCoords};
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
DrawableBatch::IndexType* DrawableBatch::reserveMoreIndicesAndGetPtr(RenderTarget& rt, base::SizeT count)
{
#ifndef USE_GPU
    return static_cast<IndexType*>(rt.getIndicesPtr(sizeof(IndexType) * (m_nIndices + count))) + m_nIndices;
#else
    return m_indices.reserveMore(count);
#endif
}


////////////////////////////////////////////////////////////
Vertex* DrawableBatch::reserveMoreVerticesAndGetPtr(RenderTarget& rt, base::SizeT count)
{
#ifndef USE_GPU
    return static_cast<Vertex*>(rt.getVerticesPtr(sizeof(Vertex) * (m_nVertices + count))) + m_nVertices;
#else
    return m_vertices.reserveMore(count);
#endif
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
        const auto nextIndex = getNextIndex();
        IndexType* indexPtr  = reserveMoreIndicesAndGetPtr(rt, 6u * numQuads);

        for (base::SizeT i = 0u; i < numQuads; ++i)
            appendQuadIndices(indexPtr, static_cast<IndexType>(nextIndex + (i * 4u)));

        registerNIndices(6u * numQuads);
    }

    // Vertices
    {
        const auto transform = text.getTransform();
        Vertex*    vertexPtr = reserveMoreVerticesAndGetPtr(rt, 4u * numQuads);

        for (base::SizeT i = 0u; i < numQuads; ++i)
        {
            const auto idx = i * 6u;
            appendPreTransformedQuadVertices(vertexPtr, transform, data[idx + 0u], data[idx + 1u], data[idx + 2u], data[idx + 5u]);
        }

        registerNVertices(4u * numQuads);
    }
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(RenderTarget& rt, const Sprite& sprite)
{
    // Indices
    {
        const auto nextIndex = getNextIndex();
        IndexType* indexPtr  = reserveMoreIndicesAndGetPtr(rt, 6u);

        appendQuadIndices(indexPtr, nextIndex);
        registerNIndices(6u);
    }

    // Vertices
    {
        Vertex* vertexPtr = reserveMoreVerticesAndGetPtr(rt, 4u);

        priv::spriteToVertices(sprite, vertexPtr); // does not take a reference
        registerNVertices(4u);
    }
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(RenderTarget& rt, const Shape& shape)
{
    // Triangle fan
    if (const auto [fillData, fillSize] = shape.getFillVertices(); fillSize > 2u)
    {
        const auto nextFillIndex = getNextIndex();
        IndexType* indexPtr      = reserveMoreIndicesAndGetPtr(rt, 3u * fillSize);

        for (IndexType i = 1u; i < fillSize - 1; ++i)
            appendTriangleFanIndices(indexPtr, nextFillIndex, i);

        registerNIndices(3u * fillSize);

        appendPreTransformedVertices(rt, fillData, fillSize, shape.getTransform());
    }

    // Triangle strip
    if (const auto [outlineData, outlineSize] = shape.getOutlineVertices(); outlineSize > 2u)
    {
        const auto nextOutlineIndex = getNextIndex();
        IndexType* indexPtr         = reserveMoreIndicesAndGetPtr(rt, 3u * outlineSize);

        for (IndexType i = 0u; i < outlineSize - 2; ++i)
            appendTriangleIndices(indexPtr, nextOutlineIndex + i);

        registerNIndices(3u * outlineSize);

        appendPreTransformedVertices(rt, outlineData, outlineSize, shape.getTransform());
    }
}


////////////////////////////////////////////////////////////
void DrawableBatch::addSubsequentIndices(RenderTarget& rt, base::SizeT count)
{
    const auto nextIndex = getNextIndex();
    IndexType* indexPtr  = reserveMoreIndicesAndGetPtr(rt, count);

    for (IndexType i = 0u; i < static_cast<IndexType>(count); ++i)
        *indexPtr++ = static_cast<IndexType>(nextIndex + i);

    registerNIndices(count);
}


////////////////////////////////////////////////////////////
void DrawableBatch::clear()
{
#ifndef USE_GPU
    m_nVertices = 0u;
    m_nIndices  = 0u;
#else
    m_vertices.clear();
    m_indices.clear();
#endif
}

} // namespace sf
