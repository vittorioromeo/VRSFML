#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Builtins/Assume.hpp"
#include "SFML/Base/Builtins/Restrict.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
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
    const IndexType idx0 = startIndex + i;
    const IndexType idx1 = startIndex + i + 1u;
    const IndexType idx2 = startIndex + i + 2u;

    if ((i % 2u) == 0u)
    {
        // Even triangle: uses vertices `(i, i+1, i+2)` in that order
        *indexPtr++ = idx0;
        *indexPtr++ = idx1;
        *indexPtr++ = idx2;
    }
    else
    {
        // Odd triangle: uses vertices `(i, i+1, i+2)` but in order `(i+2, i+1, i)` to flip winding
        *indexPtr++ = idx2;
        *indexPtr++ = idx1;
        *indexPtr++ = idx0;
    }
}

////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendQuadIndices(IndexType*& indexPtr, const IndexType startIndex) noexcept
{
    appendTriangleIndices(indexPtr, startIndex);      // Triangle strip: triangle #0
    appendTriangleIndices(indexPtr, startIndex + 1u); // Triangle strip: triangle #1
}

////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendPreTransformedSpriteVertices(
    const Transform& transform,
    const FloatRect& textureRect,
    const Color      color,
    Vertex* const    vertexPtr)
{
    const auto& [position, size] = textureRect;
    const Vec2f absSize{SFML_BASE_MATH_FABSF(size.x), SFML_BASE_MATH_FABSF(size.y)};

    // Position
    vertexPtr[0].position.x = transform.a02;
    vertexPtr[0].position.y = transform.a12;

    vertexPtr[1].position.x = transform.a01 * absSize.y + transform.a02;
    vertexPtr[1].position.y = transform.a11 * absSize.y + transform.a12;

    vertexPtr[2].position.x = transform.a00 * absSize.x + transform.a02;
    vertexPtr[2].position.y = transform.a10 * absSize.x + transform.a12;

    vertexPtr[3].position = transform.transformPoint(absSize);

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
    // The code below should be equivalent to:
    /*
        vertexPtr[0] = {transform.transformPoint(a.position), a.color, a.texCoords};
        vertexPtr[1] = {transform.transformPoint(b.position), b.color, b.texCoords};
        vertexPtr[2] = {transform.transformPoint(c.position), c.color, c.texCoords};
        vertexPtr[3] = {transform.transformPoint(d.position), d.color, d.texCoords};
    */

    SFML_BASE_ASSUME(a.position.x == c.position.x);
    SFML_BASE_ASSUME(b.position.x == d.position.x);

    SFML_BASE_ASSUME(a.position.y == b.position.y);
    SFML_BASE_ASSUME(c.position.y == d.position.y);

    const float t00ax = transform.a00 * a.position.x;
    const float t10ax = transform.a10 * a.position.x;
    const float t00bx = transform.a00 * b.position.x;
    const float t10bx = transform.a10 * b.position.x;

    const float t01ayPlusT02 = transform.a01 * a.position.y + transform.a02;
    const float t11ayPlusT12 = transform.a11 * a.position.y + transform.a12;
    const float t01cyPlusT02 = transform.a01 * c.position.y + transform.a02;
    const float t11cyPlusT12 = transform.a11 * c.position.y + transform.a12;

    vertexPtr[0] = {{t00ax + t01ayPlusT02, t10ax + t11ayPlusT12}, a.color, a.texCoords};
    vertexPtr[1] = {{t00bx + t01ayPlusT02, t10bx + t11ayPlusT12}, b.color, b.texCoords};
    vertexPtr[2] = {{t00ax + t01cyPlusT02, t10ax + t11cyPlusT12}, c.color, c.texCoords};
    vertexPtr[3] = {{t00bx + t01cyPlusT02, t10bx + t11cyPlusT12}, d.color, d.texCoords};
}

////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void appendSpriteIndicesAndVertices(
    const Sprite&   sprite,
    const IndexType nextIndex,
    IndexType*      indexPtr,
    Vertex* const   vertexPtr) noexcept
{
    appendQuadIndices(indexPtr, nextIndex);
    appendPreTransformedSpriteVertices(sprite.getTransform(), sprite.textureRect, sprite.color, vertexPtr);
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

    for (IndexType i = 1u; i < fillSize - 1; ++i)
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

    for (IndexType i = 0u; i < outlineSize - 2; ++i)
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

} // namespace sf
