#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Builtin/Restrict.hpp"
#include "SFML/Base/SizeT.hpp"


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
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawInstancedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    VAOHandle&                          vaoHandle;
    const Vertex* SFML_BASE_RESTRICT    vertexData;
    base::SizeT                         vertexCount;
    const IndexType* SFML_BASE_RESTRICT indexData;
    base::SizeT                         indexCount;
    base::SizeT                         instanceCount;
    PrimitiveType                       primitiveType;
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
