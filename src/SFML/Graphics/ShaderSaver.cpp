#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/ShaderSaver.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLUtils.hpp"
#include "SFML/Window/Glad.hpp"


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
