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

    // Triangle strip
    m_indices.unsafePushBackMultiple(
        // Triangle 0
        static_cast<IndexType>(nextIndex + 0u),
        static_cast<IndexType>(nextIndex + 1u),
        static_cast<IndexType>(nextIndex + 2u),

        // Triangle 1
        static_cast<IndexType>(nextIndex + 1u),
        static_cast<IndexType>(nextIndex + 2u),
        static_cast<IndexType>(nextIndex + 3u));

    appendPreTransformedVertices(data, size, sprite.getTransform());
}


////////////////////////////////////////////////////////////
void DrawableBatch::add(const Shape& shape)
{
    // Triangle fan
    if (const auto [fillData, fillSize] = shape.getFillVertices(); fillSize > 2u)
    {
        const auto nextFillIndex = static_cast<IndexType>(m_vertices.size());

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
        const auto nextOutlineIndex = static_cast<IndexType>(m_vertices.size());

        m_indices.reserveMore(outlineSize * 3u);

        for (IndexType i = 0u; i < outlineSize - 2; ++i)
            m_indices.unsafePushBackMultiple(static_cast<IndexType>(nextOutlineIndex + i),
                                             static_cast<IndexType>(nextOutlineIndex + i + 1u),
                                             static_cast<IndexType>(nextOutlineIndex + i + 2u));

        appendPreTransformedVertices(outlineData, outlineSize, shape.getTransform());
    }
}


////////////////////////////////////////////////////////////
void DrawableBatch::addSubsequentIndices(base::SizeT count)
{
    const auto nextIndex = static_cast<IndexType>(m_vertices.size());

    m_indices.reserveMore(count);

    for (IndexType i = 0; i < static_cast<IndexType>(count); ++i)
        m_indices.unsafeEmplaceBack(static_cast<IndexType>(nextIndex + i));
}


////////////////////////////////////////////////////////////
void DrawableBatch::appendPreTransformedVertices(const Vertex* data, const base::SizeT count, const Transform& transform)
{
    m_vertices.reserveMore(count);

    for (const auto* const target = data + count; data != target; ++data)
        m_vertices.unsafeEmplaceBack(transform.transformPoint(data->position), data->color, data->texCoords);
}


////////////////////////////////////////////////////////////
void DrawableBatch::clear()
{
    m_vertices.clear();
    m_indices.clear();
}

} // namespace sf