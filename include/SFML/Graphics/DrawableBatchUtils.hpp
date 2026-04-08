#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Restrict.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::DrawableBatchUtils
{
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
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendTriangleStripIndices(
    IndexType*&     indexPtr,
    const IndexType startIndex,
    const IndexType i) noexcept
{
    // Even triangle: uses vertices `(i, i+1, i+2)` in that order
    // Odd triangle: uses same vertices, but in reverse order to flip winding

    // If i is even (i & 1 == 0), `swapOffset` is 0.
    // If i is odd (i & 1 == 1), `swapOffset` is 2.
    const IndexType swapOffset = (i & 1u) << 1u;

    *indexPtr++ = startIndex + i + swapOffset;      // even: i+0, odd: i+2
    *indexPtr++ = startIndex + i + 1u;              // always i+1
    *indexPtr++ = startIndex + i + 2u - swapOffset; // even: i+2, odd: i+0
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendQuadIndices(
    IndexType * SFML_BASE_RESTRICT & SFML_BASE_RESTRICT indexPtr,
    const IndexType                                     startIndex) noexcept
{
    // Triangle 0 (Top-left, Bottom-left, Top-right)
    *indexPtr++ = startIndex;
    *indexPtr++ = startIndex + 1u;
    *indexPtr++ = startIndex + 2u;

    // Triangle 1 (Top-right, Bottom-left, Bottom-right)
    *indexPtr++ = startIndex + 2u;
    *indexPtr++ = startIndex + 1u;
    *indexPtr++ = startIndex + 3u;
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendPreTransformedSpriteQuadVertices(
    const Transform&                 transform,
    const Rect2f&                    textureRect,
    const Color                      color,
    Vertex* SFML_BASE_RESTRICT const vertexPtr)
{
    const auto& [position, size] = textureRect;

    const float absSizeX = SFML_BASE_MATH_FABSF(size.x);
    const float absSizeY = SFML_BASE_MATH_FABSF(size.y);

    // Position
    vertexPtr[0].position.x = transform.a02;
    vertexPtr[0].position.y = transform.a12;

    vertexPtr[1].position.x = transform.a01 * absSizeY + transform.a02;
    vertexPtr[1].position.y = transform.a11 * absSizeY + transform.a12;

    vertexPtr[2].position.x = transform.a00 * absSizeX + transform.a02;
    vertexPtr[2].position.y = transform.a10 * absSizeX + transform.a12;

    vertexPtr[3].position.x = transform.a00 * absSizeX + vertexPtr[1].position.x;
    vertexPtr[3].position.y = transform.a10 * absSizeX + vertexPtr[1].position.y;

    // Color
    vertexPtr[0].color = color;
    vertexPtr[1].color = color;
    vertexPtr[2].color = color;
    vertexPtr[3].color = color;

    // Texture Coordinates
    vertexPtr[0].texCoords = position;
    vertexPtr[1].texCoords = position.addY(size.y);
    vertexPtr[2].texCoords = position.addX(size.x);
    vertexPtr[3].texCoords = position + size;
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendPreTransformedTextQuadVertices(
    Vertex* const SFML_BASE_RESTRICT vertexPtr,
    const Transform&                 transform,
    const Vertex& SFML_BASE_RESTRICT a,
    const Vertex& SFML_BASE_RESTRICT b,
    const Vertex& SFML_BASE_RESTRICT c,
    const Vertex& SFML_BASE_RESTRICT d) noexcept
{
    vertexPtr[0] = {transform.transformPoint(a.position), a.color, a.texCoords};
    vertexPtr[1] = {transform.transformPoint(b.position), b.color, b.texCoords};
    vertexPtr[2] = {transform.transformPoint(c.position), c.color, c.texCoords};
    vertexPtr[3] = {transform.transformPoint(d.position), d.color, d.texCoords};
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void appendSpriteIndicesAndVertices(
    const Sprite&                    sprite,
    const IndexType                  nextIndex,
    IndexType* SFML_BASE_RESTRICT    indexPtr,
    Vertex* SFML_BASE_RESTRICT const vertexPtr) noexcept
{
    appendQuadIndices(indexPtr, nextIndex);
    appendPreTransformedSpriteQuadVertices(sprite.getTransform(), sprite.textureRect, sprite.color, vertexPtr);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendTextIndicesAndVertices(
    const Transform&                       transform,
    const Vertex* SFML_BASE_RESTRICT const data,
    const IndexType                        numQuads,
    const IndexType                        nextIndex,
    IndexType*                             indexPtr,
    Vertex* SFML_BASE_RESTRICT             vertexPtr) noexcept
{
    for (IndexType i = 0u; i < numQuads; ++i)
        appendQuadIndices(indexPtr, nextIndex + (i * 4u));

    for (IndexType i = 0u; i < numQuads; ++i)
        appendPreTransformedTextQuadVertices(vertexPtr + (i * 4u),
                                             transform,
                                             data[(i * 4u) + 0u],
                                             data[(i * 4u) + 1u],
                                             data[(i * 4u) + 2u],
                                             data[(i * 4u) + 3u]);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendTransformedVertices(
    const Transform&                 transform,
    const Vertex* SFML_BASE_RESTRICT data,
    const base::SizeT                size,
    Vertex* SFML_BASE_RESTRICT       vertexPtr)
{
    for (const auto* const target = data + size; data != target; ++data)
        *vertexPtr++ = {transform.transformPoint(data->position), data->color, data->texCoords};
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendShapeFillIndicesAndVertices(
    const Transform&                       transform,
    const Vertex* SFML_BASE_RESTRICT const fillData,
    const IndexType                        fillSize,
    const IndexType                        nextFillIndex,
    IndexType*                             indexPtr,
    Vertex* SFML_BASE_RESTRICT             vertexPtr) noexcept
{
    SFML_BASE_ASSERT(fillSize > 2u);

    for (IndexType i = 1u; i < fillSize - 1u; ++i)
        appendTriangleFanIndices(indexPtr, nextFillIndex, i);

    appendTransformedVertices(transform, fillData, fillSize, vertexPtr);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendShapeOutlineIndicesAndVertices(
    const Transform&                       transform,
    const Vertex* SFML_BASE_RESTRICT const outlineData,
    const IndexType                        outlineSize,
    const IndexType                        nextOutlineIndex,
    IndexType*                             indexPtr,
    Vertex* SFML_BASE_RESTRICT             vertexPtr) noexcept
{
    SFML_BASE_ASSERT(outlineSize > 2u);

    for (IndexType i = 0u; i < outlineSize - 2u; ++i)
        appendTriangleIndices(indexPtr, nextOutlineIndex + i);

    appendTransformedVertices(transform, outlineData, outlineSize, vertexPtr);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendIncreasingIndices(const IndexType count,
                                                                                   const IndexType nextIndex,
                                                                                   IndexType*      indexPtr) noexcept
{
    for (IndexType i = 0u; i < count; ++i)
        *indexPtr++ = nextIndex + i;
}

} // namespace sf::DrawableBatchUtils


////////////////////////////////////////////////////////////
/// \namespace sf::DrawableBatchUtils
/// \ingroup graphics
///
/// `sf::DrawableBatchUtils` is a collection of low-level
/// `inline constexpr` helpers used by the drawable batching
/// implementation to append index and vertex data into raw
/// arrays. They are intended for use by `sf::CPUDrawableBatch`,
/// `sf::PersistentGPUDrawableBatch`, and other custom batchers
/// that need maximum throughput when generating quad, triangle,
/// triangle-fan, or triangle-strip primitives.
///
/// Each helper takes a writable index/vertex pointer **by
/// reference** and advances it past the data it just wrote, so
/// they can be chained together cheaply inside a tight loop.
///
/// Most application code does not need this header; it is
/// exposed for advanced users that build their own batching
/// pipelines on top of VRSFML.
///
/// \see `sf::DrawableBatch`, `sf::CPUDrawableBatch`,
///      `sf::PersistentGPUDrawableBatch`, `sf::Vertex`,
///      `sf::IndexType`
///
////////////////////////////////////////////////////////////
