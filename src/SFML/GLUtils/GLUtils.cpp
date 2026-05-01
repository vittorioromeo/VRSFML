// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GLUtils.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(const unsigned int parameterName)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    int result;

    glCheck(glGetIntegerv(parameterName, &result));
    return result;
}


////////////////////////////////////////////////////////////
unsigned int generateAndBindFramebuffer()
{
    GLuint out{};
    glCheck(glGenFramebuffers(1, &out));

    if (out != 0u)
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, out));

    return out;
}


////////////////////////////////////////////////////////////
ScissorDisableGuard::ScissorDisableGuard() :
    m_savedState([]
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    GLboolean scissorEnabled;
    glCheck(glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled));
    return scissorEnabled == GL_TRUE;
}())
{
    if (m_savedState)
        glCheck(glDisable(GL_SCISSOR_TEST));
}


////////////////////////////////////////////////////////////
ScissorDisableGuard::~ScissorDisableGuard()
{
    if (m_savedState)
        glCheck(glEnable(GL_SCISSOR_TEST));
}


/////////////////////////////////////////////////////////////
void bindAndInitializeTexture(const unsigned int textureId, const bool sRgb, const Vec2u size, const unsigned int textureWrapParam)
{
    glCheck(glBindTexture(GL_TEXTURE_2D, textureId));

    glCheck(glTexImage2D(GL_TEXTURE_2D,
                         0,
                         (sRgb ? GL_SRGB8_ALPHA8 : GL_RGBA),
                         static_cast<GLsizei>(size.x),
                         static_cast<GLsizei>(size.y),
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         nullptr));

    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(textureWrapParam)));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(textureWrapParam)));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
}

} // namespace sf::priv
