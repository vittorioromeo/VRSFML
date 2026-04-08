#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Span.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `sf::RenderTarget::drawIndexedVertices`
///
/// Bundles a vertex array, an index array, and the primitive type.
/// Indexed drawing lets you reuse vertices, which is significantly
/// cheaper than duplicating them when many primitives share corners.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    base::Span<const Vertex>    vertexSpan;    //!< Range of unique vertices that will be referenced by the indices
    base::Span<const IndexType> indexSpan;     //!< Range of indices into `vertexSpan` defining the primitives
    PrimitiveType               primitiveType; //!< How to interpret the indexed primitives
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::DrawIndexedVerticesSettings
/// \ingroup graphics
///
/// `sf::DrawIndexedVerticesSettings` is the indexed counterpart of
/// `sf::DrawVerticesSettings`. Instead of feeding the GPU one vertex
/// per primitive corner, you provide a flat vertex pool plus a list
/// of indices into that pool, allowing shared vertices to be reused.
///
/// \see `sf::DrawVerticesSettings`, `sf::IndexType`, `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
