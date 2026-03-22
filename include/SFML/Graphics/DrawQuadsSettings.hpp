#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Builtin/Restrict.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Settings used to draw an array of vertices and precomputed quad indices
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawQuadsSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    const Vertex* SFML_BASE_RESTRICT vertexData;
    base::SizeT                      vertexCount;
    PrimitiveType                    primitiveType;
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
