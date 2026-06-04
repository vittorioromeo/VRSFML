#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/PrimitiveType.hpp"

#include "ZancleBase/RequireDesignatedInitializers.hpp"
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class PersistentGPUDrawableBatch;
}

namespace za
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `za::RenderTarget::drawPersistentMappedIndexedVertices`
///
/// Issues an indexed draw call from the GPU-resident, persistent
/// mapped buffers owned by a `za::PersistentGPUDrawableBatch`. The
/// offsets and counts select the slice of the persistent buffers to
/// draw, allowing the same batch to be partially submitted.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawPersistentMappedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    ZB_REQUIRE_DESIGNATED_INITIALIZERS;

    const PersistentGPUDrawableBatch& gpuDrawableBatch; //!< Source of the persistent vertex/index buffers
    zb::SizeT                       indexCount;       //!< Number of indices to consume from `indexOffset`
    zb::SizeT                       indexOffset;      //!< Offset (in indices) into the persistent index buffer
    zb::SizeT                       vertexOffset;     //!< Base vertex offset added to each fetched index
    PrimitiveType                     primitiveType;    //!< How to interpret the indexed primitives
};

} // namespace za


////////////////////////////////////////////////////////////
/// \struct za::DrawPersistentMappedIndexedVerticesSettings
/// \ingroup graphics
///
/// `za::DrawPersistentMappedIndexedVerticesSettings` is the lowest-
/// overhead draw path for content that lives in a
/// `za::PersistentGPUDrawableBatch`: the buffers are mapped only
/// once and the GPU is told to render a slice of them.
///
/// \see `za::PersistentGPUDrawableBatch`, `za::RenderTarget`
///
////////////////////////////////////////////////////////////
