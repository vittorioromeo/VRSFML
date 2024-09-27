#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Builtins/Assume.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
using IndexType = unsigned int;


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
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendQuadIndices(IndexType*& indexPtr, const IndexType startIndex) noexcept
{
    appendTriangleIndices(indexPtr, startIndex);     // Triangle strip: triangle #0
    appendTriangleIndices(indexPtr, startIndex + 1); // Triangle strip: triangle #1
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendPreTransformedQuadVertices(
    Vertex*&         vertexPtr,
    const Transform& transform,
    const Vertex&    a,
    const Vertex&    b,
    const Vertex&    c,
    const Vertex&    d) noexcept
{
    SFML_BASE_ASSUME(a.position.x == c.position.x);
    SFML_BASE_ASSUME(b.position.x == d.position.x);

    SFML_BASE_ASSUME(a.position.y == b.position.y);
    SFML_BASE_ASSUME(c.position.y == d.position.y);

    *vertexPtr++ = {transform.transformPoint(a.position), a.color, a.texCoords};
    *vertexPtr++ = {transform.transformPoint(b.position), b.color, b.texCoords};
    *vertexPtr++ = {transform.transformPoint(c.position), c.color, c.texCoords};
    *vertexPtr++ = {transform.transformPoint(d.position), d.color, d.texCoords};
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendSpriteIndicesAndVertices(
    const Sprite&   sprite,
    const IndexType nextIndex,
    IndexType*      indexPtr,
    Vertex* const   vertexPtr) noexcept
{
    appendQuadIndices(indexPtr, nextIndex);
    priv::spriteToVertices(sprite, vertexPtr);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendTextIndicesAndVertices(
    const Transform&    transform,
    const Vertex* const data,
    const IndexType     numQuads,
    const IndexType     nextIndex,
    IndexType*          indexPtr,
    Vertex*             vertexPtr) noexcept
{
    for (IndexType i = 0u; i < numQuads; ++i)
        appendQuadIndices(indexPtr, nextIndex + (i * 4u));

    for (IndexType i = 0u; i < numQuads; ++i)
        appendPreTransformedQuadVertices(vertexPtr,
                                         transform,
                                         data[(i * 6u) + 0u],
                                         data[(i * 6u) + 1u],
                                         data[(i * 6u) + 2u],
                                         data[(i * 6u) + 5u]);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendTransformedVertices(
    const Transform&  transform,
    const Vertex*     data,
    const base::SizeT size,
    Vertex*           vertexPtr)
{
    for (const auto* const target = data + size; data != target; ++data)
        *vertexPtr++ = {transform.transformPoint(data->position), data->color, data->texCoords};
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendShapeFillIndicesAndVertices(
    const Transform&    transform,
    const Vertex* const fillData,
    const IndexType     fillSize,
    const IndexType     nextFillIndex,
    IndexType*          indexPtr,
    Vertex*             vertexPtr) noexcept
{
    if (fillSize <= 2u)
        return;

    for (IndexType i = 1u; i < fillSize - 1; ++i)
        appendTriangleFanIndices(indexPtr, nextFillIndex, i);

    appendTransformedVertices(transform, fillData, fillSize, vertexPtr);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendShapeOutlineIndicesAndVertices(
    const Transform&    transform,
    const Vertex* const outlineData,
    const IndexType     outlineSize,
    const IndexType     nextOutlineIndex,
    IndexType*          indexPtr,
    Vertex*             vertexPtr) noexcept
{
    if (outlineSize <= 2u)
        return;

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
