#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/GlobalAnchorPointMixin.hpp"
#include "SFML/System/LocalAnchorPointMixin.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct SDL_Window;

namespace sf
{
class Cursor;
} // namespace sf

namespace sf::priv
{
class SDLWindowImpl;
} // namespace sf::priv

namespace sf::Vulkan
{
struct VulkanSurfaceData;
} // namespace sf::Vulkan


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Window that serves as a base for other windows
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API WindowBase : public GlobalAnchorPointMixin<WindowBase>, public LocalAnchorPointMixin<WindowBase>
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
    /// Creates a window with the size, title, style and pacing
    /// options defined in `windowSettings`. If
    /// `windowSettings.fullscreen` is `true`, the requested
    /// size and bit depth must correspond to a valid fullscreen
    /// video mode (see `sf::VideoModeUtils::getFullscreenModes`).
    ///
    /// On failure (invalid settings, OS error, ...) returns
    /// `base::nullOpt`.
    ///
    /// \param windowSettings Window creation parameters
    ///
    /// \return The newly created window on success, `base::nullOpt` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<WindowBase> create(const Settings& windowSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Create a window wrapping an existing native control
    ///
    /// Use this overload to wrap an already existing OS-level
    /// window/control. Unlike `sf::Window::create`, this base
    /// version does not attach an OpenGL context to the window.
    ///
    /// \param handle Platform-specific handle of the control to attach to
    ///
    /// \return The newly wrapped window on success, `base::nullOpt` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<WindowBase> create(WindowHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Closes the window and frees all the resources attached to it.
    ///
    ////////////////////////////////////////////////////////////
    ~WindowBase();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    WindowBase(const WindowBase&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    WindowBase& operator=(const WindowBase&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    WindowBase(WindowBase&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    WindowBase& operator=(WindowBase&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Pop the next event from the front of the FIFO event queue, if any, and return it
    ///
    /// This function is not blocking: if there's no pending event then
    /// it will return a `base::nullOpt`. Note that more than one event
    /// may be present in the event queue, thus you should always call
    /// this function in a loop to make sure that you process every
    /// pending event.
    /// \code
    /// while (const sf::base::Optional event = window.pollEvent())
    /// {
    ///    // process event...
    /// }
    /// \endcode
    ///
    /// \return The potentially pending event, `base::nullOpt` otherwise
    ///
    /// \see `waitEvent`, `pollAndHandleEvents`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> pollEvent();

    ////////////////////////////////////////////////////////////
    /// \brief Wait for an event and return it
    ///
    /// This function is blocking: if there's no pending event then
    /// it will wait until an event is received or until the provided
    /// timeout elapses. Only if an error or a timeout occurs the
    /// returned event will be `base::nullOpt`.
    /// This function is typically used when you have a thread that is
    /// dedicated to events handling: you want to make this thread sleep
    /// as long as no new event is received.
    /// \code
    /// while (const sf::base::Optional event = window.waitEvent())
    /// {
    ///    // process event...
    /// }
    /// \endcode
    ///
    /// \param timeout Maximum time to wait (`Time{}` for infinite)
    ///
    /// \return The event on success, `base::nullOpt` on timeout or if window was closed
    ///
    /// \see `pollEvent`, `pollAndHandleEvents`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> waitEvent(Time timeout = {});

    ////////////////////////////////////////////////////////////
    /// \brief Handle all pending events
    ///
    /// This function is not blocking: if there's no pending event then
    /// it will return without calling any of the handlers.
    ///
    /// This function can take a variadic list of event handlers that
    /// each take a concrete event type as a single parameter. The event
    /// handlers can be any kind of callable object that has an
    /// `operator()` defined for a specific event type. Additionally a
    /// generic callable can also be provided that will be invoked for
    /// every event type. If both types of callables are provided, the
    /// callables taking concrete event types will be preferred over the
    /// generic callable by overload resolution. Generic callables can
    /// be used to customize handler dispatching based on the deduced
    /// type of the event and other information available at compile
    /// time.
    ///
    /// Examples of callables:
    /// - Lambda expressions: `[&](const sf::Event::KeyPressed) { ... }`
    /// - Free functions: `void handler(const sf::Event::KeyPressed&) { ... }`
    ///
    /// \code
    /// // Only provide handlers for concrete event types
    /// window.pollAndHandleEvents(
    ///     [&](sf::Event::Closed) { /* handle event */ },
    ///     [&](const sf::Event::KeyPressed& keyPress) { /* handle event */ }
    /// );
    /// \endcode
    /// \code
    /// // Provide a generic event handler
    /// window.pollAndHandleEvents(
    ///     [&](const auto& event)
    ///     {
    ///         if constexpr (std::is_same_v<std::decay_t<decltype(event)>, sf::Event::Closed>)
    ///         {
    ///             // Handle Closed
    ///             handleClosed();
    ///         }
    ///         else if constexpr (std::is_same_v<std::decay_t<decltype(event)>, sf::Event::KeyPressed>)
    ///         {
    ///             // Handle KeyPressed
    ///             handleKeyPressed(event);
    ///         }
    ///         else
    ///         {
    ///             // Handle non-KeyPressed
    ///             handleOtherEvents(event);
    ///         }
    ///     }
    /// );
    /// \endcode
    /// \code
    /// // Provide handlers for concrete types and fall back to generic handler
    /// window.pollAndHandleEvents(
    ///     [&](sf::Event::Closed) { /* handle event */ },
    ///     [&](const sf::Event::KeyPressed& keyPress) { /* handle event */ },
    ///     [&](const auto& event) { /* handle all other events */ }
    /// );
    /// \endcode
    ///
    /// Calling member functions is supported through lambda
    /// expressions.
    /// \code
    /// // Provide a generic event handler
    /// window.pollAndHandleEvents(
    ///     [this](const auto& event) { handle(event); }
    /// );
    /// \endcode
    ///
    /// \param handlers A variadic list of callables that take a specific event as their only parameter
    ///
    /// \see `waitEvent`, `pollEvent`
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Handlers, auto PollEventFn = &WindowBase::pollEvent>
    void pollAndHandleEvents(Handlers&&... handlers);

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the window
    ///
    /// This function returns the position of the top-left corner
    /// of the window *not* including any decoration such as the titlebar.
    ///
    /// \return Position of the window, in pixels
    ///
    /// \see `setPosition`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2i getPosition() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the position of the window on screen
    ///
    /// This function only works for top-level windows
    /// (i.e. it will be ignored for windows created from
    /// the handle of a child window/control).
    ///
    /// \param position New position, in pixels
    ///
    /// \see `getPosition`
    ///
    ////////////////////////////////////////////////////////////
    void setPosition(Vec2i position);

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the rendering region of the window
    ///
    /// The size doesn't include the titlebar and borders
    /// of the window.
    ///
    /// \return Size in pixels
    ///
    /// \see `setSize`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2u getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the size of the rendering region of the window
    ///
    /// \param size New size, in pixels
    ///
    /// \see `getSize`
    ///
    ////////////////////////////////////////////////////////////
    void setSize(Vec2u size);

    ////////////////////////////////////////////////////////////
    /// \brief Set the minimum window rendering region size
    ///
    /// The OS will prevent the user from resizing the window
    /// below this size. Use the `Optional` overload and pass
    /// `base::nullOpt` to clear any previously set minimum.
    ///
    /// \param minimumSize New minimum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void setMinimumSize(const Vec2u& minimumSize);
    void setMinimumSize(const base::Optional<Vec2u>& minimumSize);

    ////////////////////////////////////////////////////////////
    /// \brief Set the maximum window rendering region size
    ///
    /// The OS will prevent the user from resizing the window
    /// above this size. Use the `Optional` overload and pass
    /// `base::nullOpt` to clear any previously set maximum.
    ///
    /// \param maximumSize New maximum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void setMaximumSize(const Vec2u& maximumSize);
    void setMaximumSize(const base::Optional<Vec2u>& maximumSize);

    ////////////////////////////////////////////////////////////
    /// \brief Change the title of the window
    ///
    /// \param title New title
    ///
    /// \see `setIcon`
    ///
    ////////////////////////////////////////////////////////////
    void setTitle(const UnicodeString& title);

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// `pixels` must be an array of `size` pixels
    /// in 32-bits RGBA format.
    ///
    /// The OS default icon is used by default.
    ///
    /// \param pixels Pointer to the array of pixels in memory. The
    ///               pixels are copied, so you need not keep the
    ///               source alive after calling this function.
    /// \param size   Icon's width and height, in pixels
    ///
    /// \see `setTitle`
    ///
    ////////////////////////////////////////////////////////////
    void setIcon(const base::U8* pixels, Vec2u size);

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window
    ///
    /// The window is shown by default.
    ///
    /// \param visible `true` to show the window, `false` to hide it
    ///
    ////////////////////////////////////////////////////////////
    void setVisible(bool visible);

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the mouse cursor
    ///
    /// The mouse cursor is visible by default.
    ///
    /// \warning On Windows, this function needs to be called from the
    ///          thread that created the window.
    ///
    /// \param visible `true` to show the mouse cursor, `false` to hide it
    ///
    ////////////////////////////////////////////////////////////
    void setMouseCursorVisible(bool visible);

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor
    ///
    /// If set, grabs the mouse cursor inside this window's client
    /// area so it may no longer be moved outside its bounds.
    /// Note that grabbing is only active while the window has
    /// focus.
    ///
    /// \param grabbed `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    void setMouseCursorGrabbed(bool grabbed);

    ////////////////////////////////////////////////////////////
    /// \brief Set the displayed cursor to a native system cursor
    ///
    /// Upon window creation, the arrow cursor is used by default.
    ///
    /// \warning The cursor must not be destroyed while in use by
    ///          the window.
    ///
    /// \warning Features related to Cursor are not supported on
    ///          iOS and Android.
    ///
    /// \param cursor Native system cursor type to display
    ///
    /// \see `sf::Cursor::loadFromSystem`, `sf::Cursor::loadFromPixels`
    ///
    ////////////////////////////////////////////////////////////
    void setMouseCursor(const Cursor& cursor);

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable automatic key-repeat
    ///
    /// If key repeat is enabled, you will receive repeated
    /// KeyPressed events while keeping a key pressed. If it is disabled,
    /// you will only get a single event when the key is pressed.
    ///
    /// Key repeat is enabled by default.
    ///
    /// \param enabled `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    void setKeyRepeatEnabled(bool enabled);

    ////////////////////////////////////////////////////////////
    /// \brief Change the joystick threshold
    ///
    /// The joystick threshold is the value below which
    /// no JoystickMoved event will be generated.
    ///
    /// The threshold value is 0.1 by default.
    ///
    /// \param threshold New threshold, in the range [0, 100]
    ///
    ////////////////////////////////////////////////////////////
    void setJoystickThreshold(float threshold);

    ////////////////////////////////////////////////////////////
    /// \brief Request the current window to be made the active
    ///        foreground window
    ///
    /// At any given time, only one window may have the input focus
    /// to receive input events such as keystrokes or mouse events.
    /// If a window requests focus, it only hints to the operating
    /// system, that it would like to be focused. The operating system
    /// is free to deny the request.
    /// This is not to be confused with `setActive()`.
    ///
    /// \see `hasFocus`
    ///
    ////////////////////////////////////////////////////////////
    void requestFocus();

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has the input focus
    ///
    /// At any given time, only one window may have the input focus
    /// to receive input events such as keystrokes or most mouse
    /// events.
    ///
    /// \return `true` if window has focus, `false` otherwise
    /// \see `requestFocus`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasFocus() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return a scaling factor for DPI-aware scaling
    ///
    /// \return `1.f` for default DPI (96), otherwise the scaling factor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getDisplayScale() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the OS-specific handle of the window
    ///
    /// The type of the returned handle is `sf::WindowHandle`,
    /// which is a type alias to the handle type defined by the OS.
    /// You shouldn't need to use this function, unless you have
    /// very specific stuff to implement that SFML doesn't support,
    /// or implement a temporary workaround until a bug is fixed.
    ///
    /// \return System handle of the window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] WindowHandle getNativeHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the underlying `SDL_Window*` backing this window
    ///
    /// VRSFML's windowing layer is implemented on top of SDL.
    /// This accessor exposes the raw `SDL_Window*` for code
    /// that needs to interoperate with SDL directly.
    ///
    /// \return Pointer to the underlying SDL window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDL_Window* getSDLHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Create a Vulkan rendering surface for this window
    ///
    /// Creates a `VkSurfaceKHR` corresponding to this window
    /// using the instance, output handle, and optional
    /// allocator bundled inside `vulkanSurfaceData`.
    ///
    /// \param vulkanSurfaceData Inputs and output for the surface creation
    ///
    /// \return `true` if surface creation was successful, `false` otherwise
    ///
    /// \see `sf::Vulkan::isAvailable`, `sf::Vulkan::VulkanSurfaceData`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createVulkanSurface(const Vulkan::VulkanSurfaceData& vulkanSurfaceData);

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window is in exclusive fullscreen mode
    ///
    /// \return `true` if the window is fullscreen, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isFullscreen() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window can be resized by the user
    ///
    /// \return `true` if the user is allowed to resize the window, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isResizable() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has a titlebar / borders
    ///
    /// \return `true` if a titlebar is shown, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasTitlebar() const;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable user-driven resizing
    ///
    /// \param resizable `true` to allow resizing, `false` to prevent it
    ///
    ////////////////////////////////////////////////////////////
    void setResizable(bool resizable);

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window's title bar
    ///
    /// \param hasTitleBar `true` to show the title bar / borders, `false` to hide them
    ///
    ////////////////////////////////////////////////////////////
    void setHasTitlebar(bool hasTitleBar);

    ////////////////////////////////////////////////////////////
    /// \brief Get the local bounding rectangle of the window
    ///
    /// The local bounds always have their top-left corner at
    /// the origin and their size set to the window's current
    /// size, in pixels.
    ///
    /// \return Rect with origin `{0, 0}` and size equal to the window size
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getLocalBounds() const
    {
        return Rect2f{{0.f, 0.f}, getSize().toVec2f()};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the global bounding rectangle of the window
    ///
    /// The global bounds combine `getPosition()` and
    /// `getSize()` into a `Rect2f`.
    ///
    /// \return Rect with origin equal to the window position and size equal to the window size
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Rect2f getGlobalBounds() const
    {
        return Rect2f{getPosition().toVec2f(), getSize().toVec2f()};
    }

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct a window base from the inner implementation
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowBase(base::PassKey<WindowBase>&&, base::UniquePtr<priv::SDLWindowImpl>&& impl);

private:
    friend class Window;

    ////////////////////////////////////////////////////////////
    /// \brief Get the SDL implementation of the window
    ///
    ////////////////////////////////////////////////////////////
    priv::SDLWindowImpl& getWindowImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Processes an event before it is sent to the user
    ///
    /// This function is called every time an event is received
    /// from the internal window (through `pollEvent` or `waitEvent`).
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
    base::UniquePtr<priv::SDLWindowImpl> m_impl; //!< Platform-specific implementation of the window
    Vec2u                                m_size; //!< Current size of the window
};

} // namespace sf

#include "SFML/Window/WindowBase.inl"

////////////////////////////////////////////////////////////
/// \class sf::WindowBase
/// \ingroup window
///
/// `sf::WindowBase` is the base class for all VRSFML windows.
/// It owns the OS-level window resource and provides the
/// common interface for moving, resizing, showing/hiding,
/// controlling the cursor, and pumping events. It does *not*
/// own an OpenGL context: use `sf::Window` (which derives
/// from `WindowBase`) for that.
///
/// A `WindowBase` can create its own new top-level window via
/// `create(settings)` or be wrapped around an already
/// existing OS control via `create(handle)`.
///
/// Event handling is provided by `pollEvent()`, `waitEvent()`,
/// and `pollAndHandleEvents()`.
///
/// Usage example:
/// \code
/// // Initialize the window context (must outlive any window)
/// auto windowContext = sf::WindowContext::create().value();
///
/// // Create a new window
/// auto window = sf::WindowBase::create({.size = {800u, 600u}, .title = "SFML Window"}).value();
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
///     // Do things with the window here...
/// }
/// \endcode
///
////////////////////////////////////////////////////////////
