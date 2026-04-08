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
/// \brief Decode and log a known EGL error
///
/// Translates a previously retrieved EGL error code into a human-readable
/// description and writes it to `sf::priv::err()` along with the source
/// location of the offending call. Invoked by `EglScopedChecker` and
/// not meant to be called directly by user code; use the `eglCheck`
/// macro instead.
///
/// \param eglError   The EGL error code previously returned by `eglGetError()`
/// \param file       Source file where the offending call is located
/// \param line       Line number in the source file where the call is located
/// \param expression The evaluated expression as a string
///
////////////////////////////////////////////////////////////
[[gnu::cold]] void eglCheckError(unsigned int eglError, const char* file, unsigned int line, const char* expression);

////////////////////////////////////////////////////////////
/// \brief Helper class to check for EGL errors in debug mode
///
/// This RAII style class is used internally by the `eglCheck` macro
/// to detect and report EGL errors during development. It captures
/// the source location of an EGL call on construction (asserting that
/// no error was already pending) and drains/checks the EGL error
/// queue when the object is destroyed (i.e. after the wrapped EGL
/// call has executed).
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
        SFML_BASE_ASSERT(eglGetError() == EGL_SUCCESS);
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
///
/// In debug builds, wraps the given EGL expression with an
/// `EglScopedChecker` that captures the source location and detects
/// errors before and after the call. In release builds the macro
/// expands to the bare expression with no overhead. The wrapper
/// preserves the value category and return type of the expression,
/// so `eglCheck(eglCreateContext(...))` can be used directly in an
/// assignment.
///
////////////////////////////////////////////////////////////
#ifdef SFML_DEBUG

    #define eglCheck(...) \
        (::sf::priv::EglScopedChecker{__FILE__, #__VA_ARGS__, static_cast<unsigned int>(__LINE__)}, __VA_ARGS__)

#else

    #define eglCheck(...) (__VA_ARGS__)

#endif

} // namespace sf::priv
