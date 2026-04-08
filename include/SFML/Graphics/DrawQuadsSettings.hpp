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
/// \brief Parameters for `sf::RenderTarget::drawQuads`
///
/// `vertexSpan` is interpreted as 4 vertices per quad. The renderer
/// uses a precomputed index buffer to expand quads into two
/// triangles each, so the caller does not have to duplicate
/// vertices.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawQuadsSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    base::Span<const Vertex> vertexSpan;    //!< Range of vertices laid out as groups of 4 corners
    PrimitiveType            primitiveType; //!< Primitive type used for the expanded triangles
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::DrawQuadsSettings
/// \ingroup graphics
///
/// `sf::DrawQuadsSettings` is a convenience for drawing groups of
/// 4-vertex quads without having to manually duplicate vertices for
/// the two triangles each quad consists of. The renderer reuses a
/// precomputed quad index buffer to perform the expansion in a
/// single indexed draw call.
///
/// \see `sf::DrawIndexedVerticesSettings`, `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
