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
/// \brief Abstract class representing an OpenGL context
///
////////////////////////////////////////////////////////////
class [[nodiscard]] GlContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~GlContext();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    GlContext(const GlContext&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    GlContext& operator=(const GlContext&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Get the settings of the context
    ///
    /// Note that these settings may be different than the ones
    /// passed to the constructor; they are indeed adjusted if the
    /// original settings are not directly supported by the system.
    ///
    /// \return Structure containing the settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const ContextSettings& getSettings() const;

    ////////////////////////////////////////////////////////////
    /// \brief Display what has been rendered to the context so far
    ///
    ////////////////////////////////////////////////////////////
    virtual void display() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable vertical synchronization
    ///
    /// Activating vertical synchronization will limit the number
    /// of frames displayed to the refresh rate of the monitor.
    /// This can avoid some visual artifacts, and limit the framerate
    /// to a good value (but not constant across different computers).
    ///
    /// \param enabled `true` to enable v-sync, `false` to deactivate
    ///
    ////////////////////////////////////////////////////////////
    virtual void setVerticalSyncEnabled(bool enabled) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether vertical synchronization is enabled
    ///
    /// \return `true` if vertical synchronization is enabled, `false` otherwise
    ///
    /// \see setVerticalSyncEnabled
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool isVerticalSyncEnabled() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Perform various initializations after the context construction
    /// \param requestedSettings Requested settings during context creation
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool initialize(const GlContext& sharedGlContext, const ContextSettings& requestedSettings);

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructor is meant for derived classes only.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GlContext(unsigned int id, const ContextSettings& contextSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Activate the context as the current target
    ///        for rendering
    ///
    /// \param activate Whether to make the context current or no longer current
    ///
    /// \return `true` on success, `false` if any error happened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool makeCurrent(bool activate) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the OpenGL context ID for this context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getId() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ContextSettings m_settings; //!< Creation settings of the context

private:
    friend WindowContext;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the context is compatible with the requested settings
    /// \param requestedSettings Requested settings during context creation
    ///
    ////////////////////////////////////////////////////////////
    void checkSettings(const ContextSettings& requestedSettings) const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const unsigned int m_id;
};

} // namespace sf::priv
