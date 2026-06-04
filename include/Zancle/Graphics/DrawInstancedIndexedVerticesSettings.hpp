#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/IndexType.hpp"
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/Vertex.hpp"
#include "ZancleBase/RequireDesignatedInitializers.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Span.hpp"


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
/// \brief Parameters for `za::RenderTarget::drawInstancedIndexedVertices`
///
/// Bundles the data needed to issue an instanced *and* indexed draw
/// call: a vertex pool, an index list defining a single instance,
/// the instance count, and a `za::VAOHandle` carrying the per-instance
/// attribute streams.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawInstancedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    VAOHandle&                vaoHandle;     //!< VAO holding per-instance attribute streams
    zb::Span<const Vertex>    vertexSpan;    //!< Pool of vertices referenced by `indexSpan`
    zb::Span<const IndexType> indexSpan;     //!< Indices defining the primitives of a single instance
    zb::SizeT                 instanceCount; //!< Number of instances to draw
    PrimitiveType             primitiveType; //!< How to interpret the indexed primitives of a single instance
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::DrawInstancedIndexedVerticesSettings
/// \ingroup graphics
///
/// `za::DrawInstancedIndexedVerticesSettings` combines indexing
/// (vertex reuse) with instancing (geometry reuse). The same indexed
/// mesh is replayed `instanceCount` times in a single draw call,
/// with per-instance attributes pulled from `vaoHandle`.
///
/// \see `za::DrawIndexedVerticesSettings`, `za::DrawInstancedVerticesSettings`,
///      `za::VAOHandle`, `za::InstanceAttributeBinder`, `za::RenderTarget`
///
////////////////////////////////////////////////////////////
