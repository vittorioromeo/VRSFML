// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/TextureSaver.hpp"

#include "Zancle/GLUtils/GLCheck.hpp"
#include "Zancle/GLUtils/GLUtils.hpp"
#include "Zancle/GLUtils/Glad.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
TextureSaver::TextureSaver() : m_textureBinding(priv::getGLInteger(GL_TEXTURE_BINDING_2D))
{
}


////////////////////////////////////////////////////////////
TextureSaver::~TextureSaver()
{
    glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(m_textureBinding)));
}

} // namespace za::priv
