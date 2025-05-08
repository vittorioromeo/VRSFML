#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowSettings.hpp"
#include "SFML/Window/iOS/SFAppDelegate.hpp"
#include "SFML/Window/iOS/SFView.hpp"
#include "SFML/Window/iOS/SFViewController.hpp"
#include "SFML/Window/iOS/WindowImplUIKit.hpp"

#include "SFML/System/Err.hpp"

#include <UIKit/UIKit.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


namespace sf::priv
{
////////////////////////////////////////////////////////////
WindowImplUIKit::WindowImplUIKit(WindowHandle /* handle */)
{
    // Not implemented
}


////////////////////////////////////////////////////////////
WindowImplUIKit::WindowImplUIKit(const WindowSettings& windowSettings)
{
    m_backingScale = static_cast<float>([SFAppDelegate getInstance].backingScaleFactor);

    // Apply the fullscreen flag
    [UIApplication sharedApplication].statusBarHidden = !windowSettings.hasTitlebar || (windowSettings.fullscreen);

    // Set the orientation according to the requested size
    if (windowSettings.size.x > windowSettings.size.y)
        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft];
    else
        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationPortrait];

    // Create the window
    const CGRect frame = [UIScreen mainScreen].bounds; // Ignore user size, it wouldn't make sense to use something else
    m_window           = [[UIWindow alloc] initWithFrame:frame];
    m_hasFocus         = true;

    // Assign it to the application delegate
    [SFAppDelegate getInstance].sfWindow = this;

    const CGRect viewRect = frame;

    // Create the view
    m_view = [[SFView alloc] initWithFrame:viewRect andContentScaleFactor:(static_cast<double>(m_backingScale))];
    [m_view resignFirstResponder];

    // Create the view controller
    m_viewController                      = [SFViewController alloc];
    m_viewController.view                 = m_view;
    m_viewController.orientationCanChange = windowSettings.resizable;
    m_window.rootViewController           = m_viewController;

    // Make it the current window
    [m_window makeKeyAndVisible];
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::processEvents()
{
    while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.0001, true) == kCFRunLoopRunHandledSource)
        ;
}


////////////////////////////////////////////////////////////
WindowHandle WindowImplUIKit::getNativeHandle() const
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

    return (__bridge WindowHandle)m_window;

#pragma GCC diagnostic pop
}


////////////////////////////////////////////////////////////
Vec2i WindowImplUIKit::getPosition() const
{
    const CGPoint origin = m_window.frame.origin;
    return {static_cast<int>(origin.x * static_cast<double>(m_backingScale)),
            static_cast<int>(origin.y * static_cast<double>(m_backingScale))};
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setPosition(Vec2i /* position */)
{
}


////////////////////////////////////////////////////////////
Vec2u WindowImplUIKit::getSize() const
{
    const CGRect physicalFrame = m_window.frame;
    return {static_cast<unsigned int>(physicalFrame.size.width * static_cast<double>(m_backingScale)),
            static_cast<unsigned int>(physicalFrame.size.height * static_cast<double>(m_backingScale))};
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setSize(Vec2u size)
{
    // TODO P2: ...

    // if these sizes are required one day, don't forget to scale them!
    // size.x /= m_backingScale;
    // size.y /= m_backingScale;

    // Set the orientation according to the requested size
    if (size.x > size.y)
        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft];
    else
        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationPortrait];
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setMinimumSize(const base::Optional<Vec2u>& /* minimumSize */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setMaximumSize(const base::Optional<Vec2u>& /* maximumSize */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setTitle(const String& /* title */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setIcon(Vec2u /* size */, const base::U8* /* pixels */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setVisible(bool /* visible */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setMouseCursorVisible(bool /* visible */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setMouseCursorGrabbed(bool /* grabbed */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setMouseCursor(const CursorImpl& /* cursor */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setKeyRepeatEnabled(bool /* enabled */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::requestFocus()
{
    // To implement
}


////////////////////////////////////////////////////////////
bool WindowImplUIKit::hasFocus() const
{
    return m_hasFocus;
}


////////////////////////////////////////////////////////////
float WindowImplUIKit::getDisplayContentScale() const
{
    // TODO P1: implement
    return 1.f;
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::forwardEvent(Event event)
{
    if (event.is<Event::FocusGained>())
        m_hasFocus = true;
    else if (event.is<Event::FocusLost>())
        m_hasFocus = false;

    pushEvent(event);
}


////////////////////////////////////////////////////////////
SFView* WindowImplUIKit::getGlView() const
{
    return m_view;
}


////////////////////////////////////////////////////////////
void WindowImplUIKit::setVirtualKeyboardVisible(bool visible)
{
    if (visible)
        [m_view becomeFirstResponder];
    else
        [m_view resignFirstResponder];
}

} // namespace sf::priv
