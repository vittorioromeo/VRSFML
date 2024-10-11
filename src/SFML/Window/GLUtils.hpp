#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Get GL integer and assert success
///
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(unsigned int parameterName);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
void blitFramebuffer(bool invertYAxis, UIntRect src, UIntRect dst);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
void blitFramebuffer(bool invertYAxis, Vector2u size, Vector2u srcPos, Vector2u dstPos);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
void copyFramebuffer(bool         invertYAxis,
                     Vector2u     size,
                     unsigned int srcFBO,
                     unsigned int dstFBO,
                     Vector2u     srcPos = {0u, 0u},
                     Vector2u     dstPos = {0u, 0u});

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[nodiscard]] unsigned int generateAndBindFramebuffer();

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class ScissorDisableGuard
{
public:
    explicit ScissorDisableGuard();
    ~ScissorDisableGuard();

    ScissorDisableGuard(const ScissorDisableGuard&) = delete;
    ScissorDisableGuard(ScissorDisableGuard&&)      = delete;

    ScissorDisableGuard& operator=(const ScissorDisableGuard&) = delete;
    ScissorDisableGuard& operator=(ScissorDisableGuard&&)      = delete;

private:
    bool m_savedState;
};

} // namespace sf::priv
