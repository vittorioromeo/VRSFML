// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
unsigned int glGetErrorImpl()
{
    return glGetError();
}


////////////////////////////////////////////////////////////
bool glCheckError(const unsigned int openGlError, const char* const file, const unsigned int line, const char* const expression)
{
    if (!WindowContext::hasActiveThreadLocalGlContext())
    {
        err() << "An internal OpenGL call failed in " << Path{file}.filename() << "(" << line << ")."
              << "\nExpression:\n   " << expression << "\nNo active OpenGL context on calling thread.\n";

#ifdef SFML_FATAL_OPENGL_ERRORS
        SFML_BASE_ASSERT(false && "OpenGL error (fatal OpenGL errors enabled)");
#endif
    }


    const auto logError = [&](const char* const error, const char* const description)
    {
        err() << "An internal OpenGL call failed in " << Path{file}.filename() << "(" << line << ")."
              << "\nExpression:\n   " << expression << "\nError description:\n   " << error << "\n   " << description
              << '\n';

#ifdef SFML_FATAL_OPENGL_ERRORS
        SFML_BASE_ASSERT(false && "OpenGL error (fatal OpenGL errors enabled)");
#endif

        return false;
    };

    switch (openGlError)
    {
        case GL_NO_ERROR:
            return true;

        case GL_INVALID_ENUM:
            return logError("GL_INVALID_ENUM", "An unacceptable value has been specified for an enumerated argument.");

        case GL_INVALID_VALUE:
            return logError("GL_INVALID_VALUE", "A numeric argument is out of range.");

        case GL_INVALID_OPERATION:
            return logError("GL_INVALID_OPERATION", "The specified operation is not allowed in the current state.");

        case GL_STACK_OVERFLOW:
            return logError("GL_STACK_OVERFLOW", "This command would cause a stack overflow.");

        case GL_STACK_UNDERFLOW:
            return logError("GL_STACK_UNDERFLOW", "This command would cause a stack underflow.");

        case GL_OUT_OF_MEMORY:
            return logError("GL_OUT_OF_MEMORY", "There is not enough memory left to execute the command.");

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return logError("GL_INVALID_FRAMEBUFFER_OPERATION",
                            "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".");

        default:
            return logError("Unknown error", "Unknown description");
    }
}

} // namespace sf::priv
