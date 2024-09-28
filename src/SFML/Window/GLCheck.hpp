#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Traits/IsSame.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// Define a macro to quickly check every OpenGL API call
////////////////////////////////////////////////////////////
#if defined(SFML_DEBUG) && !defined(SFML_SYSTEM_EMSCRIPTEN) // `glCheck` is prohibitively slow on Emscripten

// In debug mode, perform a test on every OpenGL call
// The do-while loop is needed so that glCheck can be used as a single statement in if/else branches

[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr auto regularize(auto&& f)
{
    if constexpr (SFML_BASE_IS_SAME(decltype(f()), void))
    {
        f();
        return nullptr;
    }
    else
    {
        return f();
    }
}

#define glCheck(...)                                                            \
    [](auto&& f) __attribute__((always_inline, flatten))                        \
    {                                                                           \
        SFML_BASE_ASSERT(::sf::priv::glGetErrorImpl() == 0u /* GL_NO_ERROR */); \
                                                                                \
        auto _glCheckResult = ::sf::priv::regularize(f);                        \
                                                                                \
        while (!::sf::priv::glCheckError(__FILE__, __LINE__, #__VA_ARGS__))     \
            /* no-op */;                                                        \
                                                                                \
        return _glCheckResult;                                                  \
    }                                                                           \
    ([&]() __attribute__((always_inline, flatten)) { return __VA_ARGS__; })

// The variants below are expected to fail, but we don't want to pollute the state of
// `glGetError` so we have to call it anyway

#define glCheckIgnoreWithFunc(errorFunc, ...)                  \
    [&](auto&& f) __attribute__((always_inline, flatten))      \
    {                                                          \
        SFML_BASE_ASSERT(errorFunc() == 0u /* GL_NO_ERROR */); \
                                                               \
        auto _glCheckResult = ::sf::priv::regularize(f);       \
                                                               \
        while (errorFunc() != 0u /* GL_NO_ERROR */)            \
            /* no-op */;                                       \
                                                               \
        return _glCheckResult;                                 \
    }                                                          \
    ([&]() __attribute__((always_inline, flatten)) { return __VA_ARGS__; })

#else

// Else, we don't add any overhead
#define glCheck(...)                          (__VA_ARGS__)
#define glCheckIgnoreWithFunc(errorFunc, ...) (__VA_ARGS__)

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
