#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/FramebufferSaver.hpp"
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/Glad.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
FramebufferSaver::FramebufferSaver() :
m_drawFramebufferBinding(static_cast<int>(priv::getGLInteger(GL_DRAW_FRAMEBUFFER_BINDING))),
m_readFramebufferBinding(static_cast<int>(priv::getGLInteger(GL_READ_FRAMEBUFFER_BINDING)))
{
}


////////////////////////////////////////////////////////////
FramebufferSaver::~FramebufferSaver()
{
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(m_readFramebufferBinding)));
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(m_drawFramebufferBinding)));
}

} // namespace sf::priv
