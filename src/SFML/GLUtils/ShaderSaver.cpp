#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/Glad.hpp"
#include "SFML/GLUtils/ShaderSaver.hpp"


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
