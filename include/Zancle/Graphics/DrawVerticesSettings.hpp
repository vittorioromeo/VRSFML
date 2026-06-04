#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
/// \brief Parameters for `za::RenderTarget::drawVertices`
///
/// Bundles the vertex data and the primitive type used to interpret
/// it. Used by `za::RenderTarget::drawVertices` and the convenience
/// `za::RenderTarget::draw` overloads.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    zb::Span<const Vertex> vertexSpan;    //!< Range of vertices to draw
    PrimitiveType            primitiveType; //!< How to interpret the vertices (triangles, lines, ...)
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::DrawVerticesSettings
/// \ingroup graphics
///
/// `za::DrawVerticesSettings` is the simplest of the `Draw*Settings`
/// family: it points at a contiguous range of `za::Vertex` and
/// declares which `za::PrimitiveType` should be used to interpret
/// the range. It is consumed by `za::RenderTarget::drawVertices`.
///
/// For indexed, instanced, or persistent-mapped variants, see
/// `za::DrawIndexedVerticesSettings`, `za::DrawInstancedVerticesSettings`,
/// `za::DrawInstancedIndexedVerticesSettings`, and
/// `za::DrawPersistentMappedIndexedVerticesSettings`.
///
/// \see `za::RenderTarget`, `za::Vertex`, `za::PrimitiveType`
///
////////////////////////////////////////////////////////////
