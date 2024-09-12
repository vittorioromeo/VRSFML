#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLExtensions.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
[[nodiscard]] GLint getGLInteger(GLenum parameterName)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    GLint result;

    glCheck(glGetIntegerv(parameterName, &result));
    return result;
}


} // namespace sf::priv
