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
    const auto [data, size] = sprite.getVertices();
    const auto nextIndex    = static_cast<IndexType>(m_vertices.size());

    m_indices.reserveMore(6u);

    m_indices.unsafePushBackMultiple(
        // Triangle 0
        nextIndex + 0u,
        nextIndex + 1u,
        nextIndex + 2u,

        // Triangle 1
        nextIndex + 1u,
        nextIndex + 2u,
        nextIndex + 3u);

    appendPreTransformedVertices(data, size, sprite.getTransform());
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(const Shape& shape)
{
    if (const auto [fillData, fillSize] = shape.getFillVertices(); fillSize > 2u)
    {
        const auto nextFillIndex = static_cast<IndexType>(m_vertices.size());

        m_indices.reserveMore(fillSize * 3u);

        for (IndexType i = 1u; i < fillSize - 1; ++i)
            m_indices.unsafePushBackMultiple(nextFillIndex, nextFillIndex + i, nextFillIndex + i + 1u);

        appendPreTransformedVertices(fillData, fillSize, shape.getTransform());
    }

    if (const auto [outlineData, outlineSize] = shape.getOutlineVertices(); outlineSize > 2u)
    {
        const auto nextOutlineIndex = static_cast<IndexType>(m_vertices.size());

        m_indices.reserveMore(outlineSize * 3u);

        for (IndexType i = 0u; i < outlineSize - 2; ++i)
            m_indices.unsafePushBackMultiple(nextOutlineIndex + i, nextOutlineIndex + i + 1u, nextOutlineIndex + i + 2u);

        appendPreTransformedVertices(outlineData, outlineSize, shape.getTransform());
    }
}


////////////////////////////////////////////////////////////
void DrawableBatch::addSubsequentIndices(base::SizeT count)
{
    const auto nextIndex = static_cast<IndexType>(m_vertices.size());

    m_indices.reserveMore(count);

    for (IndexType i = 0; i < static_cast<IndexType>(count); ++i)
        m_indices.unsafeEmplaceBack(nextIndex + i);
}


////////////////////////////////////////////////////////////
void DrawableBatch::appendPreTransformedVertices(const Vertex* data, base::SizeT size, const Transform& transform)
{
    m_vertices.reserveMore(size);
    m_vertices.unsafeEmplaceRange(data, size);

    for (auto i = m_vertices.size() - size; i < m_vertices.size(); ++i)
        m_vertices[i].position = transform * m_vertices[i].position;
}


////////////////////////////////////////////////////////////
void DrawableBatch::clear()
{
    m_vertices.clear();
    m_indices.clear();
}

} // namespace sf
