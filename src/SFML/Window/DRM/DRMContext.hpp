#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/EGL/EGLCheck.hpp"
#include "SFML/Window/GlContext.hpp"

#include <glad/egl.h>

#include <gbm.h>
#include <xf86drmMode.h>


namespace sf::priv
{
struct Drm
{
    int fileDescriptor{};

    drmModeModeInfoPtr mode{};
    base::U32          crtcId{};
    base::U32          connectorId{};

    drmModeCrtcPtr originalCrtc{};

    drmModeConnectorPtr savedConnector{};
    drmModeEncoderPtr   savedEncoder{};
};

class WindowImplDRM;

class DRMContext : public GlContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new context, not associated to a window
    ///
    /// \param shared Context to share the new one with (can be `nullptr`)
    ///
    ////////////////////////////////////////////////////////////
    DRMContext(DRMContext* shared);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context attached to a window
    ///
    /// \param shared       Context to share the new one with
    /// \param settings     Creation parameters
    /// \param owner        Pointer to the owner window
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    DRMContext(DRMContext* shared, const ContextSettings& contextSettings, const WindowImpl& owner, unsigned int bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~DRMContext() override;

    ////////////////////////////////////////////////////////////
    /// \brief Activate the context as the current target
    ///        for rendering
    ///
    /// \param current Whether to make the context current or no longer current
    ///
    /// \return `true` on success, `false` if any error happened
    ///
    ////////////////////////////////////////////////////////////
    bool makeCurrent(bool current) override;

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
    /// \param enabled: `true` to enable v-sync, `false` to deactivate
    ///
    ////////////////////////////////////////////////////////////
    void setVerticalSyncEnabled(bool enabled) override;

    ////////////////////////////////////////////////////////////
    /// \brief Create the EGL context
    ///
    /// \param shared       Context to share the new one with (can be `nullptr`)
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    /// \param settings     Creation parameters
    ///
    ////////////////////////////////////////////////////////////
    void createContext(DRMContext* shared);

    ////////////////////////////////////////////////////////////
    /// \brief Create the EGL surface
    ///
    /// \param size    Back buffer width and height, in pixels
    /// \param scanout True to present the surface to the screen
    ///
    ////////////////////////////////////////////////////////////
    void createSurface(Vector2u size, bool scanout);

    ////////////////////////////////////////////////////////////
    /// \brief Destroy the EGL surface
    ///
    /// This function must be called when the activity is stopped, or
    /// when the orientation change.
    ///
    ////////////////////////////////////////////////////////////
    void destroySurface();

    ////////////////////////////////////////////////////////////
    /// \brief Get the best EGL visual for a given set of video settings
    ///
    /// \param display      EGL display
    /// \param settings     Requested context settings
    ///
    /// \return The best EGL config
    ///
    ////////////////////////////////////////////////////////////
    static EGLConfig getBestConfig(EGLDisplay display, unsigned int bitsPerPixel, const ContextSettings& contextSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Get the address of an OpenGL function
    ///
    /// \param name Name of the function to get the address of
    ///
    /// \return Address of the OpenGL function, 0 on failure
    ///
    ////////////////////////////////////////////////////////////
    GlFunctionPointer getFunction(const char* name);

protected:
    friend class VideoModeImpl;
    friend class WindowImplDRM;

    ////////////////////////////////////////////////////////////
    /// \brief Get Direct Rendering Manager pointer
    ///
    ////////////////////////////////////////////////////////////
    static Drm& getDRM();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Helper to copy the picked EGL configuration
    ///
    ////////////////////////////////////////////////////////////
    void updateSettings();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    EGLDisplay m_display{EGL_NO_DISPLAY}; ///< The internal EGL display
    EGLContext m_context{EGL_NO_CONTEXT}; ///< The internal EGL context
    EGLSurface m_surface{EGL_NO_SURFACE}; ///< The internal EGL surface
    EGLConfig  m_config{};                ///< The internal EGL config

    gbm_bo*      m_currentBO{};
    gbm_bo*      m_nextBO{};
    gbm_surface* m_gbmSurface{};
    bool         m_shown{};
    bool         m_scanOut{};
};

} // namespace sf::priv
