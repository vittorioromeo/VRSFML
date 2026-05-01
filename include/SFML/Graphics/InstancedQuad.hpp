#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/Vertex.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Unit-quad vertices for instanced rendering
///
/// A [-0.5, 0.5] quad with white color and [0, 1] texture
/// coordinates, suitable as the base mesh for instanced
/// sprite rendering.
///
////////////////////////////////////////////////////////////
inline constexpr Vertex instancedQuadVertices[4] = {
    {{-0.5f, -0.5f}, Color::White, {0.f, 0.f}},
    {{0.5f, -0.5f}, Color::White, {1.f, 0.f}},
    {{0.5f, 0.5f}, Color::White, {1.f, 1.f}},
    {{-0.5f, 0.5f}, Color::White, {0.f, 1.f}},
};


////////////////////////////////////////////////////////////
/// \brief Index buffer for \ref instancedQuadVertices
///
/// Two triangles (0-1-2, 2-3-0) forming the quad.
///
////////////////////////////////////////////////////////////
inline constexpr IndexType instancedQuadIndices[6] = {0u, 1u, 2u, 2u, 3u, 0u};

} // namespace sf
