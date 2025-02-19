#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Builtins/Assume.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/SizeT.hpp"

#include <array>
#include <immintrin.h>
#include <iostream>
#include <vector>

#include <cstdint>


namespace sf
{
////////////////////////////////////////////////////////////
using IndexType = unsigned int;
//
// SIMDBatch: holds data for 8 sprites in Structure-of-Arrays (SoA) style.
//
struct alignas(32) SIMDBatch
{
    // Transform components (8 sprites)
    alignas(32) float a00[8];
    alignas(32) float a01[8];
    alignas(32) float a02[8];
    alignas(32) float a10[8];
    alignas(32) float a11[8];
    alignas(32) float a12[8];

    // Texture rectangle data (8 sprites)
    alignas(32) float texPosX[8];
    alignas(32) float texPosY[8];
    alignas(32) float texSizeX[8];
    alignas(32) float texSizeY[8];

    // Colors (packed 32-bit RGBA: R in highest 8 bits)
    alignas(32) uint32_t colors[8];
};

//
// BatchedSprites collects sprites into batches of 8.
//
struct BatchedSprites
{
    static constexpr size_t SIMD_WIDTH = 8;
    std::vector<SIMDBatch>  batches;
    size_t                  currentCount = 0;
    SIMDBatch               currentBatch{};

public:
    // Call this for each sprite you want to batch.
    void addSprite(const Sprite& sprite)
    {
        const Transform& transform = sprite.getTransform();
        const FloatRect& rect      = sprite.textureRect;

        // Store transform components.
        currentBatch.a00[currentCount] = transform.a00;
        currentBatch.a01[currentCount] = transform.a01;
        currentBatch.a02[currentCount] = transform.a02;
        currentBatch.a10[currentCount] = transform.a10;
        currentBatch.a11[currentCount] = transform.a11;
        currentBatch.a12[currentCount] = transform.a12;

        // Store texture rectangle data.
        // We assume you want the absolute size.
        currentBatch.texPosX[currentCount]  = rect.position.x;
        currentBatch.texPosY[currentCount]  = rect.position.y;
        currentBatch.texSizeX[currentCount] = sf::base::fabs(rect.size.x);
        currentBatch.texSizeY[currentCount] = sf::base::fabs(rect.size.y);

        // Pack color (RGBA8888: R in highest 8 bits)
        const auto& c                     = sprite.color;
        currentBatch.colors[currentCount] = (static_cast<uint32_t>(c.r) << 24) | (static_cast<uint32_t>(c.g) << 16) |
                                            (static_cast<uint32_t>(c.b) << 8) | static_cast<uint32_t>(c.a);

        if (++currentCount == SIMD_WIDTH)
        {
            batches.push_back(currentBatch);
            currentCount = 0;
        }
    }

