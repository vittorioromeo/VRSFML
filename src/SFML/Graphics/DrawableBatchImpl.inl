#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMaxMacros.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Vertex* const SFML_BASE_RESTRICT vertexData,
                                      const base::SizeT                      vertexCount,
                                      const PrimitiveType                    type)
{
    SFML_BASE_ASSERT(type == PrimitiveType::Triangles);
    SFML_BASE_ASSERT(vertexCount % 3u == 0u);

    if (vertexData == nullptr || vertexCount == 0u)
        return;

    const IndexType firstNewVertexIndex = m_storage.getNumVertices();

    SFML_BASE_MEMCPY(m_storage.reserveMoreVertices(vertexCount), vertexData, vertexCount * sizeof(Vertex));
    m_storage.commitMoreVertices(vertexCount);

    IndexType* dstIndices = m_storage.reserveMoreIndices(vertexCount);

    for (base::SizeT i = 0u; i < vertexCount; ++i)
        *dstIndices++ = firstNewVertexIndex + static_cast<IndexType>(i);

    m_storage.commitMoreIndices(vertexCount);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Vertex* const SFML_BASE_RESTRICT    vertexData,
                                      const base::SizeT                         vertexCount,
                                      const IndexType* const SFML_BASE_RESTRICT indexData,
                                      const base::SizeT                         indexCount,
                                      const PrimitiveType                       type)
{
    SFML_BASE_ASSERT(type == PrimitiveType::Triangles);
    SFML_BASE_ASSERT(vertexCount % 3u == 0u);

    if (vertexData == nullptr || vertexCount == 0u || indexData == nullptr || indexCount == 0u)
        return;

    const IndexType firstNewVertexIndex = m_storage.getNumVertices();

    SFML_BASE_MEMCPY(m_storage.reserveMoreVertices(vertexCount), vertexData, vertexCount * sizeof(Vertex));
    m_storage.commitMoreVertices(vertexCount);

    IndexType* dstIndices = m_storage.reserveMoreIndices(indexCount);

    for (base::SizeT i = 0u; i < indexCount; ++i)
        *dstIndices++ = firstNewVertexIndex + indexData[i];

    m_storage.commitMoreIndices(indexCount);
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
void DrawableBatchImpl<TStorage>::add(const Sprite& sprite) // TODO P1: batched versions for (Sprite* b, Sprite* e)
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

        fillBoundsPosition.x = SFML_BASE_MIN(fillBoundsPosition.x, v.position.x);
        fillBoundsPosition.y = SFML_BASE_MIN(fillBoundsPosition.y, v.position.y);

        fillBoundsMaxX = SFML_BASE_MAX(fillBoundsMaxX, v.position.x);
        fillBoundsMaxY = SFML_BASE_MAX(fillBoundsMaxY, v.position.y);
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
                        [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return computeCirclePointFromAngleStep(i, angleStep, sdCircle.radius); });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const EllipseShapeData& sdEllipse)
{
    const float angleStep = computeAngleStep(sdEllipse.pointCount);

    drawShapeFromPoints(sdEllipse.pointCount,
                        sdEllipse,
                        [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return computeEllipsePointFromAngleStep(i, angleStep, sdEllipse.horizontalRadius, sdEllipse.verticalRadius); });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const RectangleShapeData& sdRectangle)
{
    drawShapeFromPoints(4u,
                        sdRectangle,
                        [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return computeRectanglePoint(i, sdRectangle.size); });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const RoundedRectangleShapeData& sdRoundedRectangle)
{
    drawShapeFromPoints(sdRoundedRectangle.cornerPointCount * 4u,
                        sdRoundedRectangle,
                        [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        return computeRoundedRectanglePoint(i,
                                            sdRoundedRectangle.size,
                                            sdRoundedRectangle.cornerRadius,
                                            sdRoundedRectangle.cornerPointCount);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Font& font, const TextData& textData)
{
    if (textData.string.isEmpty())
        return;

    const auto fillQuadCount    = precomputeTextQuadCount(textData.string, textData.style);
    const auto outlineQuadCount = textData.outlineThickness == 0.f ? 0u : fillQuadCount;

    const auto numQuads = fillQuadCount + outlineQuadCount;

    IndexType* indexPtr  = m_storage.reserveMoreIndices(6u * numQuads);
    const auto nextIndex = m_storage.getNumVertices();

    for (IndexType i = 0u; i < numQuads; ++i)
        appendQuadIndices(indexPtr, nextIndex + (i * 4u));

    const auto [sine, cosine] = base::fastSinCos(textData.rotation.asRadians());
    const auto transform      = Transform::from(textData.position, textData.scale, textData.origin, sine, cosine);

    Vertex* vertexPtr = m_storage.reserveMoreVertices(4u * numQuads);

    createTextGeometryAndGetBounds</* CalculateBounds */ false>(/* outlineVertexCount */ outlineQuadCount * 4u,
                                                                font,
                                                                textData.string,
                                                                textData.style,
                                                                textData.characterSize,
                                                                textData.letterSpacing,
                                                                textData.lineSpacing,
                                                                textData.outlineThickness,
                                                                textData.fillColor,
                                                                textData.outlineColor,
                                                                [&](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return addLinePreTransformed(transform, vertexPtr, SFML_BASE_FORWARD(xs)...); },
                                                                [&](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return addGlyphQuadPreTransformed(transform, vertexPtr, SFML_BASE_FORWARD(xs)...); });

    m_storage.commitMoreIndices(6u * numQuads);
    m_storage.commitMoreVertices(4u * numQuads);
}

} // namespace sf::priv
