#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLExtensions.hpp"


// We check for this definition in order to avoid multiple definitions of GLAD
// entities during unity builds of SFML.
#ifndef GLAD_GL_IMPLEMENTATION_INCLUDED
#define GLAD_GL_IMPLEMENTATION_INCLUDED
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#endif

#if !defined(GL_MAJOR_VERSION)
#define GL_MAJOR_VERSION 0x821B
#endif

#if !defined(GL_MINOR_VERSION)
#define GL_MINOR_VERSION 0x821C
#endif


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
