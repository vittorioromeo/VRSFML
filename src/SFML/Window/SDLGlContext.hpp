#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GlContext.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct SDL_Window;
struct SDL_GLContextState;
using SDL_GLContext = SDL_GLContextState*;

namespace sf::priv
{
class SDLWindowImpl;
}


namespace sf::priv
{
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
                                        const SDLWindowImpl&   owner,
                                        unsigned int           bitsPerPixel);

    ////////////////////////////////////////////////////////////
    ~SDLGlContext() override;

    ////////////////////////////////////////////////////////////
    SDLGlContext(const SDLGlContext&)            = delete;
    SDLGlContext& operator=(const SDLGlContext&) = delete;

    ////////////////////////////////////////////////////////////
    SDLGlContext(SDLGlContext&&)            = delete;
    SDLGlContext& operator=(SDLGlContext&&) = delete;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] GlFunctionPointer getFunction(const char* name) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool makeCurrent(bool activate) override;

    ////////////////////////////////////////////////////////////
    void display() override;

    ////////////////////////////////////////////////////////////
    void setVerticalSyncEnabled(bool enabled) override;

private:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void initContext(SDLGlContext* shared);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void destroyWindowIfNeeded();

    ////////////////////////////////////////////////////////////
    SDL_Window*   m_window;     // SDL window associated with the context
    SDL_GLContext m_context;    // SDL OpenGL context handle
    bool          m_ownsWindow; // Whether the context owns the window (for offscreen contexts)
};
} // namespace sf::priv
