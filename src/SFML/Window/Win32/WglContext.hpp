#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GlContext.hpp"

#include "SFML/System/Vector2.hpp"

#include <glad/wgl.h>


namespace sf
{
struct ContextSettings;

namespace priv
{
class WindowImpl;

////////////////////////////////////////////////////////////
/// \brief Windows (WGL) implementation of OpenGL contexts
///
////////////////////////////////////////////////////////////
class WglContext : public GlContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new default context
    ///
    /// \param shared Context to share the new one with (can be a null pointer)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WglContext(unsigned int id, WglContext* shared);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context attached to a window
    ///
    /// \param shared       Context to share the new one with
    /// \param settings     Creation parameters
    /// \param owner        Pointer to the owner window
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WglContext(unsigned int      id,
                                      WglContext*       shared,
                                      ContextSettings   contextSettings,
                                      const WindowImpl& owner,
                                      unsigned int      bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~WglContext() override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the address of an OpenGL function
    ///
    /// \param name Name of the function to get the address of
    ///
    /// \return Address of the OpenGL function, `nullptr` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] GlFunctionPointer getFunction(const char* name) const;

    ////////////////////////////////////////////////////////////
    /// \brief Activate the context as the current target for rendering
    ///
    /// \param current Whether to make the context current or no longer current
    ///
    /// \return `true` on success, `false` if any error happened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool makeCurrent(bool current) override;

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
    /// \brief Select the best pixel format for a given set of settings
    ///
    /// \param deviceContext Device context
    /// \param bitsPerPixel  Pixel depth, in bits per pixel
    /// \param settings      Requested context settings
    /// \param pbuffer       Whether the pixel format should support pbuffers
    ///
    /// \return The best pixel format
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static int selectBestPixelFormat(HDC                    deviceContext,
                                                   unsigned int           bitsPerPixel,
                                                   const ContextSettings& contextSettings,
                                                   bool                   pbuffer = false);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Data associated with a WGL surface
    ///
    ////////////////////////////////////////////////////////////
    struct SurfaceData
    {
        HWND        window{};        //!< Window to which the context is attached
        HPBUFFERARB pbuffer{};       //!< Handle to a pbuffer if one was created
        HDC         deviceContext{}; //!< Device context associated to the context
        bool        ownsWindow{};    //!< Do we own the target window?
    };

    ////////////////////////////////////////////////////////////
    /// \brief Create the context and initialize extensions
    ///
    /// \param shared   Context to share the new one with
    /// \param settings Creation parameters
    ///
    ////////////////////////////////////////////////////////////
    explicit WglContext(unsigned int id, WglContext* shared, const ContextSettings& settings, const SurfaceData& surfaceData);

    ////////////////////////////////////////////////////////////
    /// \brief Set the pixel format of the device context
    ///
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    static void setDevicePixelFormat(const ContextSettings& contextSettings, HDC deviceContext, unsigned int bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Update the context settings from the selected pixel format
    ///
    ////////////////////////////////////////////////////////////
    static void updateSettingsFromPixelFormat(ContextSettings& settings, HDC deviceContext);

    ////////////////////////////////////////////////////////////
    /// \brief Create the context's drawing surface
    ///
    /// \param shared       Shared context (can be a null pointer)
    /// \param size         Back buffer width and height, in pixels
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SurfaceData createSurface(ContextSettings settings,
                                                   WglContext*     shared,
                                                   Vector2u        size,
                                                   unsigned int    bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Create the context's drawing surface from an existing window
    ///
    /// \param window       Window handle of the owning window
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static SurfaceData createSurface(ContextSettings& settings, HWND window, unsigned int bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Create the context
    ///
    /// \param shared Context to share the new one with (can be a null pointer)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static HGLRC createContext(ContextSettings& settings, const SurfaceData& surfaceData, WglContext* shared);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    SurfaceData m_surfaceData;
    HGLRC       m_context{}; //!< OpenGL context
};

} // namespace priv
} // namespace sf
