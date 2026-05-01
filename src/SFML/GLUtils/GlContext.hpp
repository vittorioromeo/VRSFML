#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/ContextSettings.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
using GlFunctionPointer = void (*)();
class WindowContext;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class representing an OpenGL context
///
/// Provides the common interface used by `WindowContext` to manage
/// platform-specific GL contexts (such as `SDLGlContext` on desktop or
/// `EglContext` on Emscripten/Android). Each instance carries the
/// settings it was created with as well as a unique numeric ID
/// assigned by `WindowContext` for cross-context bookkeeping.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] GlContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Notifies `WindowContext` that this context is going away so
    /// that any associated bookkeeping can be cleaned up.
    ///
    ////////////////////////////////////////////////////////////
    virtual ~GlContext();

    ////////////////////////////////////////////////////////////
    GlContext(const GlContext&) = delete;

    ////////////////////////////////////////////////////////////
    GlContext& operator=(const GlContext&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Get the actual settings of the context
    ///
    /// These settings may differ from the ones passed to the
    /// constructor: they are adjusted post-creation to reflect what
    /// the underlying system actually granted (for example a lower
    /// version, fewer multisampling samples, or a different profile).
    ///
    /// \return Structure containing the effective context settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const ContextSettings& getSettings() const;

    ////////////////////////////////////////////////////////////
    /// \brief Present the rendered contents of the context (swap buffers)
    ///
    ////////////////////////////////////////////////////////////
    virtual void display() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable vertical synchronization
    ///
    /// Activating vertical synchronization caps the number of frames
    /// displayed to the refresh rate of the monitor. This avoids
    /// tearing and bounds the framerate, but the resulting cadence is
    /// not necessarily constant across different machines.
    ///
    /// \param enabled `true` to enable v-sync, `false` to disable it
    ///
    ////////////////////////////////////////////////////////////
    virtual void setVerticalSyncEnabled(bool enabled) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether vertical synchronization is enabled
    ///
    /// \return `true` if vertical synchronization is enabled, `false` otherwise
    ///
    /// \see `setVerticalSyncEnabled`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool isVerticalSyncEnabled() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Perform post-construction initialization of the context
    ///
    /// Loads enough function pointers from the shared context to query
    /// the actual OpenGL version, profile, and debug-flag attributes,
    /// and updates `m_settings` accordingly. Must be called after the
    /// context has been made current on the calling thread.
    ///
    /// \param sharedGlContext   A sibling context used to resolve GL
    ///                          function pointers (typically SFML's
    ///                          shared context)
    /// \param requestedSettings Settings originally requested at
    ///                          context creation time
    ///
    /// \return `true` on success, `false` if mandatory function
    ///         pointers could not be loaded or the GL version is
    ///         unsupported
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool initialize(const GlContext& sharedGlContext, const ContextSettings& requestedSettings);

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Construct a context with a numeric ID and initial settings
    ///
    /// Meant to be invoked by derived classes only. The settings will
    /// later be refined by `initialize()`.
    ///
    /// \param id              Unique numeric ID assigned by `WindowContext`
    /// \param contextSettings Initially requested context settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GlContext(unsigned int id, const ContextSettings& contextSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the context as the current rendering target
    ///
    /// \param activate `true` to make the context current on the
    ///                 calling thread, `false` to deactivate it
    ///
    /// \return `true` on success, `false` if the platform call failed
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool makeCurrent(bool activate) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the unique numeric ID assigned to this context
    ///
    /// \return The ID supplied to the constructor by `WindowContext`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getId() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ContextSettings m_settings; //!< Effective settings of the context (refined by `initialize()`)

private:
    friend WindowContext;

    ////////////////////////////////////////////////////////////
    /// \brief Warn if the created context does not match the requested settings
    ///
    /// Compares `m_settings` against the originally requested settings
    /// and emits a warning to `priv::err()` if version, depth/stencil
    /// bits, profile, or attribute flags do not match.
    ///
    /// \param requestedSettings Settings originally requested at
    ///                          context creation time
    ///
    ////////////////////////////////////////////////////////////
    void checkSettings(const ContextSettings& requestedSettings) const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const unsigned int m_id; //!< Unique numeric ID assigned by `WindowContext`
};

} // namespace sf::priv
