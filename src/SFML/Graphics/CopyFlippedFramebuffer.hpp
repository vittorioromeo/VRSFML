#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class GraphicsContext;
class Texture;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool copyFlippedFramebuffer(
    GraphicsContext&         graphicsContext,
    base::Optional<Texture>& optTexture,
    bool                     srgb,
    Vector2u                 size,
    unsigned int             srcFBO,
    unsigned int             dstFBO,
    Vector2u                 srcPos = {0u, 0u},
    Vector2u                 dstPos = {0u, 0u});

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool copyFlippedFramebuffer(
    Texture&     tmpTexture,
    Vector2u     size,
    unsigned int srcFBO,
    unsigned int dstFBO,
    Vector2u     srcPos = {0u, 0u},
    Vector2u     dstPos = {0u, 0u});

} // namespace sf::priv
