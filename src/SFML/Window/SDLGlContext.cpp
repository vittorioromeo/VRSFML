// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/SDLGlContext.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/SDLWindowImpl.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Err.hpp"

#include <SDL3/SDL_video.h>


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
    auto& sdlLayer = getSDLLayerSingleton();

    // Set context sharing attributes if a shared context is provided
    if (shared != nullptr)
    {
        if (!shared->makeCurrent(true))
        {
            destroyWindowIfNeeded();
            return;
        }

        // The next created context will be shared with the current one
        if (!sdlLayer.setGLAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1))
            err() << "Failed to set shared GL context attribute";
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
    if (!sdlLayer.setGLAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0))
        err() << "Failed to reset shared GL context attribute";
}


////////////////////////////////////////////////////////////
SDLGlContext::SDLGlContext(const unsigned int id, SDLGlContext* const shared, const ContextSettings& settings) :
    GlContext(id, settings),
    m_window(nullptr),
    m_context(nullptr),
    m_ownsWindow(false)
{
    if (!getSDLLayerSingleton().applyGLContextSettings(m_settings))
        err() << "Failed to apply SDL GL context settings for shared GL context hidden window";

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


////////////////////////////////////////////////////////////
bool SDLGlContext::isVerticalSyncEnabled() const
{
    int interval{};

    if (!SDL_GL_GetSwapInterval(&interval))
    {
        err() << "Failed to get vertical sync: " << SDL_GetError();
        return false;
    }

    return interval != 0;
}


} // namespace sf::priv
