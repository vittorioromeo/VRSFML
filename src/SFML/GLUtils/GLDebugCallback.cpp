// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GLDebugCallback.hpp"

#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/StringView.hpp"


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

    const auto sourceStr = [&]() -> sf::base::StringView
    {
        // clang-format off
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:             return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window System";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Third Party";
            case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
            case GL_DEBUG_SOURCE_OTHER:           return "Other";
        }
        // clang-format on
        return "";
    }();

    const auto typeStr = [&]() -> sf::base::StringView
    {
        // clang-format off
        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:               return "Error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behaviour";
            case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
            case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
            case GL_DEBUG_TYPE_MARKER:              return "Marker";
            case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
            case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
            case GL_DEBUG_TYPE_OTHER:               return "Other";
        }
        // clang-format on
        return "";
    }();

    const auto severityStr = [&]() -> sf::base::StringView
    {
        // clang-format off
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         return "High";
            case GL_DEBUG_SEVERITY_MEDIUM:       return "Medium";
            case GL_DEBUG_SEVERITY_LOW:          return "Low";
            case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
        }
        // clang-format on
        return "";
    }();

    sf::priv::errMsg("---------------\nDebug message ({}): {}\nSource: {}\nType: {}\nSeverity: {}",
                     id,
                     message,
                     sourceStr,
                     typeStr,
                     severityStr);
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
void setupGLDebugCallback()
{
#ifndef SFML_SYSTEM_EMSCRIPTEN // TODO P1: revisit this ifdef
    glCheck(glEnable(GL_DEBUG_OUTPUT));
    if (glGetError() != 0u)
        errMsg("Failed to enable OpenGL debug output");

    glCheck(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    if (glGetError() != 0u)
        errMsg("Failed to enable OpenGL synchronous debug output");

    glCheck(glDebugMessageCallback(debugGLMessageCallback, nullptr));
    if (glGetError() != 0u)
        errMsg("Failed to setup OpenGL debug output callback");
#endif
}

} // namespace sf::priv
