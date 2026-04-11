#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/Window.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"


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
    /// \brief Create a new render window
    ///
    /// Creates the render window described by `windowSettings`
    /// (size, title, style, context settings, ...). The OS window
    /// is opened immediately and an associated OpenGL context is
    /// created and made current.
    ///
    /// \param windowSettings Window creation parameters
    ///
    /// \return Render window on success, `base::nullOpt` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<RenderWindow> create(const Settings& windowSettings);

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
    [[nodiscard]] static base::Optional<RenderWindow> create(WindowHandle           handle,
                                                             const ContextSettings& contextSettings = {});

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
    /// \brief Destructor
    ///
    /// Closes the underlying OS window and releases the
    /// associated OpenGL context.
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
    [[nodiscard]] Vec2u getSize() const override;

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
    /// \return Always `false`. Modern window swapchains do not use sRGB.
    ///         Use an `sf::RenderTexture` if you need sRGB encoding.
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
    /// \brief Display the rendered frame on screen
    ///
    /// Flushes any pending auto-batched draw calls, swaps the
    /// front and back buffers of the underlying window, and
    /// returns statistics describing the work that the GPU just
    /// performed for this frame. Call this exactly once at the
    /// end of every frame.
    ///
    /// \return Per-frame draw call and vertex statistics
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget::DrawStatistics display();

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct a new window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RenderWindow(base::PassKey<RenderWindow>&&, Window&& window);

private:
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
/// Here is a typical rendering and event loop with a
/// `sf::RenderWindow`:
///
/// \code
/// // Declare and create a new render window.
/// auto window = sf::RenderWindow::create({.size = {800u, 600u}, .title = "SFML Window"}).value();
///
/// // Limit the framerate to 60 frames per second (optional).
/// window.setFramerateLimit(60u);
///
/// // The main loop -- ends as soon as the window is closed.
/// while (true)
/// {
///     // Event processing.
///     while (const sf::base::Optional event = window.pollEvent())
///     {
///         // Request for closing the window.
///         if (event->is<sf::Event::Closed>())
///             return 0; // break out of both loops
///     }
///
///     // Clear the whole window before rendering a new frame.
///     window.clear();
///
///     // Draw some graphical entities.
///     window.draw(sprite, texture); // texture passed at draw time in VRSFML
///     window.draw(circle);
///     window.draw(text);
///
///     // End the current frame and display its contents on screen.
///     window.display();
/// }
/// \endcode
///
/// Like `sf::Window`, `sf::RenderWindow` is still able to render
/// direct OpenGL geometry. It is even possible to mix together
/// custom OpenGL calls (using OpenGL ES 3.1 only -- the legacy
/// fixed-function pipeline is not available in VRSFML) and
/// regular VRSFML drawing commands. When doing so, call
/// `resetGLStates` between the two so that VRSFML can re-establish
/// the OpenGL state it expects.
///
/// \code
/// // Create the render window.
/// auto window = sf::RenderWindow::create({.size = {800u, 600u}, .title = "SFML OpenGL"}).value();
///
/// // Load some assets to display.
/// const auto       texture = sf::Texture::loadFromFile("circle.png").value();
/// const sf::Sprite sprite{.textureRect = texture.getRect()};
///
/// const auto     font = sf::Font::openFromFile("arial.ttf").value();
/// const sf::Text text{font, {.string = "Hello", .characterSize = 32u}};
///
/// // Perform OpenGL initializations (modern OpenGL only).
/// // ...
///
/// // Start the rendering loop.
/// while (true)
/// {
///     // Process events
///     // ...
///
///     // Draw a background sprite.
///     window.resetGLStates();
///     window.draw(sprite, texture);
///
///     // Draw raw OpenGL ES 3.1 geometry here.
///     // ...
///
///     // Draw text on top of the OpenGL geometry.
///     window.resetGLStates();
///     window.draw(text);
///
///     // Finally, display the rendered frame on screen.
///     window.display();
/// }
/// \endcode
///
/// \see `sf::Window`, `sf::RenderTarget`, `sf::RenderTexture`, `sf::View`
///
////////////////////////////////////////////////////////////
