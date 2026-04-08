#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Window that serves as a target for OpenGL rendering
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API Window : public WindowBase
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Window creation settings
    ///
    ////////////////////////////////////////////////////////////
    using Settings = WindowSettings;

    ////////////////////////////////////////////////////////////
    /// \brief Create a new window from a settings aggregate
    ///
    /// Creates a new top-level window described by
    /// `windowSettings` and attaches an OpenGL context to it.
    /// On failure (invalid settings, OS error, ...) returns
    /// `base::nullOpt`.
    ///
    /// \param windowSettings Window creation parameters
    ///
    /// \return The newly created window on success, `base::nullOpt` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Window> create(const Settings& windowSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Create a window wrapping an existing native control
    ///
    /// Use this overload to attach an OpenGL rendering context
    /// to an already existing OS-level window/control. This is
    /// useful for embedding SFML rendering inside a larger
    /// application built with another GUI toolkit.
    ///
    /// The optional second parameter specifies advanced OpenGL
    /// context settings (depth/stencil bits, version, profile,
    /// etc.).
    ///
    /// \param handle          Platform-specific handle of the control to attach to
    /// \param contextSettings Additional settings for the underlying OpenGL context
    ///
    /// \return The newly wrapped window on success, `base::nullOpt` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Window> create(WindowHandle handle, const ContextSettings& contextSettings = {});

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Closes the window and frees all the resources attached to it.
    ///
    ////////////////////////////////////////////////////////////
    ~Window();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Window(const Window&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Window& operator=(const Window&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Window(Window&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Window& operator=(Window&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the settings of the OpenGL context of the window
    ///
    /// Note that these settings may be different from what was
    /// passed to the constructor or the `create()` function,
    /// if one or more settings were not supported. In this case,
    /// SFML chose the closest match.
    ///
    /// \return Structure containing the OpenGL context settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const ContextSettings& getSettings() const;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable vertical synchronization
    ///
    /// Activating vertical synchronization will limit the number
    /// of frames displayed to the refresh rate of the monitor.
    /// This can avoid some visual artifacts, and limit the framerate
    /// to a good value (but not constant across different computers).
    ///
    /// Vertical synchronization is disabled by default.
    ///
    /// \param enabled `true` to enable v-sync, `false` to deactivate it
    ///
    ////////////////////////////////////////////////////////////
    void setVerticalSyncEnabled(bool enabled);

    ////////////////////////////////////////////////////////////
    /// \brief Check whether vertical synchronization is enabled
    ///
    /// \return `true` if vertical synchronization is enabled, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isVerticalSyncEnabled() const;

    ////////////////////////////////////////////////////////////
    /// \brief Limit the framerate to a maximum fixed frequency
    ///
    /// If a limit is set, the window will use a small delay after
    /// each call to `display()` to ensure that the current frame
    /// lasted long enough to match the framerate limit.
    /// SFML will try to match the given limit as much as it can,
    /// but since it internally uses `sf::sleep`, whose precision
    /// depends on the underlying OS, the results may be a little
    /// imprecise as well (for example, you can get 65 FPS when
    /// requesting 60).
    ///
    /// \param limit Framerate limit, in frames per seconds (use 0 to disable limit)
    ///
    ////////////////////////////////////////////////////////////
    void setFramerateLimit(unsigned int limit);

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the window as the current target
    ///        for OpenGL rendering
    ///
    /// A window is active only on the current thread, if you want to
    /// make it active on another thread you have to deactivate it
    /// on the previous thread first if it was active.
    /// Only one window can be active on a thread at a time, thus
    /// the window previously active (if any) automatically gets deactivated.
    /// This is not to be confused with `requestFocus()`.
    ///
    /// \param active `true` to activate, `false` to deactivate
    ///
    /// \return `true` if operation was successful, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setActive(bool active = true) const;

    ////////////////////////////////////////////////////////////
    /// \brief Display on screen what has been rendered to the window so far
    ///
    /// This function is typically called after all OpenGL rendering
    /// has been done for the current frame, in order to show
    /// it on screen.
    ///
    ////////////////////////////////////////////////////////////
    void display();

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct a window and a GL context, and a window base
    ///        from either an existing handle or by creating a window
    ///        impl
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Window(base::PassKey<Window>&&,
                                  WindowBase&&    windowBase,
                                  const Settings& windowSettings,
                                  unsigned int    bitsPerPixel);

protected:
    [[nodiscard]] bool isMovedFrom() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Window
/// \ingroup window
///
/// `sf::Window` is the main class of the Window module. It defines
/// an OS window that is able to receive an OpenGL rendering.
///
/// A `sf::Window` can create its own new window, or be embedded into
/// an already existing control using the `create(handle)` function.
/// This can be useful for embedding an OpenGL rendering area into
/// a view which is part of a bigger GUI with existing windows,
/// controls, etc. It can also serve as embedding an OpenGL rendering
/// area into a window created by another (probably richer) GUI library
/// like Qt or wxWidgets.
///
/// The `sf::Window` class provides a simple interface for manipulating
/// the window: move, resize, show/hide, control mouse cursor, etc.
/// It also provides event handling through its `pollEvent()` and `waitEvent()`
/// functions.
///
/// Note that OpenGL experts can pass their own parameters (anti-aliasing
/// level, bits for the depth and stencil buffers, etc.) to the
/// OpenGL context attached to the window, with the `sf::ContextSettings`
/// structure which is passed as an optional argument when creating the
/// window.
///
/// On dual-graphics systems consisting of a low-power integrated GPU
/// and a powerful discrete GPU, the driver picks which GPU will run an
/// SFML application. In order to inform the driver that an SFML application
/// can benefit from being run on the more powerful discrete GPU,
/// `#SFML_DEFINE_DISCRETE_GPU_PREFERENCE` can be placed in a source file
/// that is compiled and linked into the final application. The macro
/// should be placed outside of any scopes in the global namespace.
///
/// Usage example:
/// \code
/// // Initialize the window context (must outlive any window)
/// auto windowContext = sf::WindowContext::create().value();
///
/// // Create a new window
/// auto window = sf::Window::create({.size = {800u, 600u}, .title = "SFML Window"}).value();
///
/// // Limit the framerate to 60 frames per second (this step is optional)
/// window.setFramerateLimit(60);
///
/// // The main loop - ends as soon as the window is closed
/// while (true)
/// {
///     // Event processing
///     while (const sf::base::Optional event = window.pollEvent())
///     {
///         // Request for closing the window
///         if (event->is<sf::Event::Closed>())
///             return 0; // break out of both loops
///     }
///
///     // Activate the window for OpenGL rendering
///     (void)window.setActive();
///
///     // OpenGL drawing commands go here...
///
///     // End the current frame and display its contents on screen
///     window.display();
/// }
/// \endcode
///
////////////////////////////////////////////////////////////
