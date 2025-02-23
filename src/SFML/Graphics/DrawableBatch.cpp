#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/GLPersistentBuffer.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"
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

    const base::SizeT fillVertexCount = nPoints + 2u;                  // +2 for center and repeated first point
    const IndexType firstFillVertexIndex = m_storage.getNumVertices(); // index of 1st fill vertex (center of the triangle fan)

    Vertex* fillVertices = m_storage.reserveMoreVertices(fillVertexCount);
    m_storage.commitMoreVertices(fillVertexCount);

    //
    // Update fill vertex positions and compute inside bounds
    fillVertices[1].position        = transform.transformPoint(pointFn(0u)); // first point
    sf::Vector2f fillBoundsPosition = fillVertices[1].position;              // left and top

    float fillBoundsMaxX = fillVertices[1].position.x; // right
    float fillBoundsMaxY = fillVertices[1].position.y; // bottom

    for (unsigned int i = 1u; i < nPoints; ++i)
    {
        Vertex& v = fillVertices[1u + i];

        v.position = transform.transformPoint(pointFn(i));

        fillBoundsPosition.x = base::min(fillBoundsPosition.x, v.position.x);
        fillBoundsPosition.y = base::min(fillBoundsPosition.y, v.position.y);

        fillBoundsMaxX = base::max(fillBoundsMaxX, v.position.x);
        fillBoundsMaxY = base::max(fillBoundsMaxY, v.position.y);
    }

    const sf::Vector2f fillBoundsSize{fillBoundsMaxX - fillBoundsPosition.x, fillBoundsMaxY - fillBoundsPosition.y};

    fillVertices[0].position            = fillBoundsPosition + fillBoundsSize / 2.f; // center
    fillVertices[1u + nPoints].position = fillVertices[1].position;                  // repeated first point

    //
    // Update fill color and tex coords (if the shape's fill is visible)
    if (fillBoundsSize.x > 0.f && fillBoundsSize.y > 0.f) [[likely]]
    {
        const Vertex* end = fillVertices + fillVertexCount;
        for (Vertex* vertex = fillVertices; vertex != end; ++vertex)
        {
            vertex->color = descriptor.fillColor;

            const Vector2f ratio = (vertex->position - fillBoundsPosition).componentWiseDiv(fillBoundsSize);
            vertex->texCoords = descriptor.textureRect.position + descriptor.textureRect.size.componentWiseMul(ratio);
        }
    }

    const base::SizeT fillIndexCount = 3u * (fillVertexCount - 2u);

    IndexType* indexPtr = m_storage.reserveMoreIndices(fillIndexCount);

    for (IndexType i = 1u; i < fillVertexCount - 1u; ++i)
        appendTriangleFanIndices(indexPtr, firstFillVertexIndex, i);

    m_storage.commitMoreIndices(fillIndexCount);

    //
    // Update outline if needed
    if (descriptor.outlineThickness == 0.f)
        return;

    const base::SizeT outlineVertexCount = (nPoints + 1u) * 2u;

    const IndexType firstOutlineVertexIndex = m_storage.getNumVertices();

    Vertex* outlineVertices = m_storage.reserveMoreVertices(outlineVertexCount);
    m_storage.commitMoreVertices(outlineVertexCount);

    // Cannot use `vertices` here as the earlier reserve may have invalidated the pointer
    updateOutlineImpl(descriptor.outlineThickness, outlineVertices - fillVertexCount, outlineVertices, nPoints);

    //
    // Update outline colors and outline tex coords
    {
        const Vertex* end = outlineVertices + outlineVertexCount;
        for (Vertex* vertex = outlineVertices; vertex != end; ++vertex)
        {
            vertex->color     = descriptor.outlineColor;
            vertex->texCoords = descriptor.outlineTextureRect.position; // TODO P1: review this logic
        }
    }

    const base::SizeT outlineIndexCount = 3u * (outlineVertexCount - 2u);

    auto* outlineIndexPtr = m_storage.reserveMoreIndices(outlineIndexCount);

    for (IndexType i = 0u; i < outlineVertexCount - 2; ++i)
        appendTriangleIndices(outlineIndexPtr, firstOutlineVertexIndex + i);

    m_storage.commitMoreIndices(outlineIndexCount);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const CircleShapeData& sdCircle)
{
    const float angleStep = computeAngleStep(sdCircle.pointCount);

    drawShapeFromPoints(sdCircle.pointCount,
                        sdCircle,
                        [&](const base::SizeT i) __attribute__((always_inline, flatten))
    { return computeCirclePointFromAngleStep(i, angleStep, sdCircle.radius); });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const EllipseShapeData& sdEllipse)
{
    const float angleStep = computeAngleStep(sdEllipse.pointCount);

    drawShapeFromPoints(sdEllipse.pointCount,
                        sdEllipse,
                        [&](const base::SizeT i) __attribute__((always_inline, flatten))
    { return computeEllipsePointFromAngleStep(i, angleStep, sdEllipse.horizontalRadius, sdEllipse.verticalRadius); });
}

////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const RectangleShapeData& sdRectangle)
{
    drawShapeFromPoints(4u,
                        sdRectangle,
                        [&](const base::SizeT i) __attribute__((always_inline, flatten))
    { return computeRectanglePoint(i, sdRectangle.size); });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const RoundedRectangleShapeData& sdRoundedRectangle)
{
    drawShapeFromPoints(sdRoundedRectangle.cornerPointCount * 4u,
                        sdRoundedRectangle,
                        [&](const base::SizeT i) __attribute__((always_inline, flatten))
    {
        return computeRoundedRectanglePoint(i,
                                            sdRoundedRectangle.size,
                                            sdRoundedRectangle.cornerRadius,
                                            sdRoundedRectangle.cornerPointCount);
    });
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
