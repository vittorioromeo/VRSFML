#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Assert.hpp>

#include <iostream>

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


namespace
{
////////////////////////////////////////////////////////////
void extensionSanityCheck()
{
    static const auto check = [](int& flag, auto... entryPoints)
    {
        // If a required entry point is missing, flag the whole extension as unavailable
        if (!(entryPoints && ...))
            flag = 0;
    };
#ifdef SFML_OPENGL_ES
    check(GLEXT_multitexture_dependencies);
    check(GLEXT_vertex_buffer_object_dependencies);
    check(GLEXT_EXT_blend_minmax_dependencies);
#else
    check(GLEXT_blend_minmax_dependencies);
    check(GLEXT_multitexture_dependencies);
    check(GLEXT_blend_func_separate_dependencies);
    check(GLEXT_vertex_buffer_object_dependencies);
    check(GLEXT_shader_objects_dependencies);
    check(GLEXT_blend_equation_separate_dependencies);
    check(GLEXT_framebuffer_object_dependencies);
    check(GLEXT_framebuffer_blit_dependencies);
    check(GLEXT_framebuffer_multisample_dependencies);
    check(GLEXT_copy_buffer_dependencies);
#endif
}


////////////////////////////////////////////////////////////
void GLAPIENTRY mMessageCallback(
    GLenum       source,
    GLenum       type,
    unsigned int id,
    GLenum       severity,
    GLsizei /* length */,
    const char* message,
    const void* /* userParam */)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------" << '\n' << "Debug message (" << id << "): " << message << '\n';

    // clang-format off
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API";             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party";     break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application";     break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other";           break;
    }
    // clang-format on

    std::cout << '\n';

    // clang-format off
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error";                break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability";          break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance";          break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker";               break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group";           break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group";            break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other";                break;
    }
    // clang-format on

    std::cout << '\n';

    // clang-format off
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high";         break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium";       break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low";          break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    }
    // clang-format on

    std::cout << '\n' << std::endl;
}

} // namespace


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


////////////////////////////////////////////////////////////
void ensureExtensionsInit(GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

    static bool initialized = false;
    if (initialized)
        return;

    initialized = true;

    // Load OpenGL or OpenGL ES entry points using glad
    graphicsContext.loadGLEntryPointsViaGLAD();

    // Some GL implementations don't fully follow extension specifications
    // and advertise support for extensions although not providing the
    // entry points specified for the corresponding extension.
    // In order to protect ourselves from such implementations, we perform
    // a sanity check to ensure an extension is _really_ supported, even
    // from an entry point perspective.
    extensionSanityCheck();

// TODO P0:
#ifndef SFML_SYSTEM_EMSCRIPTEN
    // TODO P0: maybe conditionally enable depending on graphicscontext's debug ctx param?
    // or for emscripten, try to enable without glcheck and then drain gl errors
    glCheck(glEnable(GL_DEBUG_OUTPUT));
    glCheck(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    glCheck(glDebugMessageCallback(mMessageCallback, nullptr));

    // Retrieve the context version number
    const auto majorVersion = getGLInteger(GL_MAJOR_VERSION);
    const auto minorVersion = getGLInteger(GL_MINOR_VERSION);

    if ((majorVersion < 1) || ((majorVersion == 1) && (minorVersion < 1)))
    {
        priv::err() << "sfml-graphics requires support for OpenGL 1.1 or greater" << '\n'
                    << "Ensure that hardware acceleration is enabled if available";
    }
#endif
}

} // namespace sf::priv
