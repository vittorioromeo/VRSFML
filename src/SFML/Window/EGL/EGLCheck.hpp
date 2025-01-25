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
/// Macro to check every EGL API call
////////////////////////////////////////////////////////////
#ifdef SFML_DEBUG

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

// In debug mode, perform a test on every EGL call
// The do-while loop is needed so that glCheck can be used as a single statement in if/else branches
#define eglCheck(...)                                                        \
    [](auto&& f) __attribute__((always_inline, flatten))                     \
    {                                                                        \
        SFML_BASE_ASSERT(::eglGetError() == EGL_SUCCESS);                    \
                                                                             \
        auto sfPrivEglCheckResult = ::sf::priv::regularize(f);                    \
                                                                             \
        while (!::sf::priv::eglCheckError(__FILE__, __LINE__, #__VA_ARGS__)) \
            /* no-op */;                                                     \
                                                                             \
        return sfPrivEglCheckResult;                                              \
    }                                                                        \
    ([&]() __attribute__((always_inline, flatten)) { return __VA_ARGS__; })

#else

// Else, we don't add any overhead
#define eglCheck(...) (__VA_ARGS__)

#endif

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
[[nodiscard]] bool eglCheckError(const char* file, unsigned int line, const char* expression);

} // namespace sf::priv
