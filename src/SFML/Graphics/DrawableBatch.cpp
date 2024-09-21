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
void DrawableBatch::add(const Sprite& sprite)
{
    // Indices
    {
        const auto  nextIndex = static_cast<IndexType>(m_vertices.size());
        IndexType*& indexPtr  = m_indices.reserveMore(6u);

        // Triangle strip: triangle #0
        *indexPtr++ = nextIndex + 0u;
        *indexPtr++ = nextIndex + 1u;
        *indexPtr++ = nextIndex + 2u;

        // Triangle strip: triangle #1
        *indexPtr++ = nextIndex + 1u;
        *indexPtr++ = nextIndex + 2u;
        *indexPtr++ = nextIndex + 3u;
    }

    // Vertices
    {
        Vertex*& vertexPtr = m_vertices.reserveMore(4u);
        priv::spriteToVertices(sprite, vertexPtr); // does not take a reference
        vertexPtr += 4u;
    }
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(const Shape& shape)
{
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
}


////////////////////////////////////////////////////////////
void DrawableBatch::addSubsequentIndices(base::SizeT count)
{
    auto*& indexPtr = m_indices.reserveMore(count);

    const auto nextIndex = static_cast<IndexType>(m_vertices.size());

    for (IndexType i = 0u; i < static_cast<IndexType>(count); ++i)
        *indexPtr++ = static_cast<IndexType>(nextIndex + i);
}


////////////////////////////////////////////////////////////
void DrawableBatch::clear()
{
    m_vertices.clear();
    m_indices.clear();
}

} // namespace sf
