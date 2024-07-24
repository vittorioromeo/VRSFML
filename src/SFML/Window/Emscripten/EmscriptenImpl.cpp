////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2015 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Emscripten/JoystickImpl.hpp>
#include <SFML/Window/Emscripten/WindowImplEmscripten.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/InputImpl.hpp>
#include <SFML/Window/JoystickAxis.hpp>
#include <SFML/Window/JoystickIdentification.hpp>
#include <SFML/Window/JoystickImpl.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoModeImpl.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/StringUtfUtils.hpp>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <map>
#include <vector>

#include <cmath>


namespace
{
sf::priv::WindowImplEmscripten*      window         = nullptr;
bool                                 windowHasFocus = false;
bool                                 joysticksConnected[sf::Joystick::Count];
bool                                 keyStatus[sf::Keyboard::KeyCount];
bool                                 keyStatusInitialized = false;
bool                                 mouseStatus[sf::Mouse::ButtonCount];
bool                                 mouseStatusInitialized = false;
sf::Vector2i                         mousePosition;
bool                                 mousePositionInitialized = false;
std::map<unsigned int, sf::Vector2i> touchStatus;
bool                                 fullscreenPending = false;

sf::Keyboard::Key keyCodeToSF(unsigned long key, unsigned long location)
{
    switch (key)
    {
        case '\b':
            return sf::Keyboard::Key::Backspace;
        case '\t':
            return sf::Keyboard::Key::Tab;

        case '\r':
        {
            if (location == DOM_KEY_LOCATION_STANDARD)
                return sf::Keyboard::Key::Enter;
            else if (location == DOM_KEY_LOCATION_NUMPAD)
                return sf::Keyboard::Key::Enter;
            break;
        }

        case 16:
        {
            if (location == DOM_KEY_LOCATION_LEFT)
                return sf::Keyboard::Key::LShift;
            else if (location == DOM_KEY_LOCATION_RIGHT)
                return sf::Keyboard::Key::RShift;
            break;
        }

        case 17:
        {
            if (location == DOM_KEY_LOCATION_LEFT)
                return sf::Keyboard::Key::LControl;
            else if (location == DOM_KEY_LOCATION_RIGHT)
                return sf::Keyboard::Key::RControl;
            break;
        }

        case 18:
        {
            if (location == DOM_KEY_LOCATION_LEFT)
                return sf::Keyboard::Key::LAlt;
            else if (location == DOM_KEY_LOCATION_RIGHT)
                return sf::Keyboard::Key::RAlt;
            break;
        }

        case 19:
            return sf::Keyboard::Key::Pause;

            // case 20: Caps Lock

        case 27:
            return sf::Keyboard::Key::Escape;

        case ' ':
            return sf::Keyboard::Key::Space;
        case 33:
            return sf::Keyboard::Key::PageUp;
        case 34:
            return sf::Keyboard::Key::PageDown;
        case 35:
            return sf::Keyboard::Key::End;
        case 36:
            return sf::Keyboard::Key::Home;
        case 37:
            return sf::Keyboard::Key::Left;
        case 39:
            return sf::Keyboard::Key::Right;
        case 38:
            return sf::Keyboard::Key::Up;
        case 40:
            return sf::Keyboard::Key::Down;

            // case 42: Print Screen

        case 45:
            return sf::Keyboard::Key::Insert;
        case 46:
            return sf::Keyboard::Key::Delete;

        case ';':
            return sf::Keyboard::Key::Semicolon;

        case '=':
            return sf::Keyboard::Key::Equal;

        case 'A':
            return sf::Keyboard::Key::A;
        case 'Z':
            return sf::Keyboard::Key::Z;
        case 'E':
            return sf::Keyboard::Key::E;
        case 'R':
            return sf::Keyboard::Key::R;
        case 'T':
            return sf::Keyboard::Key::T;
        case 'Y':
            return sf::Keyboard::Key::Y;
        case 'U':
            return sf::Keyboard::Key::U;
        case 'I':
            return sf::Keyboard::Key::I;
        case 'O':
            return sf::Keyboard::Key::O;
        case 'P':
            return sf::Keyboard::Key::P;
        case 'Q':
            return sf::Keyboard::Key::Q;
        case 'S':
            return sf::Keyboard::Key::S;
        case 'D':
            return sf::Keyboard::Key::D;
        case 'F':
            return sf::Keyboard::Key::F;
        case 'G':
            return sf::Keyboard::Key::G;
        case 'H':
            return sf::Keyboard::Key::H;
        case 'J':
            return sf::Keyboard::Key::J;
        case 'K':
            return sf::Keyboard::Key::K;
        case 'L':
            return sf::Keyboard::Key::L;
        case 'M':
            return sf::Keyboard::Key::M;
        case 'W':
            return sf::Keyboard::Key::W;
        case 'X':
            return sf::Keyboard::Key::X;
        case 'C':
            return sf::Keyboard::Key::C;
        case 'V':
            return sf::Keyboard::Key::V;
        case 'B':
            return sf::Keyboard::Key::B;
        case 'N':
            return sf::Keyboard::Key::N;
        case '0':
            return sf::Keyboard::Key::Num0;
        case '1':
            return sf::Keyboard::Key::Num1;
        case '2':
            return sf::Keyboard::Key::Num2;
        case '3':
            return sf::Keyboard::Key::Num3;
        case '4':
            return sf::Keyboard::Key::Num4;
        case '5':
            return sf::Keyboard::Key::Num5;
        case '6':
            return sf::Keyboard::Key::Num6;
        case '7':
            return sf::Keyboard::Key::Num7;
        case '8':
            return sf::Keyboard::Key::Num8;
        case '9':
            return sf::Keyboard::Key::Num9;

        case 91:
        {
            if (location == DOM_KEY_LOCATION_LEFT)
                return sf::Keyboard::Key::LSystem;
            else if (location == DOM_KEY_LOCATION_RIGHT)
                return sf::Keyboard::Key::RSystem;
            break;
        }

        case 93:
            return sf::Keyboard::Key::Menu;

        case 96:
            return sf::Keyboard::Key::Numpad0;
        case 97:
            return sf::Keyboard::Key::Numpad1;
        case 98:
            return sf::Keyboard::Key::Numpad2;
        case 99:
            return sf::Keyboard::Key::Numpad3;
        case 100:
            return sf::Keyboard::Key::Numpad4;
        case 101:
            return sf::Keyboard::Key::Numpad5;
        case 102:
            return sf::Keyboard::Key::Numpad6;
        case 103:
            return sf::Keyboard::Key::Numpad7;
        case 104:
            return sf::Keyboard::Key::Numpad8;
        case 105:
            return sf::Keyboard::Key::Numpad9;

        case 106:
            return sf::Keyboard::Key::Multiply;
        case 107:
            return sf::Keyboard::Key::Add;
        case 109:
            return sf::Keyboard::Key::Subtract;
        case 111:
            return sf::Keyboard::Key::Divide;

        case 112:
            return sf::Keyboard::Key::F1;
        case 113:
            return sf::Keyboard::Key::F2;
        case 114:
            return sf::Keyboard::Key::F3;
        case 115:
            return sf::Keyboard::Key::F4;
        case 116:
            return sf::Keyboard::Key::F5;
        case 117:
            return sf::Keyboard::Key::F6;
        case 118:
            return sf::Keyboard::Key::F7;
        case 119:
            return sf::Keyboard::Key::F8;
        case 120:
            return sf::Keyboard::Key::F9;
        case 121:
            return sf::Keyboard::Key::F10;
        case 122:
            return sf::Keyboard::Key::F11;
        case 123:
            return sf::Keyboard::Key::F12;
        case 124:
            return sf::Keyboard::Key::F13;
        case 125:
            return sf::Keyboard::Key::F14;
        case 126:
            return sf::Keyboard::Key::F15;

            // case 144: Num Lock
            // case 145: Scroll Lock

        case 173:
            return sf::Keyboard::Key::Hyphen;

        case 188:
            return sf::Keyboard::Key::Comma;

        case 190:
            return sf::Keyboard::Key::Period;
        case 191:
            return sf::Keyboard::Key::Slash;
        case 192:
            return sf::Keyboard::Key::Grave;

        case 219:
            return sf::Keyboard::Key::LBracket;
        case 220:
            return sf::Keyboard::Key::Backslash;
        case 221:
            return sf::Keyboard::Key::RBracket;
        case 222:
            return sf::Keyboard::Key::Apostrophe;
    }

    return sf::Keyboard::Key::Unknown;
}

void updatePluggedList()
{
    int numJoysticks = emscripten_get_num_gamepads();

    if (numJoysticks == EMSCRIPTEN_RESULT_NOT_SUPPORTED)
    {
        for (unsigned int i = 0u; i < sf::Joystick::Count; ++i)
        {
            joysticksConnected[i] = false;
        }

        return;
    }

    for (unsigned int i = 0u; (i < sf::Joystick::Count) && (i < static_cast<unsigned int>(numJoysticks)); ++i)
    {
        EmscriptenGamepadEvent gamepadEvent;
        if (emscripten_get_gamepad_status(static_cast<int>(i), &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
        {
            sf::priv::err() << "Failed to get status of gamepad " << i;
            joysticksConnected[i] = false;
            continue;
        }

        if (gamepadEvent.connected)
            joysticksConnected[i] = true;
        else
            joysticksConnected[i] = false;
    }
}

EM_BOOL canvasSizeChangedCallback(int eventType, const void* reserved, void* userData)
{
    if (!window)
        return 0;

    int width, height, fullscreen;
    emscripten_get_canvas_size(&width, &height, &fullscreen);

    sf::Event::Resized event{.size{static_cast<unsigned int>(width), static_cast<unsigned int>(height)}};
    window->pushHtmlEvent(event);

    return 0;
}

void requestFullscreen()
{
    EmscriptenFullscreenStrategy fullscreenStrategy;

    fullscreenStrategy.scaleMode                     = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
    fullscreenStrategy.canvasResolutionScaleMode     = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
    fullscreenStrategy.filteringMode                 = EMSCRIPTEN_FULLSCREEN_FILTERING_BILINEAR;
    fullscreenStrategy.canvasResizedCallback         = canvasSizeChangedCallback;
    fullscreenStrategy.canvasResizedCallbackUserData = 0;

    emscripten_request_fullscreen_strategy(0, 0, &fullscreenStrategy);
}

EM_BOOL keyCallback(int eventType, const EmscriptenKeyboardEvent* e, void* userData)
{
    if (!window)
        return 0;

    sf::Keyboard::Key key = keyCodeToSF(e->which, e->location);

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_KEYDOWN:
        {
            if (e->repeat && !window->getKeyRepeatEnabled())
                return 1;

            if (fullscreenPending)
            {
                requestFullscreen();
                fullscreenPending = false;
            }

            keyStatus[static_cast<std::size_t>(key)] = true;

            sf::Event::KeyPressed event;
            event.alt     = e->altKey != 0;
            event.control = e->ctrlKey != 0;
            event.shift   = e->shiftKey != 0;
            event.system  = e->metaKey != 0;
            event.code    = key;
            window->pushHtmlEvent(event);

            // We try to prevent some keystrokes from bubbling
            // If we try to prevent bubbling for all keys,
            // it prevents keypress events from being generated
            if ((key == sf::Keyboard::Key::Tab) || (key == sf::Keyboard::Key::Backspace) ||
                (key == sf::Keyboard::Key::Menu) || (key == sf::Keyboard::Key::LSystem) ||
                (key == sf::Keyboard::Key::RSystem))
                return 1;

            return 0;
        }
        case EMSCRIPTEN_EVENT_KEYUP:
        {
            keyStatus[static_cast<std::size_t>(key)] = false;

            sf::Event::KeyReleased event;
            event.alt     = e->altKey != 0;
            event.control = e->ctrlKey != 0;
            event.shift   = e->shiftKey != 0;
            event.system  = e->metaKey != 0;
            event.code    = key;
            window->pushHtmlEvent(event);
            return 1;
        }
        case EMSCRIPTEN_EVENT_KEYPRESS:
        {
            if (e->charCode == 0)
                return 1;

            sf::Event::TextEntered event;
            event.unicode = e->charCode;
            window->pushHtmlEvent(event);

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

EM_BOOL mouseCallback(int eventType, const EmscriptenMouseEvent* e, void* userData)
{
    mousePosition.x = e->clientX;
    mousePosition.y = e->clientY;

    if (!window)
        return 0;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        {
            sf::Event::MouseButtonPressed event{.position{e->clientX, e->clientY}};

            if (fullscreenPending)
            {
                requestFullscreen();
                fullscreenPending = false;
            }

            if (e->button == 0)
            {
                event.button                                                   = sf::Mouse::Button::Left;
                mouseStatus[static_cast<std::size_t>(sf::Mouse::Button::Left)] = true;
            }
            else if (e->button == 1)
            {
                event.button                                                     = sf::Mouse::Button::Middle;
                mouseStatus[static_cast<std::size_t>(sf::Mouse::Button::Middle)] = true;
            }
            else if (e->button == 2)
            {
                event.button                                                    = sf::Mouse::Button::Right;
                mouseStatus[static_cast<std::size_t>(sf::Mouse::Button::Right)] = true;
            }
            else
            {
                // TODO P0: extra buttons
                // event.button = sf::Mouse::ButtonCount;
            }

            window->pushHtmlEvent(event);
            return 1;
        }
        case EMSCRIPTEN_EVENT_MOUSEUP:
        {
            sf::Event::MouseButtonReleased event{.position{e->clientX, e->clientY}};

            if (e->button == 0)
            {
                event.button                                                   = sf::Mouse::Button::Left;
                mouseStatus[static_cast<std::size_t>(sf::Mouse::Button::Left)] = false;
            }
            else if (e->button == 1)
            {
                event.button                                                     = sf::Mouse::Button::Middle;
                mouseStatus[static_cast<std::size_t>(sf::Mouse::Button::Middle)] = false;
            }
            else if (e->button == 2)
            {
                event.button                                                    = sf::Mouse::Button::Right;
                mouseStatus[static_cast<std::size_t>(sf::Mouse::Button::Right)] = false;
            }
            else
            {
                // TODO P0: extra buttons
                // event.button = sf::Mouse::ButtonCount;
            }

            window->pushHtmlEvent(event);
            return 1;
        }
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
        {
            sf::Event::MouseMoved event{.position{e->clientX, e->clientY}};
            window->pushHtmlEvent(event);
            return 1;
        }
        case EMSCRIPTEN_EVENT_MOUSEENTER:
        {
            sf::Event::MouseEntered event;
            window->pushHtmlEvent(event);
            return 1;
        }
        case EMSCRIPTEN_EVENT_MOUSELEAVE:
        {
            sf::Event::MouseLeft event;
            window->pushHtmlEvent(event);
            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

EM_BOOL wheelCallback(int eventType, const EmscriptenWheelEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_WHEEL:
        {
            if (std::fabs(e->deltaY) > 0.0)
            {
                sf::Event::MouseWheelScrolled event;
                event.wheel    = sf::Mouse::Wheel::Vertical;
                event.delta    = -static_cast<float>(e->deltaY);
                event.position = {e->mouse.clientX, e->mouse.clientY};
                window->pushHtmlEvent(event);
            }

            if (std::fabs(e->deltaX) > 0.0)
            {
                sf::Event::MouseWheelScrolled event;

                event.wheel    = sf::Mouse::Wheel::Horizontal;
                event.delta    = static_cast<float>(e->deltaX);
                event.position = {e->mouse.clientX, e->mouse.clientY};
                window->pushHtmlEvent(event);
            }

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

EM_BOOL uieventCallback(int eventType, const EmscriptenUiEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_RESIZE:
        {
            int width, height, fullscreen;
            emscripten_get_canvas_size(&width, &height, &fullscreen);

            sf::Event::Resized event;
            event.size = {static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
            window->pushHtmlEvent(event);

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

EM_BOOL focuseventCallback(int eventType, const EmscriptenFocusEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_FOCUS:
        {
            sf::Event::FocusGained event;
            window->pushHtmlEvent(event);

            windowHasFocus = true;

            return 1;
        }
        case EMSCRIPTEN_EVENT_BLUR:
        {
            sf::Event::FocusLost event;
            window->pushHtmlEvent(event);

            windowHasFocus = false;

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

EM_BOOL deviceorientationCallback(int eventType, const EmscriptenDeviceOrientationEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        default:
            break;
    }

    return 0;
}

EM_BOOL devicemotionCallback(int eventType, const EmscriptenDeviceMotionEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        default:
            break;
    }

    return 0;
}

EM_BOOL orientationchangeCallback(int eventType, const EmscriptenOrientationChangeEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        default:
            break;
    }

    return 0;
}

EM_BOOL fullscreenchangeCallback(int eventType, const EmscriptenFullscreenChangeEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        default:
            break;
    }

    return 0;
}

EM_BOOL pointerlockchangeCallback(int eventType, const EmscriptenPointerlockChangeEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        default:
            break;
    }

    return 0;
}

EM_BOOL visibilitychangeCallback(int eventType, const EmscriptenVisibilityChangeEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        case EMSCRIPTEN_VISIBILITY_UNLOADED:
        {
            sf::Event::Closed event;
            window->pushHtmlEvent(event);

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

EM_BOOL touchCallback(int eventType, const EmscriptenTouchEvent* e, void* userData)
{
    if (!window)
        return 0;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_TOUCHSTART:
        {
            sf::Event::TouchBegan event;

            for (int i = 0; i < e->numTouches; ++i)
            {
                event.finger   = e->touches[i].identifier;
                event.position = {e->touches[i].clientX, e->touches[i].clientY};
                window->pushHtmlEvent(event);

                touchStatus.insert(std::make_pair(static_cast<unsigned int>(e->touches[i].identifier),
                                                  sf::Vector2i(e->touches[i].clientX, e->touches[i].clientY)));
            }

            return 1;
        }
        case EMSCRIPTEN_EVENT_TOUCHEND:
        {
            sf::Event::TouchEnded event;

            for (int i = 0; i < e->numTouches; ++i)
            {
                event.finger   = e->touches[i].identifier;
                event.position = {e->touches[i].clientX, e->touches[i].clientY};
                window->pushHtmlEvent(event);

                touchStatus.erase(static_cast<unsigned int>(e->touches[i].identifier));
            }

            return 1;
        }
        case EMSCRIPTEN_EVENT_TOUCHMOVE:
        {
            sf::Event::TouchMoved event;

            for (int i = 0; i < e->numTouches; ++i)
            {
                event.finger   = e->touches[i].identifier;
                event.position = {e->touches[i].clientX, e->touches[i].clientY};
                window->pushHtmlEvent(event);

                touchStatus[static_cast<unsigned int>(e->touches[i].identifier)] = sf::Vector2i(e->touches[i].clientX,
                                                                                                e->touches[i].clientY);
            }

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

EM_BOOL gamepadCallback(int eventType, const EmscriptenGamepadEvent* e, void* userData)
{
    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_GAMEPADCONNECTED:
        {
            bool previousConnected[sf::Joystick::Count];
            std::memcpy(previousConnected, joysticksConnected, sizeof(previousConnected));

            updatePluggedList();

            if (window)
            {
                for (int i = 0; i < sf::Joystick::Count; ++i)
                {
                    if (!previousConnected[i] && joysticksConnected[i])
                    {
                        sf::Event::JoystickConnected event;
                        event.joystickId = i;
                        window->pushHtmlEvent(event);
                    }
                }
            }

            return 1;
        }
        case EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED:
        {
            bool previousConnected[sf::Joystick::Count];
            std::memcpy(previousConnected, joysticksConnected, sizeof(previousConnected));

            updatePluggedList();

            if (window)
            {
                for (int i = 0; i < sf::Joystick::Count; ++i)
                {
                    if (previousConnected[i] && !joysticksConnected[i])
                    {
                        sf::Event::JoystickDisconnected event;
                        event.joystickId = i;
                        window->pushHtmlEvent(event);
                    }
                }
            }

            return 1;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void setCallbacks()
{
    static bool callbacksSet = false;

    if (callbacksSet)
        return;

    if (emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, keyCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set keypress callback";

    if (emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, keyCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set keydown callback";

    if (emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, keyCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set keyup callback";

    if (emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set click callback";

    if (emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set mousedown callback";

    if (emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set mouseup callback";

    if (emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set dblclick callback";

    if (emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set mousemove callback";

    if (emscripten_set_mouseenter_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set mouseenter callback";

    if (emscripten_set_mouseleave_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set mouseleave callback";

    if (emscripten_set_mouseover_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set mouseover callback";

    if (emscripten_set_mouseout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouseCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set mouseout callback";

    if (emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, wheelCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set wheel callback";

    if (emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, uieventCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set resize callback";

    if (emscripten_set_scroll_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, uieventCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set scroll callback";

    if (emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focuseventCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set blur callback";

    if (emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focuseventCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set focus callback";

    if (emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focuseventCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set focusin callback";

    if (emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focuseventCallback) !=
        EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set focusout callback";

    if (emscripten_set_deviceorientation_callback(0, 1, deviceorientationCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set deviceorientation callback";

    if (emscripten_set_devicemotion_callback(0, 1, devicemotionCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set devicemotion callback";

    if (emscripten_set_orientationchange_callback(0, 1, orientationchangeCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set orientationchange callback";

    if (!keyStatusInitialized)
    {
        for (unsigned int i = 0u; i < sf::Keyboard::KeyCount; ++i)
        {
            keyStatus[i] = false;
        }

        keyStatusInitialized = true;
    }

    if (!mouseStatusInitialized)
    {
        for (unsigned int i = 0u; i < sf::Mouse::ButtonCount; ++i)
        {
            mouseStatus[i] = false;
        }

        mouseStatusInitialized = true;
    }

    callbacksSet = true;
}
} // namespace


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
WindowImplEmscripten::WindowImplEmscripten(WindowHandle handle) : m_keyRepeatEnabled(true)
{
    err() << "Creating a window from a WindowHandle unsupported";
    std::abort();
}


////////////////////////////////////////////////////////////
WindowImplEmscripten::WindowImplEmscripten(VideoMode mode, const String& title, Style style, State state, const ContextSettings& settings) :
m_keyRepeatEnabled(true)
{
    if (window)
    {
        err() << "Creating multiple windows is unsupported";
        std::abort();
    }

    setCallbacks();

    window = this;

    setSize(mode.size);

    if (state == State::Fullscreen)
    {
        fullscreenPending = true;
    }
}


////////////////////////////////////////////////////////////
WindowImplEmscripten::~WindowImplEmscripten()
{
    window = nullptr;
}


////////////////////////////////////////////////////////////
WindowHandle WindowImplEmscripten::getNativeHandle() const
{
    // Not applicable
    return 0;
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::processEvents()
{
    // Not applicable
}


////////////////////////////////////////////////////////////
Vector2i WindowImplEmscripten::getPosition() const
{
    // Not applicable
    return Vector2i();
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setPosition(Vector2i position)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
Vector2u WindowImplEmscripten::getSize() const
{
    int width, height, fullscreen;
    emscripten_get_canvas_size(&width, &height, &fullscreen);

    return Vector2u(width, height);
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setSize(Vector2u size)
{
    emscripten_set_canvas_size(size.x, size.y);
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setTitle(const String& title)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setIcon(Vector2u size, const std::uint8_t* pixels)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setVisible(bool visible)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursorVisible(bool visible)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursorGrabbed(bool grabbed)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursor(const CursorImpl& cursor)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool WindowImplEmscripten::getKeyRepeatEnabled() const
{
    return m_keyRepeatEnabled;
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setKeyRepeatEnabled(bool enabled)
{
    m_keyRepeatEnabled = enabled;
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::requestFocus()
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool WindowImplEmscripten::hasFocus() const
{
    return windowHasFocus;
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::pushHtmlEvent(const Event& event)
{
    pushEvent(event);
}


////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(Keyboard::Key key)
{
    if (!keyStatusInitialized)
    {
        for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        {
            keyStatus[i] = false;
        }

        keyStatusInitialized = true;

        return false;
    }

    return keyStatus[static_cast<std::size_t>(key)];
}

////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(Keyboard::Scancode code)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
Keyboard::Key InputImpl::localize(Keyboard::Scancode code)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
Keyboard::Scancode InputImpl::delocalize(Keyboard::Key key)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
String InputImpl::getDescription(Keyboard::Scancode code)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void InputImpl::setVirtualKeyboardVisible(bool visible)
{
    // Not applicable
}

////////////////////////////////////////////////////////////
bool InputImpl::isMouseButtonPressed(Mouse::Button button)
{
    if (!mouseStatusInitialized)
    {
        for (int i = 0; i < sf::Mouse::ButtonCount; ++i)
        {
            mouseStatus[i] = false;
        }

        mouseStatusInitialized = true;

        return false;
    }

    return mouseStatus[static_cast<std::size_t>(button)];
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition()
{
    return mousePosition;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition(const WindowBase& relativeTo)
{
    return getMousePosition();
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(Vector2i position)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(Vector2i position, const WindowBase& relativeTo)
{
    setMousePosition(position);
}


////////////////////////////////////////////////////////////
bool InputImpl::isTouchDown(unsigned int finger)
{
    if (touchStatus.find(finger) == touchStatus.end())
        return false;

    return true;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int finger)
{
    std::map<unsigned int, Vector2i>::const_iterator iter = touchStatus.find(finger);
    if (iter == touchStatus.end())
        return Vector2i();

    return iter->second;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int finger, const WindowBase& relativeTo)
{
    return getTouchPosition(finger);
}

////////////////////////////////////////////////////////////
struct JoystickImpl::Impl
{
    int                      index;          ///< Index of the joystick
    Joystick::Identification identification; ///< Joystick identification
};


////////////////////////////////////////////////////////////
JoystickImpl::JoystickImpl() = default;


////////////////////////////////////////////////////////////
JoystickImpl::~JoystickImpl() = default;


////////////////////////////////////////////////////////////
void JoystickImpl::initialize()
{
    static bool callbacksSet = false;

    if (callbacksSet)
        return;

    if (emscripten_set_gamepadconnected_callback(0, 1, gamepadCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set gamepadconnected callback";

    if (emscripten_set_gamepaddisconnected_callback(0, 1, gamepadCallback) != EMSCRIPTEN_RESULT_SUCCESS)
        sf::priv::err() << "Failed to set gamepaddisconnected callback";

    callbacksSet = true;
}


////////////////////////////////////////////////////////////
void JoystickImpl::cleanup()
{
}


////////////////////////////////////////////////////////////
bool JoystickImpl::isConnected(unsigned int index)
{
    return joysticksConnected[index];
}


////////////////////////////////////////////////////////////
bool JoystickImpl::open(unsigned int index)
{
    if (!isConnected(index))
        return false;

    int numJoysticks = emscripten_get_num_gamepads();

    if (numJoysticks == EMSCRIPTEN_RESULT_NOT_SUPPORTED)
        return false;

    if (index >= numJoysticks)
        return false;

    EmscriptenGamepadEvent gamepadEvent;
    if (emscripten_get_gamepad_status(index, &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
    {
        sf::priv::err() << "Failed to get status of gamepad " << index;
        joysticksConnected[index] = false;
        return false;
    }

    if (!gamepadEvent.connected)
    {
        joysticksConnected[index] = false;
        return false;
    }

    m_impl->index = index;

    m_impl->identification.name      = StringUtfUtils::fromUtf8(gamepadEvent.id, gamepadEvent.id + 64);
    m_impl->identification.vendorId  = 0;
    m_impl->identification.productId = 0;

    return true;
}


////////////////////////////////////////////////////////////
void JoystickImpl::close()
{
    m_impl->index = 0;
}


////////////////////////////////////////////////////////////
JoystickCaps JoystickImpl::getCapabilities() const
{
    JoystickCaps caps;

    EmscriptenGamepadEvent gamepadEvent;
    if (emscripten_get_gamepad_status(m_impl->index, &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
    {
        sf::priv::err() << "Failed to get status of gamepad " << m_impl->index;
        joysticksConnected[m_impl->index] = false;
        return caps;
    }

    // Get the number of buttons
    caps.buttonCount = gamepadEvent.numButtons;

    if (caps.buttonCount > Joystick::ButtonCount)
        caps.buttonCount = Joystick::ButtonCount;

    // Only support the "standard" mapping for now
    if (std::strcmp(gamepadEvent.mapping, "standard") == 0)
    {
        caps.axes[Joystick::Axis::X]    = true;
        caps.axes[Joystick::Axis::Y]    = true;
        caps.axes[Joystick::Axis::Z]    = false;
        caps.axes[Joystick::Axis::R]    = true;
        caps.axes[Joystick::Axis::U]    = true;
        caps.axes[Joystick::Axis::V]    = false;
        caps.axes[Joystick::Axis::PovX] = false;
        caps.axes[Joystick::Axis::PovY] = false;
    }
    else
    {
        caps.axes[Joystick::Axis::X]    = false;
        caps.axes[Joystick::Axis::Y]    = false;
        caps.axes[Joystick::Axis::Z]    = false;
        caps.axes[Joystick::Axis::R]    = false;
        caps.axes[Joystick::Axis::U]    = false;
        caps.axes[Joystick::Axis::V]    = false;
        caps.axes[Joystick::Axis::PovX] = false;
        caps.axes[Joystick::Axis::PovY] = false;
    }

    return caps;
}


////////////////////////////////////////////////////////////
Joystick::Identification JoystickImpl::getIdentification() const
{
    return m_impl->identification;
}


////////////////////////////////////////////////////////////
JoystickState JoystickImpl::update()
{
    JoystickState state;

    EmscriptenGamepadEvent gamepadEvent;
    if (emscripten_get_gamepad_status(m_impl->index, &gamepadEvent) != EMSCRIPTEN_RESULT_SUCCESS)
    {
        sf::priv::err() << "Failed to get status of gamepad " << m_impl->index;
        joysticksConnected[m_impl->index] = false;
        return state;
    }

    for (int i = 0; (i < gamepadEvent.numButtons) && (i < Joystick::ButtonCount); ++i)
    {
        state.buttons[i] = gamepadEvent.digitalButton[i];
    }

    if (std::strcmp(gamepadEvent.mapping, "standard") == 0)
    {
        state.axes[Joystick::Axis::X] = static_cast<float>(gamepadEvent.axis[0] * 100.0);
        state.axes[Joystick::Axis::Y] = static_cast<float>(gamepadEvent.axis[1] * 100.0);
        state.axes[Joystick::Axis::R] = static_cast<float>(gamepadEvent.axis[2] * 100.0);
        state.axes[Joystick::Axis::U] = static_cast<float>(gamepadEvent.axis[3] * 100.0);
    }

    return state;
}


////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    VideoMode desktop = getDesktopMode();

    std::vector<VideoMode> modes;
    modes.push_back(desktop);
    return modes;
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    int width  = emscripten_run_script_int("screen.width");
    int height = emscripten_run_script_int("screen.height");
    return VideoMode({static_cast<unsigned int>(width), static_cast<unsigned int>(height)});
}

} // namespace priv
} // namespace sf
