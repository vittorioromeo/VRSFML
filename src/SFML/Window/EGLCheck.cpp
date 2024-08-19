#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/EGLCheck.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"

// Emscripten does not use GLAD
#ifdef SFML_SYSTEM_EMSCRIPTEN
#define KHRONOS_APIENTRY
#include <EGL/egl.h>
#else
#include <glad/egl.h>
#endif


namespace sf::priv
{
////////////////////////////////////////////////////////////
bool eglCheckError(const char* file, unsigned int line, const char* expression)
{
    const auto logError = [&](const char* error, const char* description)
    {
        err() << "An internal EGL call failed in " << Path{file}.filename() << " (" << line << ") : "
              << "\nExpression:\n   " << expression << "\nError description:\n   " << error << "\n   " << description
              << '\n';

        return false;
    };

    switch (eglGetError())
    {
        case EGL_SUCCESS:
            return true;

        case EGL_NOT_INITIALIZED:
            return logError("EGL_NOT_INITIALIZED",
                            "EGL is not initialized, or could not be initialized, for the specified display");

        case EGL_BAD_ACCESS:
            return logError("EGL_BAD_ACCESS",
                            "EGL cannot access a requested resource (for example, a context is bound in another "
                            "thread)");

        case EGL_BAD_ALLOC:
            return logError("EGL_BAD_ALLOC", "EGL failed to allocate resources for the requested operation");

        case EGL_BAD_ATTRIBUTE:
            return logError("EGL_BAD_ATTRIBUTE",
                            "an unrecognized attribute or attribute value was passed in an attribute list");

        case EGL_BAD_CONTEXT:
            return logError("EGL_BAD_CONTEXT", "an EGLContext argument does not name a valid EGLContext");

        case EGL_BAD_CONFIG:
            return logError("EGL_BAD_CONFIG", "an EGLConfig argument does not name a valid EGLConfig");

        case EGL_BAD_CURRENT_SURFACE:
            return logError("EGL_BAD_CURRENT_SURFACE",
                            "the current surface of the calling thread is a window, pbuffer, or pixmap that is no "
                            "longer valid");

        case EGL_BAD_DISPLAY:
            return logError("EGL_BAD_DISPLAY",
                            "an EGLDisplay argument does not name a valid EGLDisplay; or, EGL is not initialized "
                            "on the specified EGLDisplay");

        case EGL_BAD_SURFACE:
            return logError("EGL_BAD_SURFACE",
                            "an EGLSurface argument does not name a valid surface (window, pbuffer, or pixmap) "
                            "configured for rendering");

        case EGL_BAD_MATCH:
            return logError("EGL_BAD_MATCH",
                            "arguments are inconsistent; for example, an otherwise valid context requires buffers "
                            "(e.g. depth or stencil) not allocated by an otherwise valid surface");

        case EGL_BAD_PARAMETER:
            return logError("EGL_BAD_PARAMETER", "one or more argument values are invalid");

        case EGL_BAD_NATIVE_PIXMAP:
            return logError("EGL_BAD_NATIVE_PIXMAP",
                            "an EGLNativePixmapType argument does not refer to a valid native pixmap");

        case EGL_BAD_NATIVE_WINDOW:
            return logError("EGL_BAD_NATIVE_WINDOW",
                            "an EGLNativeWindowType argument does not refer to a valid native window");

        case EGL_CONTEXT_LOST:
            return logError("EGL_CONTEXT_LOST",
                            "a power management event has occurred. The application must destroy all contexts and "
                            "reinitialize client API state and objects to continue rendering");

        default:
            return logError("Unknown error", "Unknown description");
    }
}

} // namespace sf::priv
