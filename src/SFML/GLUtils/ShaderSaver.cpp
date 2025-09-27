// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/ShaderSaver.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/Glad.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
ShaderSaver::ShaderSaver() : m_shaderBinding(static_cast<int>(priv::getGLInteger(GL_CURRENT_PROGRAM)))
{
}


////////////////////////////////////////////////////////////
ShaderSaver::~ShaderSaver()
{
    glCheck(glUseProgram(static_cast<GLuint>(m_shaderBinding)));
}

} // namespace sf::priv
