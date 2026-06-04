#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "ZancleBase/RequireDesignatedInitializers.hpp"
#include "ZancleBase/Span.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `za::RenderTarget::drawQuads`
///
/// `vertexSpan` is interpreted as 4 vertices per quad. The renderer
/// uses a precomputed index buffer to expand quads into two
/// triangles each, so the caller does not have to duplicate
/// vertices.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawQuadsSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    zb::Span<const Vertex> vertexSpan;    //!< Range of vertices laid out as groups of 4 corners
    PrimitiveType          primitiveType; //!< Primitive type used for the expanded triangles
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::DrawQuadsSettings
/// \ingroup graphics
///
/// `za::DrawQuadsSettings` is a convenience for drawing groups of
/// 4-vertex quads without having to manually duplicate vertices for
/// the two triangles each quad consists of. The renderer reuses a
/// precomputed quad index buffer to perform the expansion in a
/// single indexed draw call.
///
/// \see `za::DrawIndexedVerticesSettings`, `za::RenderTarget`
///
////////////////////////////////////////////////////////////
