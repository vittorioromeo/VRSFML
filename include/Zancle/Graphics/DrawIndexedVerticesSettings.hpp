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
#include "ZancleBase/Span.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `za::RenderTarget::drawIndexedVertices`
///
/// Bundles a vertex array, an index array, and the primitive type.
/// Indexed drawing lets you reuse vertices, which is significantly
/// cheaper than duplicating them when many primitives share corners.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    zb::Span<const Vertex>    vertexSpan;    //!< Range of unique vertices that will be referenced by the indices
    zb::Span<const IndexType> indexSpan;     //!< Range of indices into `vertexSpan` defining the primitives
    PrimitiveType             primitiveType; //!< How to interpret the indexed primitives
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::DrawIndexedVerticesSettings
/// \ingroup graphics
///
/// `za::DrawIndexedVerticesSettings` is the indexed counterpart of
/// `za::DrawVerticesSettings`. Instead of feeding the GPU one vertex
/// per primitive corner, you provide a flat vertex pool plus a list
/// of indices into that pool, allowing shared vertices to be reused.
///
/// \see `za::DrawVerticesSettings`, `za::IndexType`, `za::RenderTarget`
///
////////////////////////////////////////////////////////////
