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

    appendShapeFillIndicesAndVertices(transform,
                                      data,
                                      static_cast<IndexType>(size),
                                      m_storage.getNumVertices(),
                                      m_storage.reserveMoreIndices(3u * (size - 2u)),
                                      m_storage.reserveMoreVertices(size));

    m_storage.commitMoreIndices(3u * (size - 2u));
    m_storage.commitMoreVertices(size);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::addShapeOutline(const Transform& transform, const Vertex* data, const base::SizeT size)
{
    if (size < 3u) [[unlikely]]
        return;

    appendShapeOutlineIndicesAndVertices(transform,
                                         data,
                                         static_cast<IndexType>(size),
                                         m_storage.getNumVertices(),
                                         m_storage.reserveMoreIndices(3u * (size - 2u)),
                                         m_storage.reserveMoreVertices(size));

    m_storage.commitMoreIndices(3u * (size - 2u));
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
void DrawableBatchImpl<TStorage>::add(const CircleShapeData& sdCircle)
{
    if (sdCircle.pointCount < 3u) [[unlikely]]
        return;

    // TODO P1: improve, also add to RenderTarget

    base::TrivialVector<Vertex> vertices;
    base::SizeT                 verticesEndIndex = 0u;

    vertices.resize(sdCircle.pointCount + 2u); // + 2 for center and repeated first point
    verticesEndIndex = sdCircle.pointCount + 2u;

    //
    // Update vertex positions
    const float angleStep = sf::base::tau / static_cast<float>(sdCircle.pointCount);

    for (unsigned int i = 0u; i < sdCircle.pointCount; ++i)
        vertices[1u + i].position = computeCirclePointFromAngleStep(i, angleStep, sdCircle.radius);

    vertices[1u + sdCircle.pointCount].position = vertices[1].position;

    //
    // Update the bounding rectangle
    vertices[0]             = vertices[1]; // so that the result of getBounds() is correct
    const auto insideBounds = getVertexRangeBounds(vertices.data(), verticesEndIndex);

    // Compute the center and make it the first vertex
    vertices[0].position = insideBounds.getCenter();

    //
    // Update fill color
    {
        const auto* end = vertices.data() + verticesEndIndex;
        for (Vertex* vertex = vertices.data(); vertex != end; ++vertex)
            vertex->color = sdCircle.fillColor;
    }

    //
    // Update tex coords
    // Make sure not to divide by zero when the points are aligned on a vertical or horizontal line
    const Vector2f safeInsideSize(insideBounds.size.x > 0 ? insideBounds.size.x : 1.f,
                                  insideBounds.size.y > 0 ? insideBounds.size.y : 1.f);

    for (Vertex& vertex : vertices)
    {
        const Vector2f ratio = (vertex.position - insideBounds.position).componentWiseDiv(safeInsideSize);
        vertex.texCoords     = sdCircle.textureRect.position + sdCircle.textureRect.size.componentWiseMul(ratio);
    }

    // TODO: we can already add fill vertices here

    //
    // Update outline if needed
    if (sdCircle.outlineThickness == 0.f)
    {
        verticesEndIndex = vertices.size();
        // TODO: can we return here?
    }
    else
    {
        const base::SizeT count = vertices.size() - 2;
        vertices.resize(verticesEndIndex + (count + 1) * 2);

        updateOutlineImpl(sdCircle.outlineThickness, verticesEndIndex, vertices.data(), count);

        //
        // Update outline colors
        {
            const auto* end = vertices.data() + vertices.size();
            for (Vertex* vertex = vertices.data() + verticesEndIndex; vertex != end; ++vertex)
                vertex->color = sdCircle.outlineColor;
        }

        //
        // Update outline tex coords
        const auto* end = vertices.data() + vertices.size();
        for (Vertex* vertex = vertices.data() + verticesEndIndex; vertex != end; ++vertex)
            vertex->texCoords = sdCircle.outlineTextureRect.position;
    }

    const auto [sine, cosine] = base::fastSinCos(sdCircle.rotation.asRadians());
    const auto transform      = Transform::from(sdCircle.position, sdCircle.scale, sdCircle.origin, sine, cosine);

    addShapeFill(transform, vertices.data(), verticesEndIndex);
    addShapeOutline(transform, vertices.data() + verticesEndIndex, vertices.size() - verticesEndIndex);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const RectangleShapeData& sdRectangle)
{
    // TODO P1: improve, also add to RenderTarget

    // const auto [sine, cosine] = base::fastSinCos(sdRectangle.rotation.asRadians());
    // const auto transform = Transform::from(sdRectangle.position, sdRectangle.scale, sdRectangle.origin, sine, cosine);

    m_shapeBuffer.position = sdRectangle.position;
    m_shapeBuffer.scale    = sdRectangle.scale;
    m_shapeBuffer.origin   = sdRectangle.origin;
    m_shapeBuffer.rotation = sdRectangle.rotation;

    m_shapeBuffer.m_textureRect        = sdRectangle.textureRect;
    m_shapeBuffer.m_outlineTextureRect = sdRectangle.outlineTextureRect;
    m_shapeBuffer.m_fillColor          = sdRectangle.fillColor;
    m_shapeBuffer.m_outlineColor       = sdRectangle.outlineColor;
    m_shapeBuffer.m_outlineThickness   = sdRectangle.outlineThickness;

    m_shapeBuffer.m_vertices.resize(4u + 2u); // + 2 for center and repeated first point
    m_shapeBuffer.m_verticesEndIndex = 4u + 2u;

    //
    // Update vertex positions
    m_shapeBuffer.m_vertices[1u + 0u].position = {0.f, 0.f};
    m_shapeBuffer.m_vertices[1u + 1u].position = {sdRectangle.size.x, 0.f};
    m_shapeBuffer.m_vertices[1u + 2u].position = {sdRectangle.size.x, sdRectangle.size.y};
    m_shapeBuffer.m_vertices[1u + 3u].position = {0.f, sdRectangle.size.y};

    m_shapeBuffer.m_vertices[1u + 4u].position = m_shapeBuffer.m_vertices[1].position;

    //
    // Update the bounding rectangle
    m_shapeBuffer.m_vertices[0] = m_shapeBuffer.m_vertices[1]; // so that the result of getBounds() is correct
    m_shapeBuffer.m_insideBounds = getVertexRangeBounds(m_shapeBuffer.m_vertices.data(), m_shapeBuffer.m_verticesEndIndex);

    // Compute the center and make it the first vertex
    m_shapeBuffer.m_vertices[0].position = m_shapeBuffer.m_insideBounds.getCenter();

    // Updates
    m_shapeBuffer.updateFillColors();
    m_shapeBuffer.updateTexCoords();
    m_shapeBuffer.updateOutline(/* mustUpdateBounds */ false);
    m_shapeBuffer.updateOutlineTexCoords();

    add(m_shapeBuffer);
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
