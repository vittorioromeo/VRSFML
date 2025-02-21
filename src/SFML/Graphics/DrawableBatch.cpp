#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/GLPersistentBuffer.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsSame.hpp"
#include "SFML/Base/TrivialVector.hpp"


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
    SFML_BASE_ASSERT(size % 4u == 0);

    const auto numQuads = static_cast<IndexType>(size / 4u);

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
void DrawableBatchImpl<TStorage>::addShapeFill(const Transform& transform, const Vertex* data, const base::SizeT size)
{
    if (size < 3u) [[unlikely]]
        return;

    const base::SizeT indexCount = 3u * (size - 2u);

    appendShapeFillIndicesAndVertices(transform,
                                      data,
                                      static_cast<IndexType>(size),
                                      m_storage.getNumVertices(),
                                      m_storage.reserveMoreIndices(indexCount),
                                      m_storage.reserveMoreVertices(size));

    m_storage.commitMoreIndices(indexCount);
    m_storage.commitMoreVertices(size);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::addShapeOutline(const Transform& transform, const Vertex* data, const base::SizeT size)
{
    if (size < 3u) [[unlikely]]
        return;

    const base::SizeT indexCount = 3u * (size - 2u);

    appendShapeOutlineIndicesAndVertices(transform,
                                         data,
                                         static_cast<IndexType>(size),
                                         m_storage.getNumVertices(),
                                         m_storage.reserveMoreIndices(indexCount),
                                         m_storage.reserveMoreVertices(size));

    m_storage.commitMoreIndices(indexCount);
    m_storage.commitMoreVertices(size);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Shape& shape)
{
    const auto transform = shape.getTransform();

    const auto [fillData, fillSize]       = shape.getFillVertices();
    const auto [outlineData, outlineSize] = shape.getOutlineVertices();

    addShapeFill(transform, fillData, fillSize);
    addShapeOutline(transform, outlineData, outlineSize);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::drawShapeFromPoints(const base::SizeT nPoints, const auto& descriptor, auto&& pointFn)
{
    if (nPoints < 3u) [[unlikely]]
        return;

    const auto [sine, cosine] = base::fastSinCos(descriptor.rotation.asRadians());
    const auto transform      = Transform::from(descriptor.position, descriptor.scale, descriptor.origin, sine, cosine);

    // TODO P1: improve, also add to RenderTarget

    const base::SizeT vertexCount   = nPoints + 2u; // + 2 for center and repeated first point
    const IndexType   nextFillIndex = m_storage.getNumVertices();

    Vertex* vertices = m_storage.reserveMoreVertices(vertexCount);
    m_storage.commitMoreVertices(vertexCount);

    //
    // Update vertex positions
    for (unsigned int i = 0u; i < nPoints; ++i)
        vertices[1u + i].position = transform.transformPoint(pointFn(i));

    vertices[1u + nPoints].position = vertices[1].position;

    //
    // Update the bounding rectangle
    vertices[0]             = vertices[1]; // so that the result of getBounds() is correct
    const auto insideBounds = getVertexRangeBounds(vertices, vertexCount);

    // Compute the center and make it the first vertex
    vertices[0].position = insideBounds.getCenter();

    //
    // Update fill color
    {
        const auto* end = vertices + vertexCount;
        for (Vertex* vertex = vertices; vertex != end; ++vertex)
            vertex->color = descriptor.fillColor;
    }

    //
    // Update tex coords
    {
        // Make sure not to divide by zero when the points are aligned on a vertical or horizontal line
        const Vector2f safeInsideSize(insideBounds.size.x > 0 ? insideBounds.size.x : 1.f,
                                      insideBounds.size.y > 0 ? insideBounds.size.y : 1.f);

        const auto* end = vertices + vertexCount;
        for (Vertex* vertex = vertices; vertex != end; ++vertex)
        {
            const Vector2f ratio = (vertex->position - insideBounds.position).componentWiseDiv(safeInsideSize);
            vertex->texCoords = descriptor.textureRect.position + descriptor.textureRect.size.componentWiseMul(ratio);
        }
    }

    const base::SizeT indexCount = 3u * (vertexCount - 2u);

    auto* indexPtr = m_storage.reserveMoreIndices(indexCount);

    for (IndexType i = 1u; i < vertexCount - 1; ++i)
        appendTriangleFanIndices(indexPtr, nextFillIndex, i);

    m_storage.commitMoreIndices(indexCount);

    //
    // Update outline if needed
    if (descriptor.outlineThickness == 0.f)
        return;

    {
        if constexpr (SFML_BASE_IS_SAME(decltype(m_storage), CPUStorage))
        {
            SFML_BASE_ASSERT(m_storage.vertices.data() + nextFillIndex == vertices);
        }

        const base::SizeT count        = vertexCount - 2u;
        const base::SizeT outlineCount = (count + 1u) * 2u;

        const IndexType nextOutlineIndex = m_storage.getNumVertices();

        Vertex* outlineVertices = m_storage.reserveMoreVertices(outlineCount);
        m_storage.commitMoreVertices(outlineCount);

        if constexpr (SFML_BASE_IS_SAME(decltype(m_storage), CPUStorage))
        {
            SFML_BASE_ASSERT(m_storage.vertices.data() + nextFillIndex == outlineVertices - vertexCount);
        }

        updateOutlineImpl(descriptor.outlineThickness, vertexCount, outlineVertices - vertexCount, count);

        //
        // Update outline colors
        {
            const auto* end = outlineVertices + outlineCount;
            for (Vertex* vertex = outlineVertices; vertex != end; ++vertex)
                vertex->color = descriptor.outlineColor;
        }

        //
        // Update outline tex coords
        const auto* end = outlineVertices + outlineCount;
        for (Vertex* vertex = outlineVertices; vertex != end; ++vertex)
            vertex->texCoords = descriptor.outlineTextureRect.position;

        const base::SizeT outlineIndexCount = 3u * (outlineCount - 2u);

        auto* outlineIndexPtr = m_storage.reserveMoreIndices(outlineIndexCount);

        for (IndexType i = 0u; i < outlineCount - 2; ++i)
            appendTriangleIndices(outlineIndexPtr, nextOutlineIndex + i);

        m_storage.commitMoreIndices(outlineIndexCount);
    }
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const CircleShapeData& sdCircle)
{
    drawShapeFromPoints(sdCircle.pointCount,
                        sdCircle,
                        [&](const base::SizeT i) __attribute__((always_inline, flatten))
    {
        const float angleStep = sf::base::tau / static_cast<float>(sdCircle.pointCount);
        return computeCirclePointFromAngleStep(i, angleStep, sdCircle.radius);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const RectangleShapeData& sdRectangle)
{
    const Vector2f points[]{{0.f, 0.f},
                            {sdRectangle.size.x, 0.f},
                            {sdRectangle.size.x, sdRectangle.size.y},
                            {0.f, sdRectangle.size.y}};

    drawShapeFromPoints(4u,
                        sdRectangle,
                        [&](const base::SizeT i) __attribute__((always_inline, flatten)) { return points[i]; });
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
