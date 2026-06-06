#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/Vocabulary/Span.hpp"

#include "Zancle/Base/RequireDesignatedInitializers.hpp"
#include "Zancle/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class VAOHandle;
}

namespace za
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `za::RenderTarget::drawInstancedVertices`
///
/// Bundles the data needed to issue an instanced draw call: a vertex
/// array shared by all instances, a `za::VAOHandle` carrying any
/// per-instance attribute streams (set up via `za::InstanceAttributeBinder`),
/// the number of instances, and the primitive type.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawInstancedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ZA_REQUIRE_DESIGNATED_INITIALIZERS;

    VAOHandle&             vaoHandle;     //!< VAO holding per-instance attribute streams
    za::Span<const Vertex> vertexSpan;    //!< Range of vertices defining a single instance
    za::SizeT              instanceCount; //!< Number of instances to draw
    PrimitiveType          primitiveType; //!< How to interpret the vertices of a single instance
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::DrawInstancedVerticesSettings
/// \ingroup graphics
///
/// `za::DrawInstancedVerticesSettings` is the instanced counterpart
/// of `za::DrawVerticesSettings`. The same vertex range is replayed
/// `instanceCount` times in a single draw call, with per-instance
/// attributes pulled from the streams configured on `vaoHandle`.
///
/// \see `za::DrawVerticesSettings`, `za::DrawInstancedIndexedVerticesSettings`,
///      `za::VAOHandle`, `za::InstanceAttributeBinder`, `za::RenderTarget`
///
////////////////////////////////////////////////////////////
