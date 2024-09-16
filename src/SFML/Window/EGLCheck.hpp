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
/// Let's define a macro to quickly check every EGL API call
////////////////////////////////////////////////////////////
#ifdef SFML_DEBUG

// In debug mode, perform a test on every EGL call
// The do-while loop is needed so that glCheck can be used as a single statement in if/else branches
#define eglCheck(...)                                                        \
    do                                                                       \
    {                                                                        \
        SFML_BASE_ASSERT(eglGetError() == EGL_SUCCESS);                      \
                                                                             \
        __VA_ARGS__;                                                         \
                                                                             \
        while (!::sf::priv::eglCheckError(__FILE__, __LINE__, #__VA_ARGS__)) \
            /* no-op */;                                                     \
                                                                             \
    } while (false)

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
