// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/EGL/EGLCheck.hpp"

#include "SFML/GLUtils/EGL/EGLGlad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
void eglCheckError(const unsigned int eglError, const char* file, unsigned int line, const char* expression)
{
    const auto logError = [&](const char* error, const char* description)
    {
        err() << "An internal EGL call failed in " << Path{file}.filename() << " (" << line << ")."
              << "\nExpression:\n   " << expression << "\nError description:\n   " << error << "\n   " << description
              << '\n';
    };

    SFML_BASE_ASSERT(eglError != EGL_SUCCESS);

    switch (eglError)
    {
        case EGL_NOT_INITIALIZED:
            logError("EGL_NOT_INITIALIZED",
                     "EGL is not initialized, or could not be initialized, for the specified display");
            return;

        case EGL_BAD_ACCESS:
            logError("EGL_BAD_ACCESS",
                     "EGL cannot access a requested resource (for example, a context is bound in another "
                     "thread)");
            return;

        case EGL_BAD_ALLOC:
            logError("EGL_BAD_ALLOC", "EGL failed to allocate resources for the requested operation");
            return;

        case EGL_BAD_ATTRIBUTE:
            logError("EGL_BAD_ATTRIBUTE",
                     "an unrecognized attribute or attribute value was passed in an attribute list");
            return;

        case EGL_BAD_CONTEXT:
            logError("EGL_BAD_CONTEXT", "an EGLContext argument does not name a valid EGLContext");
            return;

        case EGL_BAD_CONFIG:
            logError("EGL_BAD_CONFIG", "an EGLConfig argument does not name a valid EGLConfig");
            return;

        case EGL_BAD_CURRENT_SURFACE:
            logError("EGL_BAD_CURRENT_SURFACE",
                     "the current surface of the calling thread is a window, pbuffer, or pixmap that is no "
                     "longer valid");
            return;

        case EGL_BAD_DISPLAY:
            logError("EGL_BAD_DISPLAY",
                     "an EGLDisplay argument does not name a valid EGLDisplay; or, EGL is not initialized "
                     "on the specified EGLDisplay");
            return;

        case EGL_BAD_SURFACE:
            logError("EGL_BAD_SURFACE",
                     "an EGLSurface argument does not name a valid surface (window, pbuffer, or pixmap) "
                     "configured for rendering");
            return;

        case EGL_BAD_MATCH:
            logError("EGL_BAD_MATCH",
                     "arguments are inconsistent; for example, an otherwise valid context requires buffers "
                     "(e.g. depth or stencil) not allocated by an otherwise valid surface");
            return;

        case EGL_BAD_PARAMETER:
            logError("EGL_BAD_PARAMETER", "one or more argument values are invalid");
            return;

        case EGL_BAD_NATIVE_PIXMAP:
            logError("EGL_BAD_NATIVE_PIXMAP", "an EGLNativePixmapType argument does not refer to a valid native pixmap");
            return;

        case EGL_BAD_NATIVE_WINDOW:
            logError("EGL_BAD_NATIVE_WINDOW", "an EGLNativeWindowType argument does not refer to a valid native window");
            return;

        case EGL_CONTEXT_LOST:
            logError("EGL_CONTEXT_LOST",
                     "a power management event has occurred. The application must destroy all contexts and "
                     "reinitialize client API state and objects to continue rendering");
            return;

        default:
            logError("Unknown error", "Unknown description");
            return;
    }
}

} // namespace sf::priv
