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
/// \brief Settings used to draw a persistent mapped buffer and indices
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawPersistentMappedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    const PersistentGPUDrawableBatch& gpuDrawableBatch;
    base::SizeT                       indexCount;
    base::SizeT                       indexOffset;
    base::SizeT                       vertexOffset;
    PrimitiveType                     primitiveType;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderTarget
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
