#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/Window.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Vector2.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Image;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Window that can serve as a target for 2D drawing
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API RenderWindow : public Window, public RenderTarget
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Window creation settings
    ///
    ////////////////////////////////////////////////////////////
    using Settings = WindowSettings;

    ////////////////////////////////////////////////////////////
    /// \brief Construct a new window
    ///
    /// Creates the render window with the specified \a windowSettings.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RenderWindow(const Settings& windowSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Construct the window from an existing control
    ///
    /// Use this constructor if you want to create an SFML
    /// rendering area into an already existing control.
    ///
    /// The second parameter is an optional structure specifying
    /// advanced OpenGL context settings such as anti-aliasing,
    /// depth-buffer bits, etc. You shouldn't care about these
    /// parameters for a regular usage of the graphics module.
    ///
    /// \param handle   Platform-specific handle of the control (\a HWND on
    ///                 Windows, \a %Window on Linux/FreeBSD, \a NSWindow on macOS)
    /// \param settings Additional settings for the underlying OpenGL context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RenderWindow(WindowHandle handle, const ContextSettings& contextSettings = {});

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderWindow(const RenderWindow&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderWindow& operator=(const RenderWindow&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderWindow(RenderWindow&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderWindow& operator=(RenderWindow&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    ~RenderWindow() override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the rendering region of the window
    ///
    /// The size doesn't include the titlebar and borders
    /// of the window.
    ///
    /// \return Size in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2u getSize() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// The OS default icon is used by default.
    ///
    /// \param icon Image to use as the icon. The image is copied,
    ///             so you need not keep the source alive after
    ///             calling this function.
    ///
    ////////////////////////////////////////////////////////////
    void setIcon(const Image& icon);
    using Window::setIcon;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the window will use sRGB encoding when drawing on it
    ///
    /// You can request sRGB encoding for a window by having the sRgbCapable flag set in the `ContextSettings`
    ///
    /// \return `true` if the window use sRGB encoding, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isSrgb() const override;

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
    [[nodiscard]] bool setActive(bool active = true) override;

    ////////////////////////////////////////////////////////////
    /// \brief Sets the size of the window and forwards to `WindowBase::setSize`
    ///
    /// \see WindowBase::setSize
    ///
    ////////////////////////////////////////////////////////////
    void setSize(const Vector2u& size);

    ////////////////////////////////////////////////////////////
    /// \brief Polls the next event and forwards to `WindowBase::pollEvent`
    ///
    /// \see WindowBase::pollEvent
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> pollEvent();

    ////////////////////////////////////////////////////////////
    /// \brief Waits for the next event and forwards to `WindowBase::waitEvent`
    ///
    /// \see WindowBase::waitEvent
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> waitEvent(Time timeout = Time::Zero);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void display();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Construct a new window
    ///
    ////////////////////////////////////////////////////////////
    template <typename... TWindowArgs>
    [[nodiscard]] explicit RenderWindow(int /* disambiguator */, TWindowArgs&&... windowArgs);

    ////////////////////////////////////////////////////////////
    /// \brief Function called after the window has been resized
    ///
    /// This function is called so that derived classes can
    /// perform custom actions when the size of the window changes.
    ///
    ////////////////////////////////////////////////////////////
    void onResize();

    ////////////////////////////////////////////////////////////
    /// \brief Processes an event before it is sent to the user
    ///
    /// This function is called every time an event is received
    /// from the internal window (through pollEvent or waitEvent).
    /// It filters out unwanted events, and performs whatever internal
    /// stuff the window needs before the event is returned to the
    /// user.
    ///
    /// \param event Event to filter
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> filterEvent(base::Optional<Event> event);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int m_defaultFrameBuffer{}; //!< Framebuffer to bind when targeting this window
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderWindow
/// \ingroup graphics
///
/// `sf::RenderWindow` is the main class of the Graphics module.
/// It defines an OS window that can be painted using the other
/// classes of the graphics module.
///
/// `sf::RenderWindow` is derived from `sf::Window`, thus it inherits
/// all its features: events, window management, OpenGL rendering,
/// etc. See the documentation of `sf::Window` for a more complete
/// description of all these features, as well as code examples.
///
/// On top of that, `sf::RenderWindow` adds more features related to
/// 2D drawing with the graphics module (see its base class
/// `sf::RenderTarget` for more details).
/// Here is a typical rendering and event loop with a `sf::RenderWindow`:
///
/// \code
/// // Declare and create a new render-window
/// sf::RenderWindow window({.size{800u, 600u}, .title = "SFML Window"});
///
/// // Limit the framerate to 60 frames per second (this step is optional)
/// window.setFramerateLimit(60);
///
/// // The main loop - ends as soon as the window is closed
/// while (true)
/// {
///    // Event processing
///    while (const sf::base::Optional event = window.pollEvent())
///    {
///        // Request for closing the window
///        if (event->is<sf::Event::Closed>())
///            return 0; // break out of both loops
///    }
///
///    // Clear the whole window before rendering a new frame
///    window.clear();
///
///    // Draw some graphical entities
///    window.draw(sprite);
///    window.draw(circle);
///    window.draw(text);
///
///    // End the current frame and display its contents on screen
///    window.display();
/// }
/// \endcode
///
/// Like `sf::Window`, `sf::RenderWindow` is still able to render direct
/// OpenGL stuff. It is even possible to mix together OpenGL calls
/// and regular SFML drawing commands.
///
/// \code
/// // Create the render window
/// sf::RenderWindow window({.size{800u, 600u}, .title = "SFML OpenGL"});
///
/// // Create a sprite and a text to display
/// const auto texture = sf::Texture::loadFromFile("circle.png").value();
/// sf::Sprite sprite(texture);
/// const auto font = sf::Font::openFromFile("arial.ttf").value();
/// sf::Text text(font);
/// ...
///
/// // Perform OpenGL initializations
/// glMatrixMode(GL_PROJECTION);
/// ...
///
/// // Start the rendering loop
/// while (true)
/// {
///     // Process events
///     ...
///
///     // Draw a background sprite
///     window.resetGLStates();
///     window.draw(sprite, texture);
///
///     // Draw a 3D object using OpenGL
///     glBegin(GL_TRIANGLES);
///         glVertex3f(...);
///         ...
///     glEnd();
///
///     // Draw text on top of the 3D object
///     window.resetGLStates();
///     window.draw(text);
///
///     // Finally, display the rendered frame on screen
///     window.display();
/// }
/// \endcode
///
/// \see `sf::Window`, `sf::RenderTarget`, `sf::RenderTexture`, `sf::View`
///
////////////////////////////////////////////////////////////
