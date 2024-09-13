#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/GlContext.hpp"

////////////////////////////////////////////////////////////
/// Predefine OBJC classes
////////////////////////////////////////////////////////////
#ifdef __OBJC__

@class NSOpenGLContext;
using NSOpenGLContextRef = NSOpenGLContext*;

@class NSOpenGLView;
using NSOpenGLViewRef = NSOpenGLView*;

@class NSWindow;
using NSWindowRef = NSWindow*;

#else // If C++

using NSOpenGLContextRef = void*;
using NSOpenGLViewRef    = void*;
using NSWindowRef        = void*;

#endif


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief macOS (Cocoa) implementation of OpenGL contexts
///
////////////////////////////////////////////////////////////
class SFContext : public GlContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new context, not associated to a window
    ///
    /// \param shared Context to share the new one with (can be a null pointer)
    ///
    ////////////////////////////////////////////////////////////
    SFContext(SFContext* shared);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context attached to a window
    ///
    /// \param shared       Context to share the new one with
    /// \param settings     Creation parameters
    /// \param owner        Pointer to the owner window
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    SFContext(SFContext* shared, const ContextSettings& contextSettings, const WindowImpl& owner, unsigned int bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SFContext() override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the address of an OpenGL function
    ///
    /// \param name Name of the function to get the address of
    ///
    /// \return Address of the OpenGL function, 0 on failure
    ///
    ////////////////////////////////////////////////////////////
    GlFunctionPointer getFunction(const char* name);

    ////////////////////////////////////////////////////////////
    /// \brief Display what has been rendered to the context so far
    ///
    ////////////////////////////////////////////////////////////
    void display() override;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable vertical synchronization
    ///
    /// Activating vertical synchronization will limit the number
    /// of frames displayed to the refresh rate of the monitor.
    /// This can avoid some visual artifacts, and limit the framerate
    /// to a good value (but not constant across different computers).
    ///
    /// \param enabled True to enable v-sync, false to deactivate
    ///
    ////////////////////////////////////////////////////////////
    void setVerticalSyncEnabled(bool enabled) override;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Activate the context as the current target
    ///        for rendering
    ///
    /// \param current Whether to make the context current or no longer current
    ///
    /// \return True on success, false if any error happened
    ///
    ////////////////////////////////////////////////////////////
    bool makeCurrent(bool current) override;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Create the context
    /// \note Must only be called from Ctor.
    ///
    /// \param shared       Context to share the new one with (can be a null pointer)
    /// \param bitsPerPixel bpp
    /// \param settings     Creation parameters
    ///
    ////////////////////////////////////////////////////////////
    void createContext(SFContext* shared, unsigned int bitsPerPixel, const ContextSettings& contextSettings);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    NSOpenGLContextRef m_context{}; ///< OpenGL context.
    NSOpenGLViewRef    m_view{};    ///< Only for offscreen context.
    NSWindowRef        m_window{};  ///< Only for offscreen context.
};

} // namespace sf::priv
