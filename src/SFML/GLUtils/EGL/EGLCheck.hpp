#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/GLUtils/EGL/EGLGlad.hpp" // not expensive, included everwhere `EGLCheck.hpp` is included

#include "SFML/Base/Assert.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Check the last EGL error
///
/// \param file Source file where the call is located
/// \param line Line number of the source file where the call is located
/// \param expression The evaluated expression as a string
///
/// \return `false` if an error occurred, `true` otherwise
///
////////////////////////////////////////////////////////////
[[gnu::cold]] void eglCheckError(unsigned int eglError, const char* file, unsigned int line, const char* expression);

////////////////////////////////////////////////////////////
/// \brief Helper class to check for EGL errors in debug mode
///
/// This RAII style class is used internally to detect and report EGL
/// errors during development. It captures the location of an EGL call
/// and checks for errors when is object is destroyed (i.e. after the
/// EGL call executes).
///
////////////////////////////////////////////////////////////
struct EglScopedChecker
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] explicit EglScopedChecker(const char* const  theFile,
                                                     const char* const  theExpression,
                                                     const unsigned int theLine) :
        file{theFile},
        expression{theExpression},
        line{theLine}
    {
        SFML_BASE_ASSERT(::eglGetError() == EGL_SUCCESS);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~EglScopedChecker()
    {
        while (true)
        {
            const auto eglError = static_cast<unsigned int>(eglGetError());

            if (eglError == EGL_SUCCESS) [[likely]]
                return;

            eglCheckError(eglError, file, line, expression);
        }
    }

    ////////////////////////////////////////////////////////////
    const char*        file;
    const char*        expression;
    const unsigned int line;
};

////////////////////////////////////////////////////////////
/// Macro to check every EGL API call
////////////////////////////////////////////////////////////
#ifdef SFML_DEBUG

    // In debug mode, perform a test on every EGL call
    // The do-while loop is needed so that glCheck can be used as a single statement in if/else branches
    #define eglCheck(...) \
        (::sf::priv::EglScopedChecker{__FILE__, #__VA_ARGS__, static_cast<unsigned int>(__LINE__)}, __VA_ARGS__)

#else

    #define eglCheck(...) (__VA_ARGS__)

#endif

} // namespace sf::priv
