#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Span.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class VAOHandle;
}

namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `sf::RenderTarget::drawInstancedIndexedVertices`
///
/// Bundles the data needed to issue an instanced *and* indexed draw
/// call: a vertex pool, an index list defining a single instance,
/// the instance count, and a `sf::VAOHandle` carrying the per-instance
/// attribute streams.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawInstancedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    VAOHandle&                  vaoHandle;     //!< VAO holding per-instance attribute streams
    base::Span<const Vertex>    vertexSpan;    //!< Pool of vertices referenced by `indexSpan`
    base::Span<const IndexType> indexSpan;     //!< Indices defining the primitives of a single instance
    base::SizeT                 instanceCount; //!< Number of instances to draw
    PrimitiveType               primitiveType; //!< How to interpret the indexed primitives of a single instance
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::DrawInstancedIndexedVerticesSettings
/// \ingroup graphics
///
/// `sf::DrawInstancedIndexedVerticesSettings` combines indexing
/// (vertex reuse) with instancing (geometry reuse). The same indexed
/// mesh is replayed `instanceCount` times in a single draw call,
/// with per-instance attributes pulled from `vaoHandle`.
///
/// \see `sf::DrawIndexedVerticesSettings`, `sf::DrawInstancedVerticesSettings`,
///      `sf::VAOHandle`, `sf::InstanceAttributeBinder`, `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
