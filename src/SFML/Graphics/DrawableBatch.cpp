#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/GLPersistentBuffer.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/SizeT.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
PersistentGPUStorage::PersistentGPUStorage(RenderTarget& renderTarget) :
vboPersistentBuffer{renderTarget.getVBOPersistentBuffer()},
eboPersistentBuffer{renderTarget.getEBOPersistentBuffer()}
{
}


////////////////////////////////////////////////////////////
Vertex* PersistentGPUStorage::reserveMoreVertices(base::SizeT count)
{
    vboPersistentBuffer.reserve(sizeof(Vertex) * (nVertices + count));
    return static_cast<Vertex*>(vboPersistentBuffer.data()) + nVertices;
}


////////////////////////////////////////////////////////////
IndexType* PersistentGPUStorage::reserveMoreIndices(base::SizeT count)
{
    eboPersistentBuffer.reserve(sizeof(IndexType) * (nIndices + count));
    return static_cast<IndexType*>(eboPersistentBuffer.data()) + nIndices;
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::addTriangles(const Transform& transform, const Vertex* data, base::SizeT size)
{
    appendIncreasingIndices(static_cast<IndexType>(size), m_storage.getNumVertices(), m_storage.reserveMoreIndices(size));
    m_storage.commitMoreIndices(size);

    appendTransformedVertices(transform, data, size, m_storage.reserveMoreVertices(size));
    m_storage.commitMoreVertices(size);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Text& text)
{
    const auto [data, size] = text.getVertices();
    SFML_BASE_ASSERT(size % 6u == 0);

    const auto numQuads = static_cast<IndexType>(size / 6u);

    appendTextIndicesAndVertices(text.getTransform(),
                                 data,
                                 numQuads,
                                 m_storage.getNumVertices(),
                                 m_storage.reserveMoreIndices(6u * numQuads),
                                 m_storage.reserveMoreVertices(4u * numQuads));

    m_storage.commitMoreIndices(6u * numQuads);
    m_storage.commitMoreVertices(4u * numQuads);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Sprite& sprite)
{
    appendSpriteIndicesAndVertices(sprite,
                                   m_storage.getNumVertices(),
                                   m_storage.reserveMoreIndices(6u),
                                   m_storage.reserveMoreVertices(4u));

    m_storage.commitMoreIndices(6u);
    m_storage.commitMoreVertices(4u);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Shape& shape)
{
    const auto transform = shape.getTransform();

    if (const auto [fillData, fillSize] = shape.getFillVertices(); fillSize > 2u)
    {
        appendShapeFillIndicesAndVertices(transform,
                                          fillData,
                                          static_cast<IndexType>(fillSize),
                                          m_storage.getNumVertices(),
                                          m_storage.reserveMoreIndices(3u * (fillSize - 2u)),
                                          m_storage.reserveMoreVertices(fillSize));

        m_storage.commitMoreIndices(3u * (fillSize - 2u));
        m_storage.commitMoreVertices(fillSize);
    }

    if (const auto [outlineData, outlineSize] = shape.getOutlineVertices(); outlineSize > 2u)
    {
        appendShapeOutlineIndicesAndVertices(transform,
                                             outlineData,
                                             static_cast<IndexType>(outlineSize),
                                             m_storage.getNumVertices(),
                                             m_storage.reserveMoreIndices(3u * (outlineSize - 2u)),
                                             m_storage.reserveMoreVertices(outlineSize));

        m_storage.commitMoreIndices(3u * (outlineSize - 2u));
        m_storage.commitMoreVertices(outlineSize);
    }
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::clear()
{
    m_storage.clear();
}


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////
template class DrawableBatchImpl<CPUStorage>;
template class DrawableBatchImpl<PersistentGPUStorage>;

} // namespace sf::priv