    // Process all sprites into the vertex and index buffers.
    // 'vertices' must have enough space for (sprites * 4) vertices.
    // 'indices' must have enough space for (sprites * 6) indices.
    void processBatch(Vertex* vertices, uint32_t* indices)
    {
        // Process complete batches.
        size_t nSprite = 0;
        for (const auto& batch : batches)
        {
            processSIMDBatch(batch, vertices, indices, nSprite);
            vertices += SIMD_WIDTH * 4; // 4 vertices per sprite.
            indices += SIMD_WIDTH * 6;  // 6 indices per sprite.
            nSprite += SIMD_WIDTH;
        }
        // Process any remaining sprites (if you haven't reached 8).
        flushPartial(vertices, indices);

        // Reset for next frame.
        batches.clear();
        currentCount = 0;
    }

private:
    // Process a complete batch of 8 sprites using AVX.
    void processSIMDBatch(const SIMDBatch& batch, Vertex* vertices, uint32_t* indices, size_t nSprite)
    {
        // Load transform components.
        const __m256 a00 = _mm256_load_ps(batch.a00);
        const __m256 a01 = _mm256_load_ps(batch.a01);
        const __m256 a02 = _mm256_load_ps(batch.a02);
        const __m256 a10 = _mm256_load_ps(batch.a10);
        const __m256 a11 = _mm256_load_ps(batch.a11);
        const __m256 a12 = _mm256_load_ps(batch.a12);

        // Load the absolute texture sizes.
        const __m256 texSizeX = _mm256_load_ps(batch.texSizeX);
        const __m256 texSizeY = _mm256_load_ps(batch.texSizeY);

        // Compute transformed positions.
        // Vertex 0: local (0,0) → (a02, a12)
        __m256 wx0 = a02;
        __m256 wy0 = a12;
        // Vertex 1: local (0, texSizeY) → (a01*texSizeY + a02, a11*texSizeY + a12)
        __m256 wx1 = _mm256_add_ps(_mm256_mul_ps(texSizeY, a01), a02);
        __m256 wy1 = _mm256_add_ps(_mm256_mul_ps(texSizeY, a11), a12);
        // Vertex 2: local (texSizeX, 0) → (a00*texSizeX + a02, a10*texSizeX + a12)
        __m256 wx2 = _mm256_add_ps(_mm256_mul_ps(texSizeX, a00), a02);
        __m256 wy2 = _mm256_add_ps(_mm256_mul_ps(texSizeX, a10), a12);
        // Vertex 3: local (texSizeX, texSizeY) → (a00*texSizeX + a01*texSizeY + a02, a10*texSizeX + a11*texSizeY + a12)
        __m256 wx3 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(texSizeX, a00), _mm256_mul_ps(texSizeY, a01)), a02);
        __m256 wy3 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(texSizeX, a10), _mm256_mul_ps(texSizeY, a11)), a12);

        // Store the computed positions.
        alignas(32) float posX0[8], posY0[8];
        alignas(32) float posX1[8], posY1[8];
        alignas(32) float posX2[8], posY2[8];
        alignas(32) float posX3[8], posY3[8];
        _mm256_store_ps(posX0, wx0);
        _mm256_store_ps(posY0, wy0);
        _mm256_store_ps(posX1, wx1);
        _mm256_store_ps(posY1, wy1);
        _mm256_store_ps(posX2, wx2);
        _mm256_store_ps(posY2, wy2);
        _mm256_store_ps(posX3, wx3);
        _mm256_store_ps(posY3, wy3);

        // Process texture coordinates.
        const __m256 texPosX = _mm256_load_ps(batch.texPosX);
        const __m256 texPosY = _mm256_load_ps(batch.texPosY);
        // Compute per-vertex texture coordinates.
        __m256 tx0 = texPosX; // vertex 0: (texPosX, texPosY)
        __m256 ty0 = texPosY;
        __m256 tx1 = texPosX; // vertex 1: (texPosX, texPosY + texSizeY)
        __m256 ty1 = _mm256_add_ps(texPosY, texSizeY);
        __m256 tx2 = _mm256_add_ps(texPosX, texSizeX); // vertex 2: (texPosX + texSizeX, texPosY)
        __m256 ty2 = texPosY;
        __m256 tx3 = _mm256_add_ps(texPosX, texSizeX); // vertex 3: (texPosX + texSizeX, texPosY + texSizeY)
        __m256 ty3 = _mm256_add_ps(texPosY, texSizeY);

        // Store texture coordinate results.
        alignas(32) float tX0[8], tY0[8];
        alignas(32) float tX1[8], tY1[8];
        alignas(32) float tX2[8], tY2[8];
        alignas(32) float tX3[8], tY3[8];
        _mm256_store_ps(tX0, tx0);
        _mm256_store_ps(tY0, ty0);
        _mm256_store_ps(tX1, tx1);
        _mm256_store_ps(tY1, ty1);
        _mm256_store_ps(tX2, tx2);
        _mm256_store_ps(tY2, ty2);
        _mm256_store_ps(tX3, tx3);
        _mm256_store_ps(tY3, ty3);

        // Fill the vertex buffer for each of the 8 sprites.
        for (int i = 0; i < 8; i++)
        {
            int baseIndex = i * 4; // 4 vertices per sprite

            // Vertex 0: transformed (0,0)
            vertices[baseIndex + 0].position.x  = posX0[i];
            vertices[baseIndex + 0].position.y  = posY0[i];
            vertices[baseIndex + 0].texCoords.x = tX0[i];
            vertices[baseIndex + 0].texCoords.y = tY0[i];

            // Vertex 1: transformed (0, texSizeY)
            vertices[baseIndex + 1].position.x  = posX1[i];
            vertices[baseIndex + 1].position.y  = posY1[i];
            vertices[baseIndex + 1].texCoords.x = tX1[i];
            vertices[baseIndex + 1].texCoords.y = tY1[i];

            // Vertex 2: transformed (texSizeX, 0)
            vertices[baseIndex + 2].position.x  = posX2[i];
            vertices[baseIndex + 2].position.y  = posY2[i];
            vertices[baseIndex + 2].texCoords.x = tX2[i];
            vertices[baseIndex + 2].texCoords.y = tY2[i];

            // Vertex 3: transformed (texSizeX, texSizeY)
            vertices[baseIndex + 3].position.x  = posX3[i];
            vertices[baseIndex + 3].position.y  = posY3[i];
            vertices[baseIndex + 3].texCoords.x = tX3[i];
            vertices[baseIndex + 3].texCoords.y = tY3[i];

            // Unpack the packed color.
            uint32_t  packedColor = batch.colors[i];
            sf::Color color;
            color.r                       = (packedColor >> 24) & 0xFF;
            color.g                       = (packedColor >> 16) & 0xFF;
            color.b                       = (packedColor >> 8) & 0xFF;
            color.a                       = packedColor & 0xFF;
            vertices[baseIndex + 0].color = color;
            vertices[baseIndex + 1].color = color;
            vertices[baseIndex + 2].color = color;
            vertices[baseIndex + 3].color = color;
        }

        // Generate indices for these 8 sprites.
        for (int i = 0; i < 8; i++)
        {
            int baseVertex       = nSprite + i * 4;
            int idxBase          = i * 6;
            indices[idxBase + 0] = baseVertex + 0;
            indices[idxBase + 1] = baseVertex + 1;
            indices[idxBase + 2] = baseVertex + 2;
            indices[idxBase + 3] = baseVertex + 1;
            indices[idxBase + 4] = baseVertex + 2;
            indices[idxBase + 5] = baseVertex + 3;
        }
    }

    void flushPartial(Vertex* vertices, uint32_t* indices)
    {
        // Implement scalar fallback for remaining sprites
        // ...
    }
};


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
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendPreTransformedSpriteVertices(
    const Transform& transform,
    const FloatRect& textureRect,
    const Color      color,
    Vertex* const    vertexPtr)
{
    const auto& [position, size] = textureRect;
    const Vector2f absSize(base::fabs(size.x), base::fabs(size.y)); // TODO P0: consider dropping support for negative UVs

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
    vertexPtr[1].texCoords = position + Vector2f{0.f, size.y};
    vertexPtr[2].texCoords = position + Vector2f{size.x, 0.f};
    vertexPtr[3].texCoords = position + size;
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
[[gnu::always_inline, gnu::flatten]] inline void appendSpriteIndicesAndVertices(
    const Sprite&   sprite,
    const IndexType nextIndex,
    IndexType*      indexPtr,
    Vertex* const   vertexPtr) noexcept
{
#if 1
    appendQuadIndices(indexPtr, nextIndex);
    appendPreTransformedSpriteVertices(sprite.getTransform(), sprite.textureRect, sprite.color, vertexPtr);
#else

    static BatchedSprites batchProcessor;

    batchProcessor.addSprite(sprite);

    // Auto-flush when vertex buffer is near full
    if (batchProcessor.batches.size() >= 50000)
    {
        batchProcessor.processBatch(vertexPtr - 4 * 8 * 50000, indexPtr - 6 * 8 * 50000);
    }
#endif
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
                                         data[(i * 4u) + 0u],
                                         data[(i * 4u) + 1u],
                                         data[(i * 4u) + 2u],
                                         data[(i * 4u) + 3u]);
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
    SFML_BASE_ASSERT(fillSize > 2u);

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

// TODO P2: explore opportunities for SIMD
