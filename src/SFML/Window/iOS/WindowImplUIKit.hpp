#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Event.hpp"
#include "SFML/Window/WindowImpl.hpp"
#include "SFML/Window/iOS/ObjCType.hpp"


SFML_DECLARE_OBJC_CLASS(UIWindow);
SFML_DECLARE_OBJC_CLASS(SFView);
SFML_DECLARE_OBJC_CLASS(SFViewController);


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief iOS (UIKit) implementation of WindowImpl
///
////////////////////////////////////////////////////////////
class [[nodiscard]] WindowImplUIKit : public WindowImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the window implementation from an existing control
    ///
    /// \param handle Platform-specific handle of the control
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowImplUIKit(WindowHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Create the window implementation
    ///
    /// \param windowSettings Window settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit WindowImplUIKit(const WindowSettings& windowSettings);

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
    /// \brief Set the minimum window rendering region size
    ///
    /// Pass `base::nullOpt` to unset the minimum size
    ///
    /// \param minimumSize New minimum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void setMinimumSize(const base::Optional<Vector2u>& minimumSize) override;

    ////////////////////////////////////////////////////////////
    /// \brief Set the maximum window rendering region size
    ///
    /// Pass `base::nullOpt` to unset the maximum size
    ///
    /// \param maximumSize New maximum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    void setMaximumSize(const base::Optional<Vector2u>& maximumSize) override;

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
    /// \brief Clips or releases the mouse cursor
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

    ////////////////////////////////////////////////////////////
    /// \brief Notify an event
    ///
    /// \param event Event to forward
    ///
    ////////////////////////////////////////////////////////////
    void forwardEvent(Event event);

    ////////////////////////////////////////////////////////////
    /// \brief Get the window's view
    ///
    /// \return Pointer to the window's view
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFView* getGlView() const;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the virtual keyboard
    ///
    /// \param visible `true` to show, `false` to hide
    ///
    ////////////////////////////////////////////////////////////
    void setVirtualKeyboardVisible(bool visible);

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Process incoming events from the operating system
    ///
    ////////////////////////////////////////////////////////////
    void processEvents() override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    UIWindow*         m_window{};         ///< Pointer to the internal UIKit window
    SFView*           m_view{};           ///< OpenGL view of the window
    SFViewController* m_viewController{}; ///< Controller attached to the view
    bool              m_hasFocus{};       ///< Current focus state of the window
    float             m_backingScale{};   ///< Converts from points to pixels and vice versa
};

} // namespace sf::priv
