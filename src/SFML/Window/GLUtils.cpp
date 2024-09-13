#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLUtils.hpp"
#include "SFML/Window/Glad.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(unsigned int parameterName)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    int result;

    glCheck(glGetIntegerv(parameterName, &result));
    return result;
}


} // namespace sf::priv
