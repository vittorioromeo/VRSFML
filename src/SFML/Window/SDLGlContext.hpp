#pragma once
#include "SFML/GLUtils/GlContext.hpp"

#include <SFML/Copyright.hpp>

////////////////////////////////////////////////////////////
struct SDL_Window;
struct SDL_GLContextState;
using SDL_GLContext = SDL_GLContextState*;

namespace sf::priv
{
class WindowImpl;

////////////////////////////////////////////////////////////
class SDLGlContext : public GlContext
{
public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SDLGlContext(unsigned int id, SDLGlContext* shared, const ContextSettings& settings);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SDLGlContext(unsigned int           id,
                                        SDLGlContext*          shared,
                                        const ContextSettings& settings,
                                        const WindowImpl&      owner,
                                        unsigned int           bitsPerPixel);

    ////////////////////////////////////////////////////////////
    ~SDLGlContext() override;

    ////////////////////////////////////////////////////////////
    GlFunctionPointer getFunction(const char* name) const;

    ////////////////////////////////////////////////////////////
    bool makeCurrent(bool current) override;

    ////////////////////////////////////////////////////////////
    void display() override;

    ////////////////////////////////////////////////////////////
    void setVerticalSyncEnabled(bool enabled) override;

private:
    ////////////////////////////////////////////////////////////
    SDL_Window*   m_window;     // SDL window associated with the context
    SDL_GLContext m_context;    // SDL OpenGL context handle
    bool          m_ownsWindow; // Whether the context owns the window (for offscreen contexts)
};
} // namespace sf::priv
