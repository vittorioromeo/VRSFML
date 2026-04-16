#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/BatchedGeometry.hpp"
#include "SFML/Graphics/ChevronShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/CogShapeData.hpp"
#include "SFML/Graphics/CrossShapeData.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/DrawIndexedVerticesSettings.hpp"
#include "SFML/Graphics/DrawVerticesSettings.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/FontFace.hpp"
#include "SFML/Graphics/GlyphMappedText.hpp"
#include "SFML/Graphics/GlyphMappedTextData.hpp"
#include "SFML/Graphics/GlyphMapping.hpp"
#include "SFML/Graphics/HeartShapeData.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/TrapezoidShapeData.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FloatEpsilon.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SinCosLookup.hpp"
#include "SFML/Base/SizeT.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline void updateOutlineVerticesColorAndTextureRect(const auto&           descriptor,
                                                                            sf::Vertex* const     outlineVertexPtr,
                                                                            const sf::base::SizeT outlineVertexCount)
{
    const sf::Vertex* const end = outlineVertexPtr + outlineVertexCount;
    for (sf::Vertex* vertex = outlineVertexPtr; vertex != end; ++vertex)
    {
        vertex->color     = descriptor.outlineColor;
        vertex->texCoords = descriptor.outlineTextureRect.position; // TODO P1: review this logic
    }
}


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline void generateRingVertices(
    const sf::Rect2f&     textureRect,
    const sf::Color&      fillColor,
    const float           outerRadius,
    const float           innerRadius,
    auto&&                fTransform,
    const sf::base::SizeT numArcPoints,
    const float           startRadians,
    const float           angleStep,
    const sf::Vec2f       invLocalBoundsSize,
    sf::Vertex* const     fillVertexPtr)
{
    for (unsigned int i = 0u; i < numArcPoints; ++i)
    {
        const auto [outerPoint,
                    innerPoint] = sf::ShapeUtils::computeRingPointsFromAngleStep(i, startRadians, angleStep, outerRadius, innerRadius);

        // Outer vertex of the pair
        const sf::Vec2f ratioO   = outerPoint.componentWiseMul(invLocalBoundsSize);
        fillVertexPtr[2 * i + 0] = {
            .position  = fTransform(outerPoint),
            .color     = fillColor,
            .texCoords = textureRect.position + textureRect.size.componentWiseMul(ratioO),
        };

        // Inner vertex of the pair
        const sf::Vec2f ratioI   = innerPoint.componentWiseMul(invLocalBoundsSize);
        fillVertexPtr[2 * i + 1] = {
            .position  = fTransform(innerPoint),
            .color     = fillColor,
            .texCoords = textureRect.position + textureRect.size.componentWiseMul(ratioI),
        };
    }
}


////////////////////////////////////////////////////////////
// Adapter combining GlyphMapping (glyph/metrics) + FontFace (kerning) into a single font source
struct GlyphMappingWithKerning
{
    const sf::GlyphMapping& mapping;
    const sf::FontFace&     fontFace;

    [[nodiscard]] const sf::Glyph& getGlyph(char32_t cp, unsigned int cs, bool b, float ot) const
    {
        return mapping.getGlyph(cp, cs, b, ot);
    }

    [[nodiscard]] sf::GlyphMapping::GlyphPair getFillAndOutlineGlyph(char32_t cp, unsigned int cs, bool b, float ot) const
    {
        return mapping.getFillAndOutlineGlyph(cp, cs, b, ot);
    }

    [[nodiscard]] float getKerning(char32_t first, char32_t second, unsigned int cs, bool b) const
    {
        return fontFace.getKerning(first, second, cs, b);
    }

    [[nodiscard]] float getLineSpacing(unsigned int cs) const
    {
        return mapping.getLineSpacing(cs);
    }

    [[nodiscard]] float getAscent(unsigned int cs) const
    {
        return mapping.getAscent(cs);
    }

    [[nodiscard]] float getDescent(unsigned int cs) const
    {
        return mapping.getDescent(cs);
    }

    [[nodiscard]] float getUnderlinePosition(unsigned int cs) const
    {
        return mapping.getUnderlinePosition(cs);
    }

