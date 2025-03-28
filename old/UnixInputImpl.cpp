#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/Unix/Display.hpp"
#include "SFML/Window/Unix/KeyboardImpl.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowHandle.hpp"

#include "SFML/System/String.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>


namespace sf::priv::InputImpl
{
////////////////////////////////////////////////////////////
bool isKeyPressed(const Keyboard::Key key)
{
    (void)getSDLLayerSingleton();
    return InputImpl::isKeyPressed(InputImpl::delocalize(key));
}


////////////////////////////////////////////////////////////
bool isKeyPressed(const Keyboard::Scancode code)
{
    const bool* keyboardState = SDL_GetKeyboardState(nullptr);
    SFML_BASE_ASSERT(keyboardState != nullptr);

    return keyboardState[mapSFMLScancodeToSDL(code)];
}


////////////////////////////////////////////////////////////
Keyboard::Key localize(const Keyboard::Scancode code)
{
    return priv::localizeViaSDL(code);
}


////////////////////////////////////////////////////////////
Keyboard::Scancode delocalize(const Keyboard::Key key)
{
    return priv::delocalizeViaSDL(key);
}


////////////////////////////////////////////////////////////
String getDescription(const Keyboard::Scancode code)
{
    return SDL_GetKeyName(mapSFMLKeycodeToSDL(InputImpl::localize(code)));
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(bool /*visible*/)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool isMouseButtonPressed(Mouse::Button button)
{
    // Open a connection with the X server
    const auto display = openDisplay();

    // we don't care about these but they are required
    ::Window root  = 0;
    ::Window child = 0;
    int      wx    = 0;
    int      wy    = 0;
    int      gx    = 0;
    int      gy    = 0;

    unsigned int buttons = 0;
    XQueryPointer(display.get(), DefaultRootWindow(display.get()), &root, &child, &gx, &gy, &wx, &wy, &buttons);

    // Buttons 4 and 5 are the vertical wheel and 6 and 7 the horizontal wheel.
    // There is no mask for buttons 8 and 9, so checking the state of buttons
    // Mouse::Button::Extra1 and Mouse::Button::Extra2 is not supported.
    // clang-format off
    switch (button)
    {
        case Mouse::Button::Left:   return buttons & Button1Mask;
        case Mouse::Button::Right:  return buttons & Button3Mask;
        case Mouse::Button::Middle: return buttons & Button2Mask;
        case Mouse::Button::Extra1: return false; // not supported by X
        case Mouse::Button::Extra2: return false; // not supported by X
        default:                    return false;
    }
    // clang-format on
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition()
{
    // Open a connection with the X server
    const auto display = openDisplay();

    // we don't care about these but they are required
    ::Window     root    = 0;
    ::Window     child   = 0;
    int          x       = 0;
    int          y       = 0;
    unsigned int buttons = 0;

    int gx = 0;
    int gy = 0;
    XQueryPointer(display.get(), DefaultRootWindow(display.get()), &root, &child, &gx, &gy, &x, &y, &buttons);

    return {gx, gy};
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition(const WindowBase& relativeTo)
{
    if (const WindowHandle handle = relativeTo.getNativeHandle())
    {
        // Open a connection with the X server
        const auto display = openDisplay();

        // we don't care about these but they are required
        ::Window     root    = 0;
        ::Window     child   = 0;
        int          gx      = 0;
        int          gy      = 0;
        unsigned int buttons = 0;

        int x = 0;
        int y = 0;
        XQueryPointer(display.get(), handle, &root, &child, &gx, &gy, &x, &y, &buttons);

        return {x, y};
    }

    return {};
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position)
{
    // Open a connection with the X server
    const auto display = openDisplay();

    XWarpPointer(display.get(), None, DefaultRootWindow(display.get()), 0, 0, 0, 0, position.x, position.y);
    XFlush(display.get());
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position, const WindowBase& relativeTo)
{
    // Open a connection with the X server
    const auto display = openDisplay();

    if (const WindowHandle handle = relativeTo.getNativeHandle())
    {
        XWarpPointer(display.get(), None, handle, 0, 0, 0, 0, position.x, position.y);
        XFlush(display.get());
    }
}


////////////////////////////////////////////////////////////
bool isTouchDown(unsigned int /* finger */)
{
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int /* finger */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int /* finger */, const WindowBase& /* relativeTo */)
{
    // Not applicable
    return {};
}

} // namespace sf::priv::InputImpl
