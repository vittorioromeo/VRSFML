#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2Base.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Get GL integer and assert success
///
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(unsigned int parameterName);

////////////////////////////////////////////////////////////
/// \brief Generate and bind a new framebuffer object
///
/// Creates a new OpenGL framebuffer object, binds it to the current context,
/// and returns its ID. Returns 0 if framebuffer creation failed.
///
/// \return New framebuffer ID or 0 on failure
///
////////////////////////////////////////////////////////////
[[nodiscard]] unsigned int generateAndBindFramebuffer();

////////////////////////////////////////////////////////////
/// \brief RAII guard for temporary scissor test disabling
///
/// Automatically disables scissor test on construction and restores
/// the original scissor test state on destruction.
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

////////////////////////////////////////////////////////////
/// \brief Bind and initialize an OpenGL texture
///
/////////////////////////////////////////////////////////////
void bindAndInitializeTexture(unsigned int textureId, bool sRgb, Vec2u size, unsigned int textureWrapParam);

} // namespace sf::priv
