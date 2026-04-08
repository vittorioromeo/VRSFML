#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Span.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `sf::RenderTarget::drawVertices`
///
/// Bundles the vertex data and the primitive type used to interpret
/// it. Used by `sf::RenderTarget::drawVertices` and the convenience
/// `sf::RenderTarget::draw` overloads.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    base::Span<const Vertex> vertexSpan;    //!< Range of vertices to draw
    PrimitiveType            primitiveType; //!< How to interpret the vertices (triangles, lines, ...)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::DrawVerticesSettings
/// \ingroup graphics
///
/// `sf::DrawVerticesSettings` is the simplest of the `Draw*Settings`
/// family: it points at a contiguous range of `sf::Vertex` and
/// declares which `sf::PrimitiveType` should be used to interpret
/// the range. It is consumed by `sf::RenderTarget::drawVertices`.
///
/// For indexed, instanced, or persistent-mapped variants, see
/// `sf::DrawIndexedVerticesSettings`, `sf::DrawInstancedVerticesSettings`,
/// `sf::DrawInstancedIndexedVerticesSettings`, and
/// `sf::DrawPersistentMappedIndexedVerticesSettings`.
///
/// \see `sf::RenderTarget`, `sf::Vertex`, `sf::PrimitiveType`
///
////////////////////////////////////////////////////////////