    [[nodiscard]] float getUnderlineThickness(unsigned int cs) const
    {
        return mapping.getUnderlineThickness(cs);
    }
};

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const DrawVerticesSettings& settings)
{
    const auto& [vertexSpan, type] = settings;

    if (vertexSpan.isNullOrEmpty())
        return;

    const auto* vertexData  = vertexSpan.data();
    const auto  vertexCount = vertexSpan.size();

    IndexType   numTrianglesInStripOrFan = 0u; // Only used for triangle strips and triangle fans
    base::SizeT numIndicesToGenerate     = 0u;

    switch (type)
    {
        case PrimitiveType::Triangles:
        {
            SFML_BASE_ASSERT(vertexCount % 3u == 0u);
            numIndicesToGenerate = vertexCount;
            break;
        }

        case PrimitiveType::TriangleStrip:
        case PrimitiveType::TriangleFan:
        {
            SFML_BASE_ASSERT(vertexCount >= 3u);
            numTrianglesInStripOrFan = static_cast<IndexType>(vertexCount - 2u);
            numIndicesToGenerate     = static_cast<base::SizeT>(numTrianglesInStripOrFan) * 3u;
            break;
        }

        default:
            SFML_BASE_ASSERT(false && "Unsupported primitive type");
            return;
    }

    if (numIndicesToGenerate == 0u)
        return;

    const IndexType firstNewVertexIndex = m_storage.getNumVertices();

    SFML_BASE_MEMCPY(m_storage.reserveMoreVertices(vertexCount), vertexData, vertexCount * sizeof(Vertex));
    m_storage.commitMoreVertices(vertexCount);

    IndexType* dstIndices = m_storage.reserveMoreIndices(numIndicesToGenerate);

    if (type == PrimitiveType::Triangles)
    {
        SFML_BASE_ASSERT(numIndicesToGenerate == vertexCount);

        for (base::SizeT i = 0u; i < numIndicesToGenerate; ++i)
            *dstIndices++ = firstNewVertexIndex + static_cast<IndexType>(i);
    }
    else if (type == PrimitiveType::TriangleStrip)
    {
        for (IndexType i = 0u; i < numTrianglesInStripOrFan; ++i)
            DrawableBatchUtils::appendTriangleStripIndices(dstIndices, firstNewVertexIndex, i);
    }
    else
    {
        SFML_BASE_ASSERT(type == PrimitiveType::TriangleFan);

        for (IndexType i = 0u; i < numTrianglesInStripOrFan; ++i)
            DrawableBatchUtils::appendTriangleFanIndices(dstIndices,
                                                         firstNewVertexIndex,
                                                         /* second vertex relative index */ i + 1u);
    }

    m_storage.commitMoreIndices(numIndicesToGenerate);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const DrawIndexedVerticesSettings& settings)
{
    const auto& [vertexSpan, indexSpan, type] = settings;

    if (vertexSpan.isNullOrEmpty() || indexSpan.isNullOrEmpty())
        return;

    const auto* vertexData  = vertexSpan.data();
    const auto  vertexCount = vertexSpan.size();

    const auto* indexData  = indexSpan.data();
    const auto  indexCount = indexSpan.size();

    IndexType   numTrianglesInStripOrFan = 0u; // Only used for triangle strips and triangle fans
    base::SizeT numIndicesToGenerate     = 0u;

    // Type-specific assertions. The core logic of copying indices is type-agnostic.
    switch (type)
    {
        case PrimitiveType::Triangles:
        {
            SFML_BASE_ASSERT(indexCount % 3u == 0u);
            numIndicesToGenerate = indexCount;
            break;
        }

        case PrimitiveType::TriangleStrip:
        case PrimitiveType::TriangleFan:
        {
            SFML_BASE_ASSERT(indexCount == 0u || indexCount >= 3u);
            numTrianglesInStripOrFan = static_cast<IndexType>(indexCount - 2u);
            numIndicesToGenerate     = static_cast<base::SizeT>(numTrianglesInStripOrFan) * 3u;
            break;
        }

        default:
            SFML_BASE_ASSERT(false && "Unsupported primitive type");
            return;
    }

    if (numIndicesToGenerate == 0u)
        return;

#ifdef SFML_DEBUG
    for (base::SizeT i = 0u; i < indexCount; ++i)
        SFML_BASE_ASSERT(indexData[i] < static_cast<IndexType>(vertexCount));
#endif

    const IndexType firstNewVertexIndex = m_storage.getNumVertices();

    SFML_BASE_MEMCPY(m_storage.reserveMoreVertices(vertexCount), vertexData, vertexCount * sizeof(Vertex));
    m_storage.commitMoreVertices(vertexCount);

    IndexType* dstIndices = m_storage.reserveMoreIndices(numIndicesToGenerate);

    if (type == PrimitiveType::Triangles)
    {
        for (base::SizeT i = 0u; i < indexCount; ++i)
            *dstIndices++ = firstNewVertexIndex + indexData[i];
    }
    else if (type == PrimitiveType::TriangleStrip)
    {
        for (IndexType k = 0u; k < numTrianglesInStripOrFan; ++k) // `k` is the triangle index within the strip
        {
            // Get the global indices for the `k-th`, `(k+1)-th`, and (`k+2)-th` vertices of the strip
            const IndexType vGlobalK0 = firstNewVertexIndex + indexData[k + 0u];
            const IndexType vGlobalK1 = firstNewVertexIndex + indexData[k + 1u];
            const IndexType vGlobalK2 = firstNewVertexIndex + indexData[k + 2u];

            if ((k % 2u) == 0u) // Even triangle: `(V_k, V_{k+1}, V_{k+2})`
            {
                *dstIndices++ = vGlobalK0;
                *dstIndices++ = vGlobalK1;
                *dstIndices++ = vGlobalK2;
            }
            else // Odd triangle: `(V_{k+2}, V_{k+1}, V_k)` to maintain winding, consistent with `appendTriangleStripIndices`
            {
                *dstIndices++ = vGlobalK2;
                *dstIndices++ = vGlobalK1;
                *dstIndices++ = vGlobalK0;
            }
        }
    }
    else
    {
        SFML_BASE_ASSERT(type == PrimitiveType::TriangleFan);

        const IndexType vCenterGlobal = firstNewVertexIndex + indexData[0];

        for (IndexType k = 0u; k < numTrianglesInStripOrFan; ++k) // `k` is the triangle index within the fan
        {
            // Triangles are `(Center, V_{k+1}, V_{k+2})`
            const IndexType vGlobalK1 = firstNewVertexIndex + indexData[k + 1u];
            const IndexType vGlobalK2 = firstNewVertexIndex + indexData[k + 2u];

            *dstIndices++ = vCenterGlobal;
            *dstIndices++ = vGlobalK1;
            *dstIndices++ = vGlobalK2;
        }
    }

    m_storage.commitMoreIndices(numIndicesToGenerate);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::addTextImpl(const auto& text)
{
    const auto [data, size] = text.getVertices();
    SFML_BASE_ASSERT(size % 4u == 0);

    const auto numQuads = static_cast<IndexType>(size / 4u);

    DrawableBatchUtils::appendTextIndicesAndVertices(text.getTransform(),
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
void DrawableBatchImpl<TStorage>::add(const Text& text)
{
    addTextImpl(text);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const GlyphMappedText& text)
{
    addTextImpl(text);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
void DrawableBatchImpl<TStorage>::add(const Sprite& sprite)
{
    DrawableBatchUtils::appendSpriteIndicesAndVertices(sprite,
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

    DrawableBatchUtils::appendShapeFillIndicesAndVertices(transform,
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

    DrawableBatchUtils::appendShapeOutlineIndicesAndVertices(transform,
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
BatchedGeometry DrawableBatchImpl<TStorage>::drawTriangleFanShapeFromPoints(
    const base::SizeT  nPoints,
    const auto&        descriptor,
    auto&&             pointFn,
    const Vec2f* const localApex)
{
    if (nPoints < 3u) [[unlikely]]
        return {};

    const auto transform = Transform::fromPositionScaleOriginRotation(descriptor.position,
                                                                      descriptor.scale,
                                                                      descriptor.origin,
                                                                      descriptor.rotation);

    // TODO P1: improve, also add to RenderTarget

    const base::SizeT fillVertexCount = nPoints + 2u;                  // +2 for center and repeated first point
    const IndexType firstFillVertexIndex = m_storage.getNumVertices(); // index of 1st fill vertex (center of the triangle fan)

    Vertex* const fillVertexPtr = m_storage.reserveMoreVertices(fillVertexCount);
    m_storage.commitMoreVertices(fillVertexCount);

    //
    // Update fill vertex positions and compute inside bounds
    fillVertexPtr[1].position    = transform.transformPoint(pointFn(0u)); // first point
    sf::Vec2f fillBoundsPosition = fillVertexPtr[1].position;             // left and top

    float fillBoundsMaxX = fillVertexPtr[1].position.x; // right
    float fillBoundsMaxY = fillVertexPtr[1].position.y; // bottom

    for (unsigned int i = 1u; i < nPoints; ++i)
    {
        Vertex& v = fillVertexPtr[1u + i];

        v.position = transform.transformPoint(pointFn(i));

        fillBoundsPosition.x = SFML_BASE_MIN(fillBoundsPosition.x, v.position.x);
        fillBoundsPosition.y = SFML_BASE_MIN(fillBoundsPosition.y, v.position.y);

        fillBoundsMaxX = SFML_BASE_MAX(fillBoundsMaxX, v.position.x);
        fillBoundsMaxY = SFML_BASE_MAX(fillBoundsMaxY, v.position.y);
    }

    const sf::Vec2f fillBoundsSize{fillBoundsMaxX - fillBoundsPosition.x, fillBoundsMaxY - fillBoundsPosition.y};

    // Fan apex: if the caller supplied a local apex (needed for non-convex shapes whose bbox
    // center may lie outside the polygon), transform it alongside the geometry; otherwise fall
    // back to the world-space bbox center (valid for convex/centrally-symmetric shapes).
    fillVertexPtr[0].position            = (localApex != nullptr) ? transform.transformPoint(*localApex)
                                                                  : fillBoundsPosition + fillBoundsSize / 2.f;
    fillVertexPtr[1u + nPoints].position = fillVertexPtr[1].position; // repeated first point

    //
    // Update fill color and tex coords (if the shape's fill is visible)
    if (fillBoundsSize.x > 0.f && fillBoundsSize.y > 0.f) [[likely]]
    {
        const Vertex* end = fillVertexPtr + fillVertexCount;
        for (Vertex* vertex = fillVertexPtr; vertex != end; ++vertex)
        {
            vertex->color = descriptor.fillColor;

            const Vec2f ratio = (vertex->position - fillBoundsPosition).componentWiseDiv(fillBoundsSize);
            vertex->texCoords = descriptor.textureRect.position + descriptor.textureRect.size.componentWiseMul(ratio);
        }
    }

    const base::SizeT fillIndexCount = 3u * (fillVertexCount - 2u);

    IndexType* indexPtr = m_storage.reserveMoreIndices(fillIndexCount);

    for (IndexType i = 1u; i < fillVertexCount - 1u; ++i)
        DrawableBatchUtils::appendTriangleFanIndices(indexPtr, firstFillVertexIndex, i);

    m_storage.commitMoreIndices(fillIndexCount);

    //
    // Update outline if needed
    if (descriptor.outlineThickness == 0.f)
        return {.fill = {fillVertexPtr, fillVertexCount}, .outline = {}};

    const base::SizeT outlineVertexCount = (nPoints + 1u) * 2u;

    const IndexType firstOutlineVertexIndex = m_storage.getNumVertices();

    Vertex* const outlineVertexPtr = m_storage.reserveMoreVertices(outlineVertexCount);
    m_storage.commitMoreVertices(outlineVertexCount);

    // Cannot use `vertices` here as the earlier reserve may have invalidated the pointer
    ShapeUtils::updateOutlineFromTriangleFanFill(descriptor.outlineThickness,
                                                 outlineVertexPtr - fillVertexCount + 1u, // Skip the first vertex (center point)
                                                 outlineVertexPtr,
                                                 nPoints,
                                                 descriptor.miterLimit);

    //
    // Update outline colors and outline tex coords
    updateOutlineVerticesColorAndTextureRect(descriptor, outlineVertexPtr, outlineVertexCount);

    const base::SizeT outlineIndexCount = 3u * (outlineVertexCount - 2u);

    auto* outlineIndexPtr = m_storage.reserveMoreIndices(outlineIndexCount);

    for (IndexType i = 0u; i < outlineVertexCount - 2; ++i)
        DrawableBatchUtils::appendTriangleIndices(outlineIndexPtr, firstOutlineVertexIndex + i);

    m_storage.commitMoreIndices(outlineIndexCount);

    return {
        .fill    = {outlineVertexPtr - fillVertexCount, fillVertexCount},
        .outline = {outlineVertexPtr, outlineVertexCount},
    };
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const ArrowShapeData& sdArrow)
{
    if (!sdArrow.hasVisibleGeometry()) [[unlikely]]
        return {};

    // Fan apex in LOCAL coords: on the arrow's axis of symmetry, inside whichever region
    // (head triangle or shaft rectangle) is wide enough to see every perimeter vertex.
    // The arrow is non-convex (barbs at v1/v5), so only one of the two regions is a valid
    // star-shaped kernel -- pick based on which of head/shaft is wider.
    const Vec2f localApex = sdArrow.headWidth > sdArrow.shaftWidth
                                ? Vec2f{sdArrow.shaftLength + sdArrow.headLength / 3.f, 0.f}
                                : Vec2f{sdArrow.shaftLength * 0.5f, 0.f};

    return drawTriangleFanShapeFromPoints(7u,
                                          sdArrow,
                                          [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        return ShapeUtils::computeArrowPoint(i, sdArrow.shaftLength, sdArrow.shaftWidth, sdArrow.headLength, sdArrow.headWidth);
    },
                                          &localApex);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const CircleShapeData& sdCircle)
{
    if (!sdCircle.hasVisibleGeometry()) [[unlikely]]
        return {};

    const float angleStep = base::tau / static_cast<float>(sdCircle.pointCount);

    return drawTriangleFanShapeFromPoints(sdCircle.pointCount,
                                          sdCircle,
                                          [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        return ShapeUtils::computeCirclePointFromAngleStep(i, sdCircle.startAngle.asRadians(), angleStep, sdCircle.radius);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const CurvedArrowShapeData& sd)
{
    if (!sd.hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto adjustedOrigin = sd.origin - Vec2f{sd.outerRadius, sd.outerRadius};

    const auto transform = Transform::fromPositionScaleOriginRotation(sd.position, sd.scale, adjustedOrigin, sd.rotation);

    const float sweepAngleRadians    = sd.sweepAngle.asRadians();
    const float absSweepAngleRadians = base::fabs(sweepAngleRadians);
    const float sweepSign            = (sweepAngleRadians < 0.f) ? -1.f : 1.f;

    // Adjust `pointCount` based on sweep angle for consistent smoothness
    const unsigned int numArcPoints = base::max(2u,
                                                static_cast<unsigned int>(SFML_BASE_MATH_CEILF(
                                                    static_cast<float>(sd.pointCount) *
                                                    (absSweepAngleRadians / sf::base::halfPi))));

    // Reserve memory upfront to avoid pointer invalidation
    const base::SizeT bodyFillVertexCount = 2u * numArcPoints;

    const base::SizeT outlinePerimeterPointCount = numArcPoints + 3u + numArcPoints;

    const base::SizeT outlineVertexCount = sd.outlineThickness != 0.f
                                               ? (outlinePerimeterPointCount + 1u) * 2u // For closed triangle strip
                                               : 0u;

    Vertex* const reservedVertexPtr = m_storage.reserveMoreVertices(bodyFillVertexCount + 3u + outlineVertexCount);

    // Body fill vertices and indices
    const IndexType firstBodyFillVertexIndex = m_storage.getNumVertices();
    Vertex* const   bodyFillVertexPtr        = reservedVertexPtr;

    // Using a bounding box around the center of the curve for simplicity in texcoords for the body part
    // This is consistent with how `generateRingVertices` might calculate them for a full ring
    const Vec2f localBoundsSize    = {2.f * sd.outerRadius, 2.f * sd.outerRadius};
    const Vec2f invLocalBoundsSize = {1.f / localBoundsSize.x, 1.f / localBoundsSize.y};

    const float angleStep    = sweepAngleRadians / static_cast<float>(numArcPoints - 1u);
    const float startRadians = sd.startAngle.asRadians();

    generateRingVertices(sd.textureRect, sd.fillColor, sd.outerRadius, sd.innerRadius, [](const Vec2f p) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
        return p;
    }, numArcPoints, startRadians, angleStep, invLocalBoundsSize, bodyFillVertexPtr);

    m_storage.commitMoreVertices(bodyFillVertexCount);
    const Vec2f ringNaturalCenter = {sd.outerRadius, sd.outerRadius};
    Transform   correctionTransform;
    correctionTransform.translate(-ringNaturalCenter); // Shift so the arc center is at (0,0)

    for (base::SizeT i = 0; i < bodyFillVertexCount; ++i)
    {
        bodyFillVertexPtr[i].position = transform.transformPoint(
            correctionTransform.transformPoint(bodyFillVertexPtr[i].position));
        // Texture coords from generateRingVertices are assumed to be fine as they are relative.
    }

    const base::SizeT numBodySegments    = numArcPoints - 1u;
    const base::SizeT bodyFillIndexCount = numBodySegments * 6u; // 2 triangles per segment, 3 indices per triangle

    IndexType* bodyFillIndexPtr = m_storage.reserveMoreIndices(bodyFillIndexCount);
    for (IndexType i = 0u; i < numBodySegments; ++i)
    {
        const IndexType idxOuterCurr = firstBodyFillVertexIndex + 2u * i + 0u;
        const IndexType idxInnerCurr = firstBodyFillVertexIndex + 2u * i + 1u;
        const IndexType idxOuterNext = firstBodyFillVertexIndex + 2u * (i + 1u) + 0u;
        const IndexType idxInnerNext = firstBodyFillVertexIndex + 2u * (i + 1u) + 1u;

        // Triangle 1 of segment
        *bodyFillIndexPtr++ = idxOuterCurr;
        *bodyFillIndexPtr++ = idxInnerCurr;
        *bodyFillIndexPtr++ = idxOuterNext;

        // Triangle 2 of segment
        *bodyFillIndexPtr++ = idxInnerCurr;
        *bodyFillIndexPtr++ = idxInnerNext;
        *bodyFillIndexPtr++ = idxOuterNext;
    }

    m_storage.commitMoreIndices(bodyFillIndexCount);

    // Head fill vertices and indices
    const IndexType firstHeadFillVertexIndex = m_storage.getNumVertices();
    Vertex* const   headFillVertexPtr        = reservedVertexPtr + bodyFillVertexCount; // After body

    const float endAngleRad = base::positiveRemainder(startRadians + static_cast<float>(numArcPoints - 1u) * angleStep,
                                                      base::tau);
    const auto [endAngleRadSin, endAngleRadCos] = base::sinCosLookup(endAngleRad);

    // Centerline point at the end of the body's curve (local, untransformed)
    // This uses the ringLocalCenter from `computeRingPointsFromAngleStep` as the reference.
    // Center used by `computeRingPointsFromAngleStep` for its calculations
    const float pathRadius = sd.innerRadius + (sd.outerRadius - sd.innerRadius) * 0.5f;

    const Vec2f ringLocalCenter = {sd.outerRadius, sd.outerRadius};
    const Vec2f headAttachPointLocal = ringLocalCenter + Vec2f{pathRadius * endAngleRadCos, pathRadius * endAngleRadSin};

    // Tangent direction at the end of the curve (unit vector)
    // For angle 'a', tangent is (-sin a, cos a) for CCW.
    // sweepSign ensures tip points 'forward' along directed curve.
    const Vec2f tangentDir = {-endAngleRadSin, endAngleRadCos};

    // Radial direction outwards from the curve's center of curvature at endAngleRad (unit vector)
    const Vec2f radialOutDir = {endAngleRadCos, endAngleRadSin};

    // Assign head vertices with transformation
    headFillVertexPtr[0] = {
        // Tip
        .position = transform.transformPoint(
            correctionTransform.transformPoint(headAttachPointLocal + tangentDir * (sd.headLength * sweepSign))),
        .color     = sd.fillColor,
        .texCoords = sd.textureRect.position + sd.textureRect.size.componentWiseMul({0.5f, 1.f}) // Example: Mid-top
    };

    headFillVertexPtr[1] = {
        // Outer Barb
        .position = transform.transformPoint(
            correctionTransform.transformPoint(headAttachPointLocal + radialOutDir * (sd.headWidth / 2.f))),
        .color     = sd.fillColor,
        .texCoords = sd.textureRect.position + sd.textureRect.size.componentWiseMul({0.f, 0.f}) // Example: Top-left
    };

    headFillVertexPtr[2] = {
        // Inner Barb
        .position = transform.transformPoint(
            correctionTransform.transformPoint(headAttachPointLocal - radialOutDir * (sd.headWidth / 2.f))),
        .color     = sd.fillColor,
        .texCoords = sd.textureRect.position + sd.textureRect.size.componentWiseMul({1.f, 0.f}) // Example: Top-right
    };

    m_storage.commitMoreVertices(3u);

    // Indices for head (3 triangles connecting body end to head points)
    // These indices refer to already transformed vertices in the buffer
    const IndexType endBodyOuterIdx = firstBodyFillVertexIndex + 2u * (numArcPoints - 1u) + 0u;
    const IndexType endBodyInnerIdx = firstBodyFillVertexIndex + 2u * (numArcPoints - 1u) + 1u;

    const IndexType headTipIdx       = firstHeadFillVertexIndex + 0u;
    const IndexType headOuterBarbIdx = firstHeadFillVertexIndex + 1u;
    const IndexType headInnerBarbIdx = firstHeadFillVertexIndex + 2u;

    IndexType* headFillIndexPtr = m_storage.reserveMoreIndices(9u); // 3 triangles

    // Stitch end of body to base of arrowhead
    // Winding should be consistent (e.g., CCW)
    // Triangle 1: (Outer side of body end, Inner side of body end, Outer barb)
    *headFillIndexPtr++ = endBodyOuterIdx;
    *headFillIndexPtr++ = endBodyInnerIdx;
    *headFillIndexPtr++ = headOuterBarbIdx;

    // Triangle 2: (Inner side of body end, Inner barb, Outer barb)
    *headFillIndexPtr++ = endBodyInnerIdx;
    *headFillIndexPtr++ = headInnerBarbIdx;
    *headFillIndexPtr++ = headOuterBarbIdx;

    // Triangle 3: (Outer barb, Inner barb, Tip)
    *headFillIndexPtr++ = headOuterBarbIdx;
    *headFillIndexPtr++ = headInnerBarbIdx;
    *headFillIndexPtr++ = headTipIdx;
    m_storage.commitMoreIndices(9u);

    // Outline generation
    if (sd.outlineThickness != 0.f)
    {
        Vertex* const outlineVertexPtr = reservedVertexPtr + bodyFillVertexCount + 3u; // After body and head fill vertices

        const IndexType firstOutlineVertexIndex = m_storage.getNumVertices();

        // Walk the perimeter CW-visually in screen space (y-down) so that outline normals point
        // inward into the curved-arrow body. This makes `outlineThickness > 0` draw on top of
        // the fill rather than growing the bounds.
        //
        // Order: outer arc (start -> end) -> head (outer barb, tip, inner barb) -> inner arc (end -> start).
        ShapeUtils::updateOutlineImpl(sd.outlineThickness,
                                      [&](const base::SizeT i)
        {
            // 1. Outer edge of the body (start to end).
            if (i < numArcPoints)
                return bodyFillVertexPtr[2 * i + 0u].position;

            // 2. Arrowhead vertices: outer barb (1) -> tip (0) -> inner barb (2).
            if (i < numArcPoints + 3u)
            {
                const unsigned int headLocal[3] = {1u, 0u, 2u};
                return headFillVertexPtr[headLocal[i - numArcPoints]].position;
            }

            // 3. Inner edge of the body (end back to start).
            const auto innerIdx = 2u * numArcPoints + 2u - static_cast<base::SizeT>(i);
            return bodyFillVertexPtr[2 * innerIdx + 1u].position;
        },
                                      outlineVertexPtr,
                                      outlinePerimeterPointCount,
                                      sd.miterLimit);

        updateOutlineVerticesColorAndTextureRect(sd, outlineVertexPtr, outlineVertexCount);
        m_storage.commitMoreVertices(outlineVertexCount);

        const base::SizeT outlineIndexCount = 3u * (outlineVertexCount - 2u); // Triangle strip indices
        IndexType*        outlineIndexPtr   = m_storage.reserveMoreIndices(outlineIndexCount);

        for (IndexType i = 0u; i < outlineVertexCount - 2u; ++i)
            DrawableBatchUtils::appendTriangleStripIndices(outlineIndexPtr, firstOutlineVertexIndex, i);

        m_storage.commitMoreIndices(outlineIndexCount);

        // Split body + head (fill) from the outline region
        return {
            .fill    = {bodyFillVertexPtr, bodyFillVertexCount + 3u},
            .outline = {outlineVertexPtr, outlineVertexCount},
        };
    }

    return {
        .fill    = {bodyFillVertexPtr, bodyFillVertexCount + 3u},
        .outline = {},
    };
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const EllipseShapeData& sdEllipse)
{
    if (!sdEllipse.hasVisibleGeometry()) [[unlikely]]
        return {};

    const float angleStep = base::tau / static_cast<float>(sdEllipse.pointCount);

    return drawTriangleFanShapeFromPoints(sdEllipse.pointCount,
                                          sdEllipse,
                                          [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        return ShapeUtils::computeEllipsePointFromAngleStep(i,
                                                            sdEllipse.startAngle.asRadians(),
                                                            angleStep,
                                                            sdEllipse.horizontalRadius,
                                                            sdEllipse.verticalRadius);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const PieSliceShapeData& sdPieSlice)
{
    if (!sdPieSlice.hasVisibleGeometry()) [[unlikely]]
        return {};

    if (SFML_BASE_MATH_FABSF(sdPieSlice.sweepAngle.asRadians()) >= base::tau - SFML_BASE_FLOAT_EPSILON)
        return add(CircleShapeData{
            .position           = sdPieSlice.position,
            .scale              = sdPieSlice.scale,
            .origin             = sdPieSlice.origin,
            .rotation           = sdPieSlice.rotation,
            .textureRect        = sdPieSlice.textureRect,
            .outlineTextureRect = sdPieSlice.outlineTextureRect,
            .fillColor          = sdPieSlice.fillColor,
            .outlineColor       = sdPieSlice.outlineColor,
            .outlineThickness   = sdPieSlice.outlineThickness,
            .radius             = sdPieSlice.radius,
            .startAngle         = sdPieSlice.startAngle,
            .pointCount         = sdPieSlice.pointCount - 2u,
        });

    const float arcAngleStep = ShapeUtils::computePieSliceArcAngleStep(sdPieSlice.sweepAngle.asRadians(), sdPieSlice.pointCount);

    return drawTriangleFanShapeFromPoints(sdPieSlice.pointCount,
                                          sdPieSlice,
                                          [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        return ShapeUtils::computePieSlicePointFromArcAngleStep(i,
                                                                sdPieSlice.radius,
                                                                arcAngleStep,
                                                                sdPieSlice.startAngle.asRadians());
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const RectangleShapeData& sdRectangle)
{
    if (!sdRectangle.hasVisibleGeometry()) [[unlikely]]
        return {};

    return drawTriangleFanShapeFromPoints(4u, sdRectangle, [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
        return ShapeUtils::computeRectanglePoint(i, sdRectangle.size);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const RoundedRectangleShapeData& sdRoundedRectangle)
{
    if (!sdRoundedRectangle.hasVisibleGeometry()) [[unlikely]]
        return {};

    return drawTriangleFanShapeFromPoints(sdRoundedRectangle.cornerPointCount * 4u,
                                          sdRoundedRectangle,
                                          [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        return ShapeUtils::computeRoundedRectanglePoint(i,
                                                        sdRoundedRectangle.size,
                                                        sdRoundedRectangle.cornerRadius,
                                                        sdRoundedRectangle.cornerPointCount);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const RingShapeData& sdRing)
{
    if (!sdRing.hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto
        transform = Transform::fromPositionScaleOriginRotation(sdRing.position, sdRing.scale, sdRing.origin, sdRing.rotation);

    const unsigned int nPoints   = sdRing.pointCount;
    const float        angleStep = base::tau / static_cast<float>(nPoints);

    //
    // Local origin `(0, 0)` is top-left of the bounding box
    // Bounding box size is `(2 * outerRadius, 2 * outerRadius)`
    // Geometric center within local coords is `(outerRadius, outerRadius)`
    const Vec2f localBoundsSize    = {2.f * sdRing.outerRadius, 2.f * sdRing.outerRadius};
    const Vec2f invLocalBoundsSize = {1.f / localBoundsSize.x, 1.f / localBoundsSize.y};

    //
    // Generate fill geometry (triangle strip)
    const base::SizeT fillVertexCount = 2u * nPoints + 2u; // `nPoints` pairs + repeated start pair

    const base::SizeT outlineVerticesPerLoop = nPoints * 2u + 2u; // nPoints original points -> nPoints inner/outer
                                                                  // pairs + duplicated start pair for closed loop
    const base::SizeT totalOutlineVertices = sdRing.outlineThickness != 0.f ? outlineVerticesPerLoop * 2u
                                                                            : 0u; // Outer + Inner loop

    const IndexType firstFillVertexIndex = m_storage.getNumVertices();
    Vertex* const   reservedVertexPtr    = m_storage.reserveMoreVertices(fillVertexCount + totalOutlineVertices);
    Vertex* const   fillVertexPtr        = reservedVertexPtr;

    generateRingVertices(sdRing.textureRect, sdRing.fillColor, sdRing.outerRadius, sdRing.innerRadius, [&](const Vec2f p) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
        return transform.transformPoint(p);
    }, nPoints, sdRing.startAngle.asRadians(), angleStep, invLocalBoundsSize, fillVertexPtr);

    //
    // Repeat first pair to close the strip
    fillVertexPtr[2 * nPoints + 0] = fillVertexPtr[0];
    fillVertexPtr[2 * nPoints + 1] = fillVertexPtr[1];

    m_storage.commitMoreVertices(fillVertexCount);

    //
    // Generate fill indices
    {
        const base::SizeT numFillTriangles = (fillVertexCount - 2u); // A strip of `V` vertices has `V - 2` triangles
        const base::SizeT fillIndexCount   = numFillTriangles * 3u;

        IndexType* fillIndexPtr = m_storage.reserveMoreIndices(fillIndexCount);
        for (IndexType i = 0u; i < numFillTriangles; ++i)
            DrawableBatchUtils::appendTriangleStripIndices(fillIndexPtr, firstFillVertexIndex, i);

        m_storage.commitMoreIndices(fillIndexCount);
    }

    //
    // Update outline if needed
    if (sdRing.outlineThickness == 0.f)
        return {.fill = {fillVertexPtr, fillVertexCount}, .outline = {}};

    const base::SizeT outlineIndicesPerLoop = 3u * (outlineVerticesPerLoop - 2u); // Indices for triangles from the strip
    const base::SizeT totalOutlineIndices = outlineIndicesPerLoop * 2u;

    const IndexType  firstOutlineVertexIndex = m_storage.getNumVertices();
    Vertex* const    outlineVertexPtr        = reservedVertexPtr + fillVertexCount;
    IndexType* const outlineIndexPtr         = m_storage.reserveMoreIndices(totalOutlineIndices);

    const IndexType firstOuterOutlineLoopVertexIndex = firstOutlineVertexIndex;
    const IndexType firstInnerOutlineLoopVertexIndex = firstOutlineVertexIndex +
                                                       static_cast<IndexType>(outlineVerticesPerLoop);

    const auto generateOutlineHelper =
        [&](Vertex* const      chosenOutlineVertexPtr,
            IndexType* const   chosenOutlineIndexPtr,
            const IndexType    firstChonsenOutlineLoopVertexIndex,
            const unsigned int fillVertexIndexOffset,
            const bool         reverseWalk)
    {
        // Generate outline vertices.
        // The inner outline walks the inner ring in reverse so its normals point away from the
        // hole (into the annulus), matching the outer outline's inward normals. This keeps
        // `outlineThickness > 0` drawing on top of the fill for both loops.
        const auto getBoundaryPoint = [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
        {
            SFML_BASE_ASSERT_AND_ASSUME(i < nPoints);
            const base::SizeT walked = reverseWalk ? (nPoints - 1u - i) : i;
            return fillVertexPtr[2 * walked + fillVertexIndexOffset].position;
        };

        Vertex* const chosenOutlineVertexStart = chosenOutlineVertexPtr;
        ShapeUtils::updateOutlineImpl(sdRing.outlineThickness,
                                      getBoundaryPoint,
                                      chosenOutlineVertexStart,
                                      nPoints,
                                      sdRing.miterLimit);

        // Generate outline indices
        IndexType* chosenOutlineIndexPtrStart = chosenOutlineIndexPtr;
        for (IndexType i = 0u; i < outlineVerticesPerLoop - 2u; ++i)
            DrawableBatchUtils::appendTriangleIndices(chosenOutlineIndexPtrStart, firstChonsenOutlineLoopVertexIndex + i);
    };

    //
    // Generate outer outline (walked forward: CW-visual, normals point inward into annulus)
    generateOutlineHelper(outlineVertexPtr, outlineIndexPtr, firstOuterOutlineLoopVertexIndex, 0, /* reverseWalk */ false);

    //
    // Generate inner outline (walked backward: CCW-visual, normals point outward from hole into annulus)
    generateOutlineHelper(outlineVertexPtr + outlineVerticesPerLoop,
                          outlineIndexPtr + outlineIndicesPerLoop,
                          firstInnerOutlineLoopVertexIndex,
                          1,
                          /* reverseWalk */ true);

    //
    // Update outline colors and outline tex coords
    updateOutlineVerticesColorAndTextureRect(sdRing, outlineVertexPtr, totalOutlineVertices);

    m_storage.commitMoreVertices(totalOutlineVertices);
    m_storage.commitMoreIndices(totalOutlineIndices);

    return {
        .fill    = {outlineVertexPtr - fillVertexCount, fillVertexCount},
        .outline = {outlineVertexPtr, totalOutlineVertices},
    };
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const RingPieSliceShapeData& sdRingPieSlice)
{
    if (!sdRingPieSlice.hasVisibleGeometry()) [[unlikely]]
        return {};

    if (SFML_BASE_MATH_FABSF(sdRingPieSlice.sweepAngle.asRadians()) >= base::tau - SFML_BASE_FLOAT_EPSILON)
        return add(RingShapeData{
            .position           = sdRingPieSlice.position,
            .scale              = sdRingPieSlice.scale,
            .origin             = sdRingPieSlice.origin,
            .rotation           = sdRingPieSlice.rotation,
            .textureRect        = sdRingPieSlice.textureRect,
            .outlineTextureRect = sdRingPieSlice.outlineTextureRect,
            .fillColor          = sdRingPieSlice.fillColor,
            .outlineColor       = sdRingPieSlice.outlineColor,
            .outlineThickness   = sdRingPieSlice.outlineThickness,
            .outerRadius        = sdRingPieSlice.outerRadius,
            .innerRadius        = sdRingPieSlice.innerRadius,
            .startAngle         = sdRingPieSlice.startAngle,
            .pointCount         = sdRingPieSlice.pointCount - 1u,
        });

    const auto transform = Transform::fromPositionScaleOriginRotation(sdRingPieSlice.position,
                                                                      sdRingPieSlice.scale,
                                                                      sdRingPieSlice.origin,
                                                                      sdRingPieSlice.rotation);

    const float absSweepAngle = SFML_BASE_MATH_FABSF(sdRingPieSlice.sweepAngle.asRadians());
    const float sweepRadians  = sdRingPieSlice.sweepAngle.asRadians();
    const float startRadians  = sdRingPieSlice.startAngle.asRadians();

    const unsigned int numArcPoints = base::max(3u,
                                                static_cast<unsigned int>(SFML_BASE_MATH_CEILF(
                                                    static_cast<float>(sdRingPieSlice.pointCount) *
                                                    (absSweepAngle / base::tau))));

    const float angleStep = sweepRadians / static_cast<float>(numArcPoints - 1u);

    //
    // Local origin `(0, 0)` is top-left of the bounding box
    // Bounding box size is `(2 * outerRadius, 2 * outerRadius)`
    // Geometric center within local coords is `(outerRadius, outerRadius)`
    const Vec2f localBoundsSize    = {2.f * sdRingPieSlice.outerRadius, 2.f * sdRingPieSlice.outerRadius};
    const Vec2f invLocalBoundsSize = {1.f / localBoundsSize.x, 1.f / localBoundsSize.y};

    //
    // Generate fill geometry (triangle strip)
    const base::SizeT fillVertexCount = 2u * numArcPoints;

    const base::SizeT numBoundaryPoints    = 2u * numArcPoints;
    const base::SizeT totalOutlineVertices = (sdRingPieSlice.outlineThickness != 0.f && numArcPoints >= 3u)
                                                 ? (numBoundaryPoints + 1u) * 2u
                                                 : 0u;

    const IndexType firstFillVertexIndex = m_storage.getNumVertices();
    Vertex* const   reservedVertexPtr    = m_storage.reserveMoreVertices(fillVertexCount + totalOutlineVertices);
    Vertex* const   fillVertexPtr        = reservedVertexPtr;

    generateRingVertices(sdRingPieSlice.textureRect,
                         sdRingPieSlice.fillColor,
                         sdRingPieSlice.outerRadius,
                         sdRingPieSlice.innerRadius,
                         [&](const Vec2f p) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return transform.transformPoint(p); },
                         numArcPoints,
                         startRadians,
                         angleStep,
                         invLocalBoundsSize,
                         fillVertexPtr);

    m_storage.commitMoreVertices(fillVertexCount);

    //
    // Generate fill indices
    if (numArcPoints >= 3)
    {
        const base::SizeT numFillTriangles = (fillVertexCount - 2u); // A strip of `V` vertices has `V - 2` triangles
        const base::SizeT fillIndexCount   = numFillTriangles * 3u;

        IndexType* fillIndexPtr = m_storage.reserveMoreIndices(fillIndexCount);
        for (IndexType i = 0u; i < numFillTriangles; ++i)
            DrawableBatchUtils::appendTriangleStripIndices(fillIndexPtr, firstFillVertexIndex, i);

        m_storage.commitMoreIndices(fillIndexCount);
    }

    //
    // Update outline if needed
    if (sdRingPieSlice.outlineThickness == 0.f || numArcPoints < 3)
        return {.fill = {fillVertexPtr, fillVertexCount}, .outline = {}};

    const base::SizeT numOutlineTriangles = totalOutlineVertices - 2u;
    const base::SizeT totalOutlineIndices = numOutlineTriangles * 3u;

    const IndexType firstOutlineVertexIndex = m_storage.getNumVertices();
    Vertex* const   outlineVertexPtr        = reservedVertexPtr + fillVertexCount;

    //
    // Generate outline vertices
    const auto getBoundaryPoint = [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        SFML_BASE_ASSERT_AND_ASSUME(i < numBoundaryPoints);
        const auto fillVertexIdx = i < numArcPoints ? 2 * i : 2 * (2 * numArcPoints - 1 - i) + 1;
        return fillVertexPtr[fillVertexIdx].position;
    };

    ShapeUtils::updateOutlineImpl(sdRingPieSlice.outlineThickness,
                                  getBoundaryPoint,  // Lambda providing positions
                                  outlineVertexPtr,  // Output vertex buffer
                                  numBoundaryPoints, // Number of unique points in the loop
                                  sdRingPieSlice.miterLimit);

    //
    // Generate outline indices
    IndexType* outlineIndexPtr = m_storage.reserveMoreIndices(totalOutlineIndices);
    for (IndexType i = 0u; i < numOutlineTriangles; ++i)
        DrawableBatchUtils::appendTriangleStripIndices(outlineIndexPtr, firstOutlineVertexIndex, i);

    //
    // Update outline colors and outline tex coords
    updateOutlineVerticesColorAndTextureRect(sdRingPieSlice, outlineVertexPtr, totalOutlineVertices);

    m_storage.commitMoreVertices(totalOutlineVertices);
    m_storage.commitMoreIndices(totalOutlineIndices);

    return {
        .fill    = {outlineVertexPtr - fillVertexCount, fillVertexCount},
        .outline = {outlineVertexPtr, totalOutlineVertices},
    };
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const StarShapeData& sdStar)
{
    if (!sdStar.hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto nPoints = sdStar.pointCount * 2u;

    SFML_BASE_ASSERT(nPoints != 0u);
    const float angleStep = base::tau / static_cast<float>(nPoints);

    return drawTriangleFanShapeFromPoints(nPoints, sdStar, [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
        return ShapeUtils::computeStarPointFromAngleStep(i, angleStep, sdStar.outerRadius, sdStar.innerRadius);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const CrossShapeData& sdCross)
{
    if (!sdCross.hasVisibleGeometry()) [[unlikely]]
        return {};

    return drawTriangleFanShapeFromPoints(12u, sdCross, [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
        return ShapeUtils::computeCrossPoint(i, sdCross.size, sdCross.armThickness);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const TrapezoidShapeData& sdTrapezoid)
{
    if (!sdTrapezoid.hasVisibleGeometry()) [[unlikely]]
        return {};

    return drawTriangleFanShapeFromPoints(4u, sdTrapezoid, [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
        return ShapeUtils::computeTrapezoidPoint(i, sdTrapezoid.topWidth, sdTrapezoid.bottomWidth, sdTrapezoid.height);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const ChevronShapeData& sdChevron)
{
    if (!sdChevron.hasVisibleGeometry()) [[unlikely]]
        return {};

    // `computeChevronPoint` clamps `thickness` to `size.y / 2` internally (so the shape cleanly
    // degenerates to a filled triangle when the inner vertices would cross), but we need the
    // same clamped value here to keep the fan apex on the chevron's axis of symmetry.
    const float w = sdChevron.size.x;
    const float h = sdChevron.size.y;
    const float t = SFML_BASE_MIN(sdChevron.thickness, h * 0.5f);

    const float innerTipX = w * (1.f - 2.f * t / h);

    // Fan apex in LOCAL coords: midway between inner and outer tip, on the chevron's axis of
    // symmetry. Inside the non-convex chevron for every valid `thickness`.
    const Vec2f localApex{(w + innerTipX) * 0.5f, h * 0.5f};

    return drawTriangleFanShapeFromPoints(6u, sdChevron, [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
        return ShapeUtils::computeChevronPoint(i, sdChevron.size, sdChevron.thickness);
    }, &localApex);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const HeartShapeData& sdHeart)
{
    if (!sdHeart.hasVisibleGeometry()) [[unlikely]]
        return {};

    return drawTriangleFanShapeFromPoints(sdHeart.pointCount,
                                          sdHeart,
                                          [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return ShapeUtils::computeHeartPoint(i, sdHeart.pointCount, sdHeart.size); });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const CogShapeData& sdCog)
{
    if (!sdCog.hasVisibleGeometry()) [[unlikely]]
        return {};

    return drawTriangleFanShapeFromPoints(4u * sdCog.toothCount,
                                          sdCog,
                                          [&](const base::SizeT i) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    {
        return ShapeUtils::computeCogPoint(i, sdCog.toothCount, sdCog.outerRadius, sdCog.innerRadius, sdCog.toothWidthRatio);
    });
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::addTextDataImpl(
    const auto&        glyphSource,
    const auto&        textData,
    const bool         isBold,
    const unsigned int characterSize,
    const float        outlineThickness)
{
    if (textData.string.isEmpty())
        return {};

    const auto fillQuadCount = TextUtils::precomputeTextQuadCount(textData.string, textData.underlined, textData.strikeThrough);
    const auto outlineQuadCount = outlineThickness == 0.f ? 0u : fillQuadCount;

    const auto numQuads = fillQuadCount + outlineQuadCount;

    IndexType* indexPtr  = m_storage.reserveMoreIndices(6u * numQuads);
    const auto nextIndex = m_storage.getNumVertices();

    for (IndexType i = 0u; i < numQuads; ++i)
        DrawableBatchUtils::appendQuadIndices(indexPtr, nextIndex + (i * 4u));

    const auto transform = Transform::fromPositionScaleOriginRotation(textData.position,
                                                                      textData.scale,
                                                                      textData.origin,
                                                                      textData.rotation);

    Vertex* const vertexPtr = m_storage.reserveMoreVertices(4u * numQuads);

    TextUtils::createTextGeometryAndGetBounds<
        /* CalculateBounds */ false>(/* outlineVertexCount */ outlineQuadCount * 4u,
                                     glyphSource,
                                     textData.string,
                                     isBold,
                                     textData.italic,
                                     textData.underlined,
                                     textData.strikeThrough,
                                     characterSize,
                                     textData.letterSpacing,
                                     textData.lineSpacing,
                                     outlineThickness,
                                     textData.fillColor,
                                     textData.outlineColor,
                                     [&](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return TextUtils::addLinePreTransformed(transform, vertexPtr, SFML_BASE_FORWARD(xs)...); },
                                     [&](auto&&... xs) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN
    { return TextUtils::addGlyphQuadPreTransformed(transform, vertexPtr, SFML_BASE_FORWARD(xs)...); });

    m_storage.commitMoreIndices(6u * numQuads);
    m_storage.commitMoreVertices(4u * numQuads);

    // `createTextGeometryAndGetBounds` lays outline quads out first, then fill quads.
    return {
        .fill    = {vertexPtr + outlineQuadCount * 4u, fillQuadCount * 4u},
        .outline = {vertexPtr, outlineQuadCount * 4u},
    };
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const Font& font, const TextData& textData)
{
    return addTextDataImpl(font, textData, textData.bold, textData.characterSize, textData.outlineThickness);
}


////////////////////////////////////////////////////////////
template <typename TStorage>
BatchedGeometry DrawableBatchImpl<TStorage>::add(const FontFace&            fontFace,
                                                 const GlyphMapping&        glyphMapping,
                                                 const GlyphMappedTextData& textData)
{
    return addTextDataImpl(GlyphMappingWithKerning{glyphMapping, fontFace},
                           textData,
                           glyphMapping.bold,
                           glyphMapping.characterSize,
                           glyphMapping.outlineThickness);
}

} // namespace sf::priv
