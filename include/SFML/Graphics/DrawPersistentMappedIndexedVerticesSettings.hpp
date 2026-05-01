#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/PrimitiveType.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class PersistentGPUDrawableBatch;
}

namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Parameters for `sf::RenderTarget::drawPersistentMappedIndexedVertices`
///
/// Issues an indexed draw call from the GPU-resident, persistent
/// mapped buffers owned by a `sf::PersistentGPUDrawableBatch`. The
/// offsets and counts select the slice of the persistent buffers to
/// draw, allowing the same batch to be partially submitted.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawPersistentMappedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    const PersistentGPUDrawableBatch& gpuDrawableBatch; //!< Source of the persistent vertex/index buffers
    base::SizeT                       indexCount;       //!< Number of indices to consume from `indexOffset`
    base::SizeT                       indexOffset;      //!< Offset (in indices) into the persistent index buffer
    base::SizeT                       vertexOffset;     //!< Base vertex offset added to each fetched index
    PrimitiveType                     primitiveType;    //!< How to interpret the indexed primitives
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::DrawPersistentMappedIndexedVerticesSettings
/// \ingroup graphics
///
/// `sf::DrawPersistentMappedIndexedVerticesSettings` is the lowest-
/// overhead draw path for content that lives in a
/// `sf::PersistentGPUDrawableBatch`: the buffers are mapped only
/// once and the GPU is told to render a slice of them.
///
/// \see `sf::PersistentGPUDrawableBatch`, `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
