#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf::VertexUtils
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
[[nodiscard, gnu::pure]] inline constexpr FloatRect getVertexRangeBounds(const Vertex* const data,
                                                                         const base::SizeT   nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float left   = data[0].position.x;
    float top    = data[0].position.y;
    float right  = data[0].position.x;
    float bottom = data[0].position.y;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        left   = SFML_BASE_MIN(left, v->position.x);
        right  = SFML_BASE_MAX(right, v->position.x);
        top    = SFML_BASE_MIN(top, v->position.y);
        bottom = SFML_BASE_MAX(bottom, v->position.y);
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
                                                                                          const base::SizeT nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.x;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = SFML_BASE_MIN(result, v->position.x);

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
                                                                                           const base::SizeT nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.x;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = SFML_BASE_MAX(result, v->position.x);

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
                                                                                         const base::SizeT nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.y;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = SFML_BASE_MIN(result, v->position.y);

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
                                                                                            const base::SizeT nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    float result = data[0].position.y;

    for (const Vertex* v = data + 1; v < end; ++v)
        result = SFML_BASE_MAX(result, v->position.y);

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
                                                                                             const base::SizeT nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = SFML_BASE_MIN(result.x, v->position.x);
        result.y = SFML_BASE_MIN(result.y, v->position.y);
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
    const base::SizeT   nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = SFML_BASE_MAX(result.x, v->position.x);
        result.y = SFML_BASE_MIN(result.y, v->position.y);
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
    const base::SizeT   nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = SFML_BASE_MIN(result.x, v->position.x);
        result.y = SFML_BASE_MAX(result.y, v->position.y);
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
    const base::SizeT   nVertices) noexcept
{
    SFML_BASE_ASSERT_AND_ASSUME(data != nullptr);
    SFML_BASE_ASSERT_AND_ASSUME(nVertices > 0u);
    const Vertex* const end = data + nVertices;

    Vec2f result = data[0].position;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        result.x = SFML_BASE_MAX(result.x, v->position.x);
        result.y = SFML_BASE_MAX(result.y, v->position.y);
    }

    return result;
}

} // namespace sf::VertexUtils


////////////////////////////////////////////////////////////
/// \struct sf::VertexUtils
/// \ingroup graphics
///
/// Utility functions for working with vertices.
///
/// \see `sf::Vertex`
///
////////////////////////////////////////////////////////////
