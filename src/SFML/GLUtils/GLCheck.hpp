#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/GLUtils/Glad.hpp" // not expensive, included everwhere `GLCheck.hpp` is included

#include "SFML/Base/Assert.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Check the last OpenGL error
///
/// \param openGlError The OpenGL error code to check
/// \param file Source file where the call is located
/// \param line Line number of the source file where the call is located
/// \param expression The evaluated expression as a string
///
/// \return `false` if an error occurred, `true` otherwise
///
////////////////////////////////////////////////////////////
[[gnu::cold]] void glCheckError(unsigned int openGlError, const char* file, unsigned int line, const char* expression);

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
[[gnu::cold]] void printUncheckedPriorGlError();

////////////////////////////////////////////////////////////
/// \brief Helper class to check for OpenGL errors in debug mode
///
/// This RAII style class is used internally to detect and report OpenGL
/// errors during development. It captures the location of an OpenGL call
/// and checks for errors when is object is destroyed (i.e. after the
/// OpenGL call executes).
///
////////////////////////////////////////////////////////////
struct GlScopedChecker
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] explicit GlScopedChecker(const char* const  theFile,
                                                    const char* const  theExpression,
                                                    const unsigned int theLine) :
        file{theFile},
        expression{theExpression},
        line{theLine}
    {
        const unsigned int openGlError = glGetError();

        if (openGlError == GL_NO_ERROR) [[likely]]
            return;

        printUncheckedPriorGlError();
        glCheckError(openGlError, file, line, expression);

        SFML_BASE_ASSERT(false);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~GlScopedChecker()
    {
        while (true)
        {
            const unsigned int openGlError = glGetError();

            if (openGlError == GL_NO_ERROR) [[likely]]
                return;

            glCheckError(openGlError, file, line, expression);
        }
    }

    ////////////////////////////////////////////////////////////
    const char*        file;
    const char*        expression;
    const unsigned int line;
};

////////////////////////////////////////////////////////////
/// \brief Helper class to check for OpenGL errors in debug mode (custom func)
///
/// Drain/ignores any error coming from the custom function.
///
////////////////////////////////////////////////////////////
template <typename TFunc>
struct GlScopedCheckerIgnoreWithFunc
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] explicit GlScopedCheckerIgnoreWithFunc(TFunc theFunc) : func{theFunc}
    {
        SFML_BASE_ASSERT(func() == GL_NO_ERROR);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~GlScopedCheckerIgnoreWithFunc()
    {
        while (func() != GL_NO_ERROR)
            /* drain/ignore any error */;
    }

    ////////////////////////////////////////////////////////////
    TFunc func;
};

////////////////////////////////////////////////////////////
/// Macro to check every OpenGL API call
////////////////////////////////////////////////////////////
#if defined(SFML_DEBUG) && !defined(SFML_SYSTEM_EMSCRIPTEN) // `glCheck` is prohibitively slow on Emscripten

// In debug mode, perform a test on every OpenGL call
// The do-while loop is needed so that glCheck can be used as a single statement in if/else branches

    #define glCheck(...) \
        (::sf::priv::GlScopedChecker{__FILE__, #__VA_ARGS__, static_cast<unsigned int>(__LINE__)}, __VA_ARGS__)

// The variants below are expected to fail, but we don't want to pollute the state of
// `glGetError` so we have to call it anyway

    #define glCheckIgnoreWithFunc(errorFunc, ...) (::sf::priv::GlScopedCheckerIgnoreWithFunc{errorFunc}, __VA_ARGS__)

#else

    #define glCheck(...)                          (__VA_ARGS__)
    #define glCheckIgnoreWithFunc(errorFunc, ...) (__VA_ARGS__)

#endif


} // namespace sf::priv
