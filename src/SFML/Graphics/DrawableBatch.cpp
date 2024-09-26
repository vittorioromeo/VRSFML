#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/PersistentGPUBuffer.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
PersistentGPUStorage::PersistentGPUStorage(RenderTarget& renderTarget) :
verticesPersistentBuffer{renderTarget.getVerticesPersistentBuffer()},
indicesPersistentBuffer{renderTarget.getIndicesPersistentBuffer()}
{
}


////////////////////////////////////////////////////////////
void PersistentGPUStorage::clear()
{
    nVertices = nIndices = 0u;
}


////////////////////////////////////////////////////////////
Vertex* PersistentGPUStorage::reserveMoreVerticesAndGetPtr(base::SizeT count)
{
    verticesPersistentBuffer.reserve(sizeof(Vertex) * (nVertices + count));

    Vertex* result = static_cast<Vertex*>(verticesPersistentBuffer.data()) + nVertices;
    nVertices += count;
    return result;
}


////////////////////////////////////////////////////////////
IndexType* PersistentGPUStorage::reserveMoreIndicesAndGetPtr(base::SizeT count)
{
    indicesPersistentBuffer.reserve(sizeof(IndexType) * (nIndices + count));

    IndexType* result = static_cast<IndexType*>(indicesPersistentBuffer.data()) + nIndices;
    nIndices += count;
    return result;
}


////////////////////////////////////////////////////////////
void CPUStorage::clear()
{
    vertices.clear();
    indices.clear();
}


////////////////////////////////////////////////////////////
Vertex* CPUStorage::reserveMoreVerticesAndGetPtr(base::SizeT count)
{
    Vertex* const result = vertices.reserveMore(count);
    vertices.unsafeSetSize(vertices.size() + count);
    return result;
}


////////////////////////////////////////////////////////////
IndexType* CPUStorage::reserveMoreIndicesAndGetPtr(base::SizeT count)
{
    IndexType* const result = indices.reserveMore(count);
    indices.unsafeSetSize(indices.size() + count);
    return result;
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::addTriangles(const Transform& transform, const Vertex* data, base::SizeT size)
{
    appendSubsequentIndices(static_cast<IndexType>(size),
                            m_storage.getNumVertices(),
                            m_storage.reserveMoreIndicesAndGetPtr(size));
    appendPreTransformedVertices(transform, data, size, m_storage.reserveMoreVerticesAndGetPtr(size));
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
                                 m_storage.reserveMoreIndicesAndGetPtr(6u * numQuads),
                                 m_storage.reserveMoreVerticesAndGetPtr(4u * numQuads));
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Sprite& sprite)
{
    appendSpriteIndicesAndVertices(sprite,
                                   m_storage.getNumVertices(),
                                   m_storage.reserveMoreIndicesAndGetPtr(6u),
                                   m_storage.reserveMoreVerticesAndGetPtr(4u));
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Shape& shape)
{
    const auto transform = shape.getTransform();

    const auto [fillData, fillSize] = shape.getFillVertices();
    appendShapeFillIndicesAndVertices(transform,
                                      fillData,
                                      static_cast<IndexType>(fillSize),
                                      m_storage.getNumVertices(),
                                      m_storage.reserveMoreIndicesAndGetPtr(3u * fillSize),
                                      m_storage.reserveMoreVerticesAndGetPtr(fillSize));

    const auto [outlineData, outlineSize] = shape.getOutlineVertices();
    appendShapeOutlineIndicesAndVertices(transform,
                                         outlineData,
                                         static_cast<IndexType>(outlineSize),
                                         m_storage.getNumVertices(),
                                         m_storage.reserveMoreIndicesAndGetPtr(3u * outlineSize),
                                         m_storage.reserveMoreVerticesAndGetPtr(outlineSize));
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::clear()
{
    m_storage.clear();
}


////////////////////////////////////////////////////////////
template class DrawableBatchImpl<CPUStorage>;
template class DrawableBatchImpl<PersistentGPUStorage>;


} // namespace sf
