#include "SDLGlContext.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowImpl.hpp"

#include "SFML/System/Err.hpp"

#include <SDL3/SDL.h>


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
    int profile = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
    if (settings.isCore())
        profile = SDL_GL_CONTEXT_PROFILE_CORE;

    int flags = 0;
    if (settings.isDebug())
        flags |= SDL_GL_CONTEXT_DEBUG_FLAG;

    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_CONTEXT_PROFILE_MASK, profile);
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_CONTEXT_FLAGS, flags);
}

} // namespace


namespace sf::priv
{
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

    // Set context sharing attributes if a shared context is provided
    if (shared)
    {
        if (!SDL_GL_MakeCurrent(shared->m_window, shared->m_context))
        {
            err() << "Failed to activate shared SDL GL context: " << SDL_GetError();

            if (m_ownsWindow)
            {
                SDL_DestroyWindow(m_window);
                m_window     = nullptr;
                m_ownsWindow = false;
            }

            return;
        }

        SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    }

    // Create the OpenGL context
    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
    {
        err() << "Failed to create SDL GL context: " << SDL_GetError();

        if (m_ownsWindow)
        {
            SDL_DestroyWindow(m_window);
            m_window     = nullptr;
            m_ownsWindow = false;
        }

        return;
    }

    // Reset sharing attribute to default
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
}


////////////////////////////////////////////////////////////
SDLGlContext::SDLGlContext(const unsigned int     id,
                           SDLGlContext* const    shared,
                           const ContextSettings& settings,
                           const WindowImpl&      owner,
                           const unsigned int /* bitsPerPixel */) :
GlContext(id, settings),
m_window(owner.getSDLHandle()),
m_context(nullptr),
m_ownsWindow(false)
{
    applyContextSettings(m_settings);

    // Set context sharing attributes if a shared context is provided
    if (shared)
    {
        if (!SDL_GL_MakeCurrent(shared->m_window, shared->m_context))
        {
            err() << "Failed to activate shared SDL GL context: " << SDL_GetError();
            return;
        }

        SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    }

    // Create the OpenGL context
    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
    {
        err() << "Failed to create SDL GL context: " << SDL_GetError();
        return;
    }

    // Reset sharing attribute to default
    SFML_PRIV_TRY_SET_SDL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
}


////////////////////////////////////////////////////////////
SDLGlContext::~SDLGlContext()
{
    WindowContext::cleanupUnsharedFrameBuffers(*this);

    // Deactivate the context if it's current
    if (m_context && SDL_GL_GetCurrentContext() == m_context)
        SDL_GL_MakeCurrent(nullptr, nullptr);

    // Delete the context
    if (m_context)
        SDL_GL_DestroyContext(m_context);

    // Destroy the window if owned
    if (m_ownsWindow && m_window != nullptr)
        SDL_DestroyWindow(m_window);
}


////////////////////////////////////////////////////////////
GlFunctionPointer SDLGlContext::getFunction(const char* const name) const
{
    return SDL_GL_GetProcAddress(name);
}


////////////////////////////////////////////////////////////
bool SDLGlContext::makeCurrent(const bool current)
{
    if (current)
    {
        if (!SDL_GL_MakeCurrent(m_window, m_context))
        {
            err() << "Failed to activate SDL GL context: " << SDL_GetError();
            return false;
        }
    }
    else
    {
        if (!SDL_GL_MakeCurrent(nullptr, nullptr))
        {
            err() << "Failed to deactivate SDL GL context: " << SDL_GetError();
            return false;
        }
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
