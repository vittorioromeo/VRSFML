#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/SDLGlContext.hpp"
#include "SFML/Window/SDLWindowImpl.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Err.hpp"

#include <SDL3/SDL_video.h>


////////////////////////////////////////////////////////////
#define SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(attribute, value)                                              \
    if (!SDL_GL_SetAttribute(attribute, value))                                                        \
    {                                                                                                  \
        ::sf::priv::err() << "Failed to set SDL attribute '" << #attribute << "': " << SDL_GetError(); \
    }


namespace
{
////////////////////////////////////////////////////////////
void applyContextSettings(const sf::ContextSettings& settings)
{
    // Set context flags
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, settings.sRgbCapable ? 1 : 0);
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_DEPTH_SIZE, static_cast<int>(settings.depthBits));
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_STENCIL_SIZE, static_cast<int>(settings.stencilBits));
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_DOUBLEBUFFER, 1);
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_MULTISAMPLEBUFFERS, settings.antiAliasingLevel > 0u ? 1 : 0);
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_MULTISAMPLESAMPLES, static_cast<int>(settings.antiAliasingLevel));

    // Set context version
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_CONTEXT_MAJOR_VERSION, static_cast<int>(settings.majorVersion));
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_CONTEXT_MINOR_VERSION, static_cast<int>(settings.minorVersion));

    // Set context flags
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_CONTEXT_PROFILE_MASK,
                                    settings.isCore() ? SDL_GL_CONTEXT_PROFILE_CORE : SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_CONTEXT_FLAGS, settings.isDebug() ? SDL_GL_CONTEXT_DEBUG_FLAG : 0);
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
void SDLGlContext::destroyWindowIfNeeded()
{
    if (!m_ownsWindow || m_window == nullptr)
        return;

    SDL_DestroyWindow(m_window);

    m_window     = nullptr;
    m_ownsWindow = false;
}


////////////////////////////////////////////////////////////
void SDLGlContext::initContext(SDLGlContext* const shared)
{
    // Set context sharing attributes if a shared context is provided
    if (shared != nullptr)
    {
        if (!shared->makeCurrent(true))
        {
            destroyWindowIfNeeded();
            return;
        }

        // The next created context will be shared with the current one
        SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    }

    // Create the OpenGL context
    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
    {
        err() << "Failed to create SDL GL context: " << SDL_GetError();
        destroyWindowIfNeeded();
        return;
    }

    // Reset sharing attribute to default
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
}


////////////////////////////////////////////////////////////
#undef SFML_PRIV_TRY_SET_SDL_ATTRIBUTE


////////////////////////////////////////////////////////////
SDLGlContext::SDLGlContext(const unsigned int id, SDLGlContext* const shared, const ContextSettings& settings) :
GlContext(id, settings),
m_window(nullptr),
m_context(nullptr),
m_ownsWindow(false)
{
    applyContextSettings(m_settings);

    // Create a hidden window for the context
    m_window = SDL_CreateWindow("", 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (m_window == nullptr)
    {
        err() << "Failed to create hidden window for SDLGlContext: " << SDL_GetError();
        return;
    }

    m_ownsWindow = true;
    initContext(shared);
}


////////////////////////////////////////////////////////////
SDLGlContext::SDLGlContext(const unsigned int     id,
                           SDLGlContext* const    shared,
                           const ContextSettings& settings,
                           const SDLWindowImpl&   owner,
                           const unsigned int /* bitsPerPixel */) :
GlContext(id, settings),
m_window(owner.getSDLHandle()),
m_context(nullptr),
m_ownsWindow(false)
{
    applyContextSettings(m_settings);
    initContext(shared);
}


////////////////////////////////////////////////////////////
SDLGlContext::~SDLGlContext()
{
    WindowContext::cleanupUnsharedFrameBuffers(*this);

    // Deactivate the context if it's current
    if (m_context && SDL_GL_GetCurrentContext() == m_context)
        (void)makeCurrent(false);

    // Delete the context
    if (m_context)
        SDL_GL_DestroyContext(m_context);

    // Destroy the window if owned
    destroyWindowIfNeeded();
}


////////////////////////////////////////////////////////////
GlFunctionPointer SDLGlContext::getFunction(const char* const name) const
{
    return SDL_GL_GetProcAddress(name);
}


////////////////////////////////////////////////////////////
bool SDLGlContext::makeCurrent(const bool activate)
{
    auto*       targetWindow  = activate ? m_window : nullptr;
    auto*       targetContext = activate ? m_context : nullptr;
    const char* targetAction  = activate ? "activate" : "deactivate";

    if (!SDL_GL_MakeCurrent(targetWindow, targetContext))
    {
        err() << "Failed to " << targetAction << " SDL GL context: " << SDL_GetError();
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
void SDLGlContext::display()
{
    SDL_GL_SwapWindow(m_window);
}


////////////////////////////////////////////////////////////
void SDLGlContext::setVerticalSyncEnabled(const bool enabled)
{
    if (!SDL_GL_SetSwapInterval(enabled ? 1 : 0))
        err() << "Failed to set vertical sync: " << SDL_GetError();
}

} // namespace sf::priv
