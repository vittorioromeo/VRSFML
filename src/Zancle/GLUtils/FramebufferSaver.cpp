// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/FramebufferSaver.hpp"

#include "Zancle/GLUtils/GLCheck.hpp"
#include "Zancle/GLUtils/GLUtils.hpp"
#include "Zancle/GLUtils/Glad.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
FramebufferSaver::FramebufferSaver() :
    m_drawFramebufferBinding(priv::getGLInteger(GL_DRAW_FRAMEBUFFER_BINDING)),
    m_readFramebufferBinding(priv::getGLInteger(GL_READ_FRAMEBUFFER_BINDING))
{
}


////////////////////////////////////////////////////////////
FramebufferSaver::~FramebufferSaver()
{
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(m_readFramebufferBinding)));
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(m_drawFramebufferBinding)));
}

} // namespace za::priv
