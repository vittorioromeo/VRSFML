#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Base/Span.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Settings used to draw an array of vertices and precomputed quad indices
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawQuadsSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    base::Span<const Vertex> vertexSpan;
    PrimitiveType            primitiveType;
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
