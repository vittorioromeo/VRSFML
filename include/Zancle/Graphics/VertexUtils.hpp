#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/AssertAndAssume.hpp"
#include "ZancleBase/MinMaxMacros.hpp"
#include "ZancleBase/SizeT.hpp"


namespace za::VertexUtils
{
////////////////////////////////////////////////////////////
/// \brief Computes the bounding rectangle of a range of vertices.
///
/// This function iterates through a sequence of vertices and determines the
/// smallest axis-aligned rectangle that encloses all the vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return A rect representing the bounding rectangle.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::pure]] inline constexpr Rect2f getVertexRangeBounds(const Vertex* const data, const zb::SizeT nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float left   = data[0].position.x;
    float top    = data[0].position.y;
    float right  = data[0].position.x;
    float bottom = data[0].position.y;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        left   = ZB_MIN(left, v->position.x);
        right  = ZB_MAX(right, v->position.x);
        top    = ZB_MIN(top, v->position.y);
        bottom = ZB_MAX(bottom, v->position.y);
    }

    return {{left, top}, {right - left, bottom - top}};
}

////////////////////////////////////////////////////////////
/// \brief Computes the minimum x-coordinate of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The minimum x-coordinate.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getLeft(const Vertex* const data,
                                                                                          const zb::SizeT nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.x;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = ZB_MIN(result, v->position.x);

    return result;
}

////////////////////////////////////////////////////////////
/// \brief Computes the maximum x-coordinate of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The maximum x-coordinate.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getRight(const Vertex* const data,
                                                                                           const zb::SizeT nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.x;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = ZB_MAX(result, v->position.x);

    return result;
}

////////////////////////////////////////////////////////////
/// \brief Computes the minimum y-coordinate of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The minimum y-coordinate.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getTop(const Vertex* const data,
                                                                                         const zb::SizeT nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.y;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = ZB_MIN(result, v->position.y);

    return result;
}


////////////////////////////////////////////////////////////
/// \brief Computes the maximum y-coordinate of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The maximum y-coordinate.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float getBottom(const Vertex* const data,
                                                                                            const zb::SizeT nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.y;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = ZB_MAX(result, v->position.y);

    return result;
}

////////////////////////////////////////////////////////////
/// \brief Computes the (min x, min y) of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The (min x, min y) of a range of vertices.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getTopLeft(const Vertex* const data,
                                                                                             const zb::SizeT nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = ZB_MIN(result.x, v->position.x);
        result.y = ZB_MIN(result.y, v->position.y);
    }

    return result;
}

////////////////////////////////////////////////////////////
/// \brief Computes the (max x, min y) of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The (max x, min y) of a range of vertices.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getTopRight(
    const Vertex* const data,
    const zb::SizeT   nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = ZB_MAX(result.x, v->position.x);
        result.y = ZB_MIN(result.y, v->position.y);
    }

    return result;
}

////////////////////////////////////////////////////////////
/// \brief Computes the (min x, max y) of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The (min x, max y) of a range of vertices.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getBottomLeft(
    const Vertex* const data,
    const zb::SizeT   nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = ZB_MIN(result.x, v->position.x);
        result.y = ZB_MAX(result.y, v->position.y);
    }

    return result;
}

////////////////////////////////////////////////////////////
/// \brief Computes the (max x, max y) of a range of vertices.
///
/// \param data      Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return The (max x, max y) of a range of vertices.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2f getBottomRight(
    const Vertex* const data,
    const zb::SizeT   nVertices) noexcept
{
    ZB_ASSERT_AND_ASSUME(data != nullptr);
    ZB_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = ZB_MAX(result.x, v->position.x);
        result.y = ZB_MAX(result.y, v->position.y);
    }

    return result;
}

} // namespace za::VertexUtils


////////////////////////////////////////////////////////////
/// \struct za::VertexUtils
/// \ingroup graphics
///
/// Utility functions for working with vertices.
///
/// \see `za::Vertex`
///
////////////////////////////////////////////////////////////
