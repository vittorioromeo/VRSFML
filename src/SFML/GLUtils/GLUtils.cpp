// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GLUtils.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"


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

} // namespace sf::priv
