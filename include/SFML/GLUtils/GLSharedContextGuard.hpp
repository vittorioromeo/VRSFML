#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/FramebufferSaver.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
class GlContext;
} // namespace sf::priv


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief RAII guard that temporarily activates SFML's shared GL context
///
/// Used to perform GL operations that must run on the shared context
/// (typically resource creation/destruction such as `glGenBuffers` or
/// `glDeleteTextures`) regardless of which window context happens to
/// be active. On construction:
///
/// - The currently active framebuffer bindings are snapshotted via
///   `FramebufferSaver` (so they survive the context switch)
/// - The previously active GL context pointer is captured
/// - The shared GL context is made current on the calling thread
///
/// On destruction, the original GL context is reactivated and the
/// framebuffer bindings are restored.
///
////////////////////////////////////////////////////////////
class GLSharedContextGuard
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor: snapshot state and switch to the shared context
    ///
    /// Saves the current framebuffer bindings and active GL context,
    /// then makes the shared GL context current on the calling thread.
    ///
    ////////////////////////////////////////////////////////////
    GLSharedContextGuard();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor: restore the previously active GL context and bindings
    ///
    ////////////////////////////////////////////////////////////
    ~GLSharedContextGuard();

    ////////////////////////////////////////////////////////////
    GLSharedContextGuard(const GLSharedContextGuard&)            = delete;
    GLSharedContextGuard& operator=(const GLSharedContextGuard&) = delete;

    ////////////////////////////////////////////////////////////
    GLSharedContextGuard(GLSharedContextGuard&&)            = delete;
    GLSharedContextGuard& operator=(GLSharedContextGuard&&) = delete;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    FramebufferSaver m_fboSaver;  //!< Snapshots framebuffer bindings across the context switch
    GlContext*       m_glContext; //!< Saved active context, restored on destruction
};

} // namespace sf::priv
