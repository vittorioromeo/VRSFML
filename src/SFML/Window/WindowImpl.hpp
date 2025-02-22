#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Event.hpp"
#include "SFML/Window/WindowHandle.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/UniquePtr.hpp"


namespace sf
{
class String;
class Time;
struct WindowSettings;

namespace Vulkan
{
struct VulkanSurfaceData;
} // namespace Vulkan

namespace priv
{
class CursorImpl;
class JoystickManager;

////////////////////////////////////////////////////////////
/// \brief Abstract base class for OS-specific window implementation
///
////////////////////////////////////////////////////////////
class [[nodiscard]] WindowImpl // TODO P1: Remove and rely on `.cpp` compilation? how to deal with state?
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new window depending on the current OS
    ///
    /// \param mode  Video mode to use
    /// \param title Title of the window
    /// \param style Window style
    /// \param state Window state
    /// \param settings Additional settings for the underlying OpenGL context
    ///
    /// \return Pointer to the created window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<WindowImpl> create(WindowSettings windowSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new window depending on to the current OS
    ///
    /// \param handle Platform-specific handle of the control
    ///
    /// \return Pointer to the created window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::UniquePtr<WindowImpl> create(WindowHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~WindowImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    WindowImpl(const WindowImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    WindowImpl& operator=(const WindowImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Change the joystick threshold, i.e. the value below which
    ///        no move event will be generated
    ///
    /// \param threshold New threshold, in range [0, 100]
    ///
    ////////////////////////////////////////////////////////////
    void setJoystickThreshold(float threshold);

    ////////////////////////////////////////////////////////////
    /// \brief Wait for and return the next available window event
    ///
    /// If there's no event available, this function calls the
    /// window's internal event processing function.
    ///
    /// \param timeout Maximum time to wait (`Time::Zero` for infinite)
    ///
    /// \return The event on success, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> waitEvent(Time timeout);

    ////////////////////////////////////////////////////////////
    /// \brief Return the next window event, if available
    ///
    /// If there's no event available, this function calls the
    /// window's internal event processing function.
    ///
    /// \return The event if available, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> pollEvent();

    ////////////////////////////////////////////////////////////
    /// \brief Get the OS-specific handle of the window
    ///
    /// \return Handle of the window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual WindowHandle getNativeHandle() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the window
    ///
    /// \return Position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual Vector2i getPosition() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the minimum window rendering region size
    ///
    /// \return Minimum size
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Vector2u> getMinimumSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the maximum window rendering region size
    ///
    /// \return Maximum size
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Vector2u> getMaximumSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the position of the window on screen
    ///
    /// \param position New position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setPosition(Vector2i position) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the client size of the window
    ///
    /// \return Size of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual Vector2u getSize() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the size of the rendering region of the window
    ///
    /// \param size New size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setSize(Vector2u size) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Set the minimum window rendering region size
    ///
    /// Pass `base::nullOpt` to unset the minimum size
    ///
    /// \param minimumSize New minimum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMinimumSize(const base::Optional<Vector2u>& minimumSize);

    ////////////////////////////////////////////////////////////
    /// \brief Set the maximum window rendering region size
    ///
    /// Pass `base::nullOpt` to unset the maximum size
    ///
    /// \param maximumSize New maximum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMaximumSize(const base::Optional<Vector2u>& maximumSize);

    ////////////////////////////////////////////////////////////
    /// \brief Change the title of the window
    ///
    /// \param title New title
    ///
    ////////////////////////////////////////////////////////////
    virtual void setTitle(const String& title) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// \param size   Icon's width and height, in pixels
    /// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
    ///
    ////////////////////////////////////////////////////////////
    virtual void setIcon(Vector2u size, const base::U8* pixels) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window
    ///
    /// \param visible `true` to show, `false` to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void setVisible(bool visible) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the mouse cursor
    ///
    /// \param visible `true` to show, `false` to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursorVisible(bool visible) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor and keeps it from leaving
    ///
    /// \param grabbed `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursorGrabbed(bool grabbed) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Set the displayed cursor to a native system cursor
    ///
    /// \param cursor Native system cursor type to display
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursor(const CursorImpl& cursor) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable automatic key-repeat
    ///
    /// \param enabled `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void setKeyRepeatEnabled(bool enabled) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Request the current window to be made the active
    ///        foreground window
    ///
    ////////////////////////////////////////////////////////////
    virtual void requestFocus() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has the input focus
    ///
    /// \return `true` if window has focus, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool hasFocus() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Return a scaling factor for DPI-aware scaling
    ///
    /// \return `1.f` for default DPI (96) or the window is not
    ///         DPI-aware, otherwise the scaling factor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual float getDPIAwareScalingFactor() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Create a Vulkan rendering surface
    ///
    /// \param instance  Vulkan instance
    /// \param surface   Created surface
    /// \param allocator Allocator to use
    ///
    /// \return `true` if surface creation was successful, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createVulkanSurface(const Vulkan::VulkanSurfaceData& vulkanSurfaceData) const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Push a new event into the event queue
    ///
    /// This function is to be used by derived classes, to
    /// notify the SFML window that a new event was triggered
    /// by the system.
    ///
    /// \param event Event to push
    ///
    ////////////////////////////////////////////////////////////
    void pushEvent(const Event& event);

    ////////////////////////////////////////////////////////////
    /// \brief Process incoming events from the operating system
    ///
    ////////////////////////////////////////////////////////////
    virtual void processEvents() = 0;

private:
    ////////////////////////////////////////////////////////////
    /// \return First event of the queue if available, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> popEvent();

    ////////////////////////////////////////////////////////////
    /// \brief Read the joysticks state and generate the appropriate events
    ///
    ////////////////////////////////////////////////////////////
    void processJoystickEvents();

    ////////////////////////////////////////////////////////////
    /// \brief Read the sensors state and generate the appropriate events
    ///
    ////////////////////////////////////////////////////////////
    void processSensorEvents();

    ////////////////////////////////////////////////////////////
    /// \brief Read joystick, sensors, and OS state and populate event queue
    ///
    ////////////////////////////////////////////////////////////
    void populateEventQueue();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 1024> m_impl; //!< Implementation details
};

} // namespace priv

} // namespace sf
