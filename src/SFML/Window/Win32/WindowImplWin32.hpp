#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowImpl.hpp"

#include "SFML/System/Vector2.hpp"
#include "SFML/System/Win32/WindowsHeader.hpp"

#include "SFML/Base/IntTypes.hpp"


namespace sf
{
class String;
struct WindowSettings;

namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Windows implementation of WindowImpl
///
////////////////////////////////////////////////////////////
class [[nodiscard]] WindowImplWin32 : public WindowImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the window implementation from an existing control
    ///
    /// \param handle Platform-specific handle of the control
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowImplWin32(WindowHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Create the window implementation
    ///
    /// \param windowSettings Window settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowImplWin32(const WindowSettings& windowSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~WindowImplWin32() override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the OS-specific handle of the window
    ///
    /// \return Handle of the window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] WindowHandle getNativeHandle() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the window
    ///
    /// \return Position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2i getPosition() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the position of the window on screen
    ///
    /// \param position New position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void setPosition(Vector2i position) override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the client size of the window
    ///
    /// \return Size of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2u getSize() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the size of the rendering region of the window
    ///
    /// \param size New size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void setSize(Vector2u size) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the title of the window
    ///
    /// \param title New title
    ///
    ////////////////////////////////////////////////////////////
    void setTitle(const String& title) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// \param size   Icon's width and height, in pixels
    /// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
    ///
    ////////////////////////////////////////////////////////////
    void setIcon(Vector2u size, const base::U8* pixels) override;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window
    ///
    /// \param visible `true` to show, `false` to hide
    ///
    ////////////////////////////////////////////////////////////
    void setVisible(bool visible) override;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the mouse cursor
    ///
    /// \param visible `true` to show, `false` to hide
    ///
    ////////////////////////////////////////////////////////////
    void setMouseCursorVisible(bool visible) override;

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor
    ///
    /// \param grabbed `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    void setMouseCursorGrabbed(bool grabbed) override;

    ////////////////////////////////////////////////////////////
    /// \brief Set the displayed cursor to a native system cursor
    ///
    /// \param cursor Native system cursor type to display
    ///
    ////////////////////////////////////////////////////////////
    void setMouseCursor(const CursorImpl& cursor) override;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable automatic key-repeat
    ///
    /// \param enabled `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    void setKeyRepeatEnabled(bool enabled) override;

    ////////////////////////////////////////////////////////////
    /// \brief Request the current window to be made the active
    ///        foreground window
    ///
    ////////////////////////////////////////////////////////////
    void requestFocus() override;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has the input focus
    ///
    /// \return `true` if window has focus, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasFocus() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Return a scaling factor for DPI-aware scaling
    ///
    /// \return `1.f` for default DPI (96) or the window is not
    ///         DPI-aware, otherwise the scaling factor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getDPIAwareScalingFactor() const override;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Process incoming events from the operating system
    ///
    ////////////////////////////////////////////////////////////
    void processEvents() override;

private:
    ////////////////////////////////////////////////////////////
    /// Register the window class
    ///
    ////////////////////////////////////////////////////////////
    void registerWindowClass();

    ////////////////////////////////////////////////////////////
    /// \brief Switch to fullscreen mode
    ///
    /// \param mode Video mode to switch to
    ///
    ////////////////////////////////////////////////////////////
    void switchToFullscreen(const VideoMode& mode);

    ////////////////////////////////////////////////////////////
    /// \brief Free all the graphical resources attached to the window
    ///
    ////////////////////////////////////////////////////////////
    void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Process a Win32 event
    ///
    /// \param message Message to process
    /// \param wParam  First parameter of the event
    /// \param lParam  Second parameter of the event
    ///
    ////////////////////////////////////////////////////////////
    void processEvent(UINT message, WPARAM wParam, LPARAM lParam);

    ////////////////////////////////////////////////////////////
    /// \brief Enables or disables tracking for the mouse cursor leaving the window
    ///
    /// \param track `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    void setTracking(bool track);

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor
    ///
    /// This is not to be confused with setMouseCursorGrabbed.
    /// Here m_cursorGrabbed is not modified; it is used,
    /// for example, to release the cursor when switching to
    /// another application.
    ///
    /// \param grabbed `true` to enable, `false` to disable
    ///
    ////////////////////////////////////////////////////////////
    void grabCursor(bool grabbed);

    ////////////////////////////////////////////////////////////
    /// \brief Convert content size to window size including window chrome
    ///
    /// \param size Size to convert
    ///
    /// \return Converted size including window chrome
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2i contentSizeToWindowSize(Vector2u size);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a Win32 virtual key code to a SFML key code
    ///
    /// \param key   Virtual key code to convert
    /// \param flags Additional flags
    ///
    /// \return SFML key code corresponding to the key
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Keyboard::Key virtualKeyCodeToSF(WPARAM key, LPARAM flags);

    ////////////////////////////////////////////////////////////
    /// \brief Function called whenever one of our windows receives a message
    ///
    /// \param handle  Win32 handle of the window
    /// \param message Message received
    /// \param wParam  First parameter of the message
    /// \param lParam  Second parameter of the message
    ///
    /// \return `true` to discard the event after it has been processed
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static LRESULT CALLBACK globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a Win32 scancode to an sfml scancode
    ///
    /// \param flags input flags
    ///
    /// \return SFML scancode corresponding to the key
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Keyboard::Scancode toScancode(WPARAM wParam, LPARAM lParam);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    HWND     m_handle{};            //!< Win32 handle of the window
    LONG_PTR m_callback{};          //!< Stores the original event callback function of the control
    bool     m_cursorVisible{true}; //!< Is the cursor visible or hidden?
    HCURSOR  m_lastCursor{
        LoadCursor(nullptr, IDC_ARROW)}; //!< Last cursor used -- this data is not owned by the window and is required to be always valid
    HICON    m_icon{};                    //!< Custom icon assigned to the window
    bool     m_keyRepeatEnabled{true}; //!< Automatic key-repeat state for keydown events
    Vector2u m_lastSize;               //!< The last handled size of the window
    bool     m_resizing{};             //!< Is the window being resized?
    char16_t m_surrogate{}; //!< First half of the surrogate pair, in case we're receiving a Unicode character in two events
    bool m_mouseInside{};   //!< Mouse is inside the window?
    bool m_fullscreen{};    //!< Is the window fullscreen?
    bool m_cursorGrabbed{}; //!< Is the mouse cursor trapped?
};

} // namespace priv

} // namespace sf
