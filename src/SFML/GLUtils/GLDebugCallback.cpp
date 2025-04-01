#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLDebugCallback.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[maybe_unused]] void GLAPIENTRY debugGLMessageCallback(
    GLenum       source,
    GLenum       type,
    unsigned int id,
    GLenum       severity,
    GLsizei /* length */,
    const char* message,
    const void* /* userParam */)
{
    // ignore non-significant error/warning codes
    if (id == 131'169 || id == 131'185 || id == 131'218 || id == 131'204 || id == 1 || id == 2)
        return;

    auto& multiLineErr = sf::priv::err(true /* multiLine */);

    multiLineErr << "---------------" << '\n' << "Debug message (" << id << "): " << message << "\nSource: ";

    // clang-format off
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             multiLineErr << "API";             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   multiLineErr << "Window System";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: multiLineErr << "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     multiLineErr << "Third Party";     break;
        case GL_DEBUG_SOURCE_APPLICATION:     multiLineErr << "Application";     break;
        case GL_DEBUG_SOURCE_OTHER:           multiLineErr << "Other";           break;
    }
    // clang-format on

    multiLineErr << "\nType: ";

    // clang-format off
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               multiLineErr << "Error";                break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: multiLineErr << "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  multiLineErr << "Undefined Behaviour";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         multiLineErr << "Portability";          break;
        case GL_DEBUG_TYPE_PERFORMANCE:         multiLineErr << "Performance";          break;
        case GL_DEBUG_TYPE_MARKER:              multiLineErr << "Marker";               break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          multiLineErr << "Push Group";           break;
        case GL_DEBUG_TYPE_POP_GROUP:           multiLineErr << "Pop Group";            break;
        case GL_DEBUG_TYPE_OTHER:               multiLineErr << "Other";                break;
    }
    // clang-format on

    multiLineErr << "\nSeverity: ";

    // clang-format off
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         multiLineErr << "High";         break;
        case GL_DEBUG_SEVERITY_MEDIUM:       multiLineErr << "Medium";       break;
        case GL_DEBUG_SEVERITY_LOW:          multiLineErr << "Low";          break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: multiLineErr << "Notification"; break;
    }
    // clang-format on

    sf::priv::err() << '\n';
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
void setupGLDebugCallback()
{
#ifndef SFML_SYSTEM_EMSCRIPTEN
    // TODO P0: maybe conditionally enable depending on graphicscontext's debug ctx param?
    // or for emscripten, try to enable without glcheck and then drain gl errors
    glCheck(glEnable(GL_DEBUG_OUTPUT));
    if (glGetError() != 0u)
        priv::err() << "Failed to enable OpenGL debug output";

    glCheck(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    if (glGetError() != 0u)
        priv::err() << "Failed to enable OpenGL synchronous debug output";

    glCheck(glDebugMessageCallback(debugGLMessageCallback, nullptr));
    if (glGetError() != 0u)
        priv::err() << "Failed to setup OpenGL debug output callback";
#endif
}

} // namespace sf::priv
