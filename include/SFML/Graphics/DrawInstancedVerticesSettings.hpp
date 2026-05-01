#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
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
/// \brief Parameters for `sf::RenderTarget::drawInstancedVertices`
///
/// Bundles the data needed to issue an instanced draw call: a vertex
/// array shared by all instances, a `sf::VAOHandle` carrying any
/// per-instance attribute streams (set up via `sf::InstanceAttributeBinder`),
/// the number of instances, and the primitive type.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawInstancedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    VAOHandle&               vaoHandle;     //!< VAO holding per-instance attribute streams
    base::Span<const Vertex> vertexSpan;    //!< Range of vertices defining a single instance
    base::SizeT              instanceCount; //!< Number of instances to draw
    PrimitiveType            primitiveType; //!< How to interpret the vertices of a single instance
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::DrawInstancedVerticesSettings
/// \ingroup graphics
///
/// `sf::DrawInstancedVerticesSettings` is the instanced counterpart
/// of `sf::DrawVerticesSettings`. The same vertex range is replayed
/// `instanceCount` times in a single draw call, with per-instance
/// attributes pulled from the streams configured on `vaoHandle`.
///
/// \see `sf::DrawVerticesSettings`, `sf::DrawInstancedIndexedVerticesSettings`,
///      `sf::VAOHandle`, `sf::InstanceAttributeBinder`, `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
