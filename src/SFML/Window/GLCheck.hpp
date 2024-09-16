#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// Let's define a macro to quickly check every OpenGL API call
////////////////////////////////////////////////////////////
#if defined(SFML_DEBUG) && !defined(SFML_SYSTEM_EMSCRIPTEN) // `glCheck` is prohibitively slow on Emscripten

// In debug mode, perform a test on every OpenGL call
// The do-while loop is needed so that glCheck can be used as a single statement in if/else branches

#define glCheck(...)                                                            \
    do                                                                          \
    {                                                                           \
        SFML_BASE_ASSERT(::sf::priv::glGetErrorImpl() == 0u /* GL_NO_ERROR */); \
                                                                                \
        __VA_ARGS__;                                                            \
                                                                                \
        while (!::sf::priv::glCheckError(__FILE__, __LINE__, #__VA_ARGS__))     \
            /* no-op */;                                                        \
                                                                                \
    } while (false)

#define glCheckExpr(...)                                                        \
    [&]                                                                         \
    {                                                                           \
        SFML_BASE_ASSERT(::sf::priv::glGetErrorImpl() == 0u /* GL_NO_ERROR */); \
                                                                                \
        auto _glCheckExprResult = __VA_ARGS__;                                  \
                                                                                \
        while (!::sf::priv::glCheckError(__FILE__, __LINE__, #__VA_ARGS__))     \
            /* no-op */;                                                        \
                                                                                \
        return _glCheckExprResult;                                              \
    }()

// The variants below are expected to fail, but we don't want to pollute the state of
// `glGetError` so we have to call it anyway

#define glCheckIgnoreWithFunc(errorFunc, ...)         \
    do                                                \
    {                                                 \
        SFML_BASE_ASSERT(errorFunc() == GL_NO_ERROR); \
                                                      \
        __VA_ARGS__;                                  \
                                                      \
        while (errorFunc() != GL_NO_ERROR)            \
            /* no-op */;                              \
                                                      \
    } while (false)

#define glCheckIgnoreExprWithFunc(errorFunc, ...)     \
    [&]                                               \
    {                                                 \
        SFML_BASE_ASSERT(errorFunc() == GL_NO_ERROR); \
                                                      \
        auto _glCheckExprResult = __VA_ARGS__;        \
                                                      \
        while (errorFunc() != GL_NO_ERROR)            \
            /* no-op */;                              \
                                                      \
        return _glCheckExprResult;                    \
    }()

#else

// Else, we don't add any overhead
#define glCheck(...)                              (__VA_ARGS__)
#define glCheckExpr(...)                          (__VA_ARGS__)
#define glCheckIgnoreWithFunc(errorFunc, ...)     (__VA_ARGS__)
#define glCheckIgnoreExprWithFunc(errorFunc, ...) (__VA_ARGS__)

#endif

////////////////////////////////////////////////////////////
/// \brief Internally calls and returns `glGetError`
///
/// Used to avoid including `gl.h` in this header
///
////////////////////////////////////////////////////////////
[[nodiscard]] unsigned int glGetErrorImpl();

////////////////////////////////////////////////////////////
/// \brief Check the last OpenGL error
///
/// \param file Source file where the call is located
/// \param line Line number of the source file where the call is located
/// \param expression The evaluated expression as a string
///
/// \return `false` if an error occurred, `true` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool glCheckError(const char* file, unsigned int line, const char* expression);

} // namespace sf::priv
