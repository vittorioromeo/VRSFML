#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextureSaver.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLUtils.hpp"
#include "SFML/Window/Glad.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
TextureSaver::TextureSaver() : m_textureBinding(static_cast<int>(priv::getGLInteger(GL_TEXTURE_BINDING_2D)))
{
}


////////////////////////////////////////////////////////////
TextureSaver::~TextureSaver()
{
    glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(m_textureBinding)));
}

} // namespace sf::priv
