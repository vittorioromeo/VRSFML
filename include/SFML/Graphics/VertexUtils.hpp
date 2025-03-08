#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Rect.hpp"

#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Computes the bounding rectangle of a range of vertices.
///
/// This function iterates through a sequence of vertices and determines the
/// smallest axis-aligned rectangle that encloses all the vertices.
///
/// \param data Pointer to the first vertex.
/// \param nVertices The number of vertices in the range.
///
/// \return A rect representing the bounding rectangle.
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline constexpr FloatRect getVertexRangeBounds(const Vertex* data, const base::SizeT nVertices) noexcept
{
    SFML_BASE_ASSERT(nVertices > 0u);

    float left   = data[0].position.x;
    float top    = data[0].position.y;
    float right  = data[0].position.x;
    float bottom = data[0].position.y;

    const Vertex* end = data + nVertices;

    for (const Vertex* v = data + 1; v < end; ++v)
    {
        left   = base::min(left, v->position.x);
        right  = base::max(right, v->position.x);
        top    = base::min(top, v->position.y);
        bottom = base::max(bottom, v->position.y);
    }

    return {{left, top}, {right - left, bottom - top}};
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::VertexUtils
/// \ingroup graphics
///
/// Utility functions for working with vertices.
///
/// \see `sf::Vertex`
///
////////////////////////////////////////////////////////////
