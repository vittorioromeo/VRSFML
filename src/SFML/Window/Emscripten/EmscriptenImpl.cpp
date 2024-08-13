#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Emscripten/EmscriptenImpl.hpp>
#include <SFML/Window/Emscripten/JoystickImpl.hpp>
#include <SFML/Window/Emscripten/WindowImplEmscripten.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/InputImpl.hpp>
#include <SFML/Window/JoystickAxis.hpp>
#include <SFML/Window/JoystickIdentification.hpp>
#include <SFML/Window/JoystickImpl.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoModeImpl.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Window/WindowSettings.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/StringUtfUtils.hpp>

#include <SFML/Base/Math/Fabs.hpp>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <unordered_map>
#include <vector>


#define EMSCRIPTEN_TRY(...)                                                         \
    [&]() -> bool                                                                   \
    {                                                                               \
        if ((__VA_ARGS__) != EMSCRIPTEN_RESULT_SUCCESS)                             \
        {                                                                           \
            ::sf::priv::err() << "Emscripten operation failed: '" #__VA_ARGS__ "'"; \
            return false;                                                           \
        }                                                                           \
                                                                                    \
        return true;                                                                \
    }()


namespace
{
constinit sf::priv::WindowImplEmscripten* window = nullptr;

constinit bool windowHasFocus    = false;
constinit bool fullscreenPending = false;

constinit bool joysticksConnected[sf::Joystick::Count]{};
constinit bool keyStatus[sf::Keyboard::KeyCount]{};
constinit bool mouseStatus[sf::Mouse::ButtonCount]{};

constinit sf::Vector2i mousePosition{};

std::unordered_map<unsigned int, sf::Vector2i> touchStatus;

[[nodiscard]] sf::Keyboard::Key keyCodeToSF(unsigned long key, unsigned long location)
{
    // clang-format off
    switch (key)
    {
        case '\b': return sf::Keyboard::Key::Backspace;
        case '\t': return sf::Keyboard::Key::Tab;

        case '\r':
        {
            if (location == DOM_KEY_LOCATION_STANDARD) return sf::Keyboard::Key::Enter;
            if (location == DOM_KEY_LOCATION_NUMPAD)   return sf::Keyboard::Key::Enter;
            return sf::Keyboard::Key::Unknown;
        }

        case 16:
        {
            if (location == DOM_KEY_LOCATION_LEFT)  return sf::Keyboard::Key::LShift;
            if (location == DOM_KEY_LOCATION_RIGHT) return sf::Keyboard::Key::RShift;
            return sf::Keyboard::Key::Unknown;
        }

        case 17:
        {
            if (location == DOM_KEY_LOCATION_LEFT)  return sf::Keyboard::Key::LControl;
            if (location == DOM_KEY_LOCATION_RIGHT) return sf::Keyboard::Key::RControl;
            return sf::Keyboard::Key::Unknown;
        }

        case 18:
        {
            if (location == DOM_KEY_LOCATION_LEFT)  return sf::Keyboard::Key::LAlt;
            if (location == DOM_KEY_LOCATION_RIGHT) return sf::Keyboard::Key::RAlt;
            return sf::Keyboard::Key::Unknown;
        }

        case 19: return sf::Keyboard::Key::Pause;

        // case 20: Caps Lock

        case 27: return sf::Keyboard::Key::Escape;

        case ' ': return sf::Keyboard::Key::Space;
        case 33: return sf::Keyboard::Key::PageUp;
        case 34: return sf::Keyboard::Key::PageDown;
        case 35: return sf::Keyboard::Key::End;
        case 36: return sf::Keyboard::Key::Home;
        case 37: return sf::Keyboard::Key::Left;
        case 39: return sf::Keyboard::Key::Right;
        case 38: return sf::Keyboard::Key::Up;
        case 40: return sf::Keyboard::Key::Down;

        // case 42: Print Screen

        case 45: return sf::Keyboard::Key::Insert;
        case 46: return sf::Keyboard::Key::Delete;

        case ';': return sf::Keyboard::Key::Semicolon;
        case '=': return sf::Keyboard::Key::Equal;

        case 'A': return sf::Keyboard::Key::A;
        case 'Z': return sf::Keyboard::Key::Z;
        case 'E': return sf::Keyboard::Key::E;
        case 'R': return sf::Keyboard::Key::R;
        case 'T': return sf::Keyboard::Key::T;
        case 'Y': return sf::Keyboard::Key::Y;
        case 'U': return sf::Keyboard::Key::U;
        case 'I': return sf::Keyboard::Key::I;
        case 'O': return sf::Keyboard::Key::O;
        case 'P': return sf::Keyboard::Key::P;
        case 'Q': return sf::Keyboard::Key::Q;
        case 'S': return sf::Keyboard::Key::S;
        case 'D': return sf::Keyboard::Key::D;
        case 'F': return sf::Keyboard::Key::F;
        case 'G': return sf::Keyboard::Key::G;
        case 'H': return sf::Keyboard::Key::H;
        case 'J': return sf::Keyboard::Key::J;
        case 'K': return sf::Keyboard::Key::K;
        case 'L': return sf::Keyboard::Key::L;
        case 'M': return sf::Keyboard::Key::M;
        case 'W': return sf::Keyboard::Key::W;
        case 'X': return sf::Keyboard::Key::X;
        case 'C': return sf::Keyboard::Key::C;
        case 'V': return sf::Keyboard::Key::V;
        case 'B': return sf::Keyboard::Key::B;
        case 'N': return sf::Keyboard::Key::N;

        case '0': return sf::Keyboard::Key::Num0;
        case '1': return sf::Keyboard::Key::Num1;
        case '2': return sf::Keyboard::Key::Num2;
        case '3': return sf::Keyboard::Key::Num3;
        case '4': return sf::Keyboard::Key::Num4;
        case '5': return sf::Keyboard::Key::Num5;
        case '6': return sf::Keyboard::Key::Num6;
        case '7': return sf::Keyboard::Key::Num7;
        case '8': return sf::Keyboard::Key::Num8;
        case '9': return sf::Keyboard::Key::Num9;

        case 91:
        {
            if (location == DOM_KEY_LOCATION_LEFT)  return sf::Keyboard::Key::LSystem;
            if (location == DOM_KEY_LOCATION_RIGHT) return sf::Keyboard::Key::RSystem;
            return sf::Keyboard::Key::Unknown;
        }

        case 93: return sf::Keyboard::Key::Menu;

        case 96: return sf::Keyboard::Key::Numpad0;
        case 97: return sf::Keyboard::Key::Numpad1;
        case 98: return sf::Keyboard::Key::Numpad2;
        case 99: return sf::Keyboard::Key::Numpad3;
        case 100: return sf::Keyboard::Key::Numpad4;
        case 101: return sf::Keyboard::Key::Numpad5;
        case 102: return sf::Keyboard::Key::Numpad6;
        case 103: return sf::Keyboard::Key::Numpad7;
        case 104: return sf::Keyboard::Key::Numpad8;
        case 105: return sf::Keyboard::Key::Numpad9;

        case 106: return sf::Keyboard::Key::Multiply;
        case 107: return sf::Keyboard::Key::Add;
        case 109: return sf::Keyboard::Key::Subtract;
        case 111: return sf::Keyboard::Key::Divide;

        case 112: return sf::Keyboard::Key::F1;
        case 113: return sf::Keyboard::Key::F2;
        case 114: return sf::Keyboard::Key::F3;
        case 115: return sf::Keyboard::Key::F4;
        case 116: return sf::Keyboard::Key::F5;
        case 117: return sf::Keyboard::Key::F6;
        case 118: return sf::Keyboard::Key::F7;
        case 119: return sf::Keyboard::Key::F8;
        case 120: return sf::Keyboard::Key::F9;
        case 121: return sf::Keyboard::Key::F10;
        case 122: return sf::Keyboard::Key::F11;
        case 123: return sf::Keyboard::Key::F12;
        case 124: return sf::Keyboard::Key::F13;
        case 125: return sf::Keyboard::Key::F14;
        case 126: return sf::Keyboard::Key::F15;

        // case 144: Num Lock
        // case 145: Scroll Lock

        case 173: return sf::Keyboard::Key::Hyphen;

        case 188: return sf::Keyboard::Key::Comma;

        case 190: return sf::Keyboard::Key::Period;
        case 191: return sf::Keyboard::Key::Slash;
        case 192: return sf::Keyboard::Key::Grave;

        case 219: return sf::Keyboard::Key::LBracket;
        case 220: return sf::Keyboard::Key::Backslash;
        case 221: return sf::Keyboard::Key::RBracket;
        case 222: return sf::Keyboard::Key::Apostrophe;
    }
    // clang-format on

    return sf::Keyboard::Key::Unknown;
}

bool updatePluggedList()
{
    SFML_BASE_ASSERT(window != nullptr);

    const int numJoysticks = emscripten_get_num_gamepads();

    if (numJoysticks == EMSCRIPTEN_RESULT_NOT_SUPPORTED)
    {
        for (bool& flag : joysticksConnected)
            flag = false;

        return false;
    }

    for (unsigned int i = 0u; (i < sf::Joystick::Count) && (i < static_cast<unsigned int>(numJoysticks)); ++i)
    {
        EmscriptenGamepadEvent gamepadEvent;

        joysticksConnected[i] = EMSCRIPTEN_TRY(emscripten_get_gamepad_status(static_cast<int>(i), &gamepadEvent))
                                    ? gamepadEvent.connected
                                    : false;
    }

    return true;
}

[[nodiscard]] EM_BOOL canvasSizeChangedCallback(int /* eventType */, const void* /* reserved */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    int width{};
    int height{};
    if (!EMSCRIPTEN_TRY(emscripten_get_canvas_element_size("#canvas", &width, &height)))
        return EM_FALSE;

    window->pushHtmlEvent(sf::Event::Resized{.size{static_cast<unsigned int>(width), static_cast<unsigned int>(height)}});
    return EM_TRUE;
}

void requestFullscreen()
{
    const EmscriptenFullscreenStrategy fullscreenStrategy{.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH,
                                                          .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF,
                                                          .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_BILINEAR,
                                                          .canvasResizedCallback         = canvasSizeChangedCallback,
                                                          .canvasResizedCallbackUserData = nullptr,
                                                          .canvasResizedCallbackTargetThread = {}};

    emscripten_request_fullscreen_strategy("#canvas", /* deferUntilInEventHandler */ EM_FALSE, &fullscreenStrategy);
}

[[nodiscard]] EM_BOOL keyCallback(int eventType, const EmscriptenKeyboardEvent* e, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    const sf::Keyboard::Key key = keyCodeToSF(e->which, e->location);

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_KEYDOWN:
        {
            if (e->repeat && !window->getKeyRepeatEnabled())
                return EM_TRUE;

            if (fullscreenPending)
            {
                requestFullscreen();
                fullscreenPending = false;
            }

            keyStatus[static_cast<std::size_t>(key)] = true;
            window->pushHtmlEvent(sf::Event::KeyPressed{.code    = key,
                                                        .alt     = e->altKey != 0,
                                                        .control = e->ctrlKey != 0,
                                                        .shift   = e->shiftKey != 0,
                                                        .system  = e->metaKey != 0});

            // We try to prevent some keystrokes from bubbling
            // If we try to prevent bubbling for all keys,
            // it prevents keypress events from being generated
            if ((key == sf::Keyboard::Key::Tab) || (key == sf::Keyboard::Key::Backspace) ||
                (key == sf::Keyboard::Key::Menu) || (key == sf::Keyboard::Key::LSystem) ||
                (key == sf::Keyboard::Key::RSystem))
                return EM_TRUE;

            return EM_FALSE;
        }
        case EMSCRIPTEN_EVENT_KEYUP:
        {
            keyStatus[static_cast<std::size_t>(key)] = false;

            window->pushHtmlEvent(sf::Event::KeyReleased{.code    = key,
                                                         .alt     = e->altKey != 0,
                                                         .control = e->ctrlKey != 0,
                                                         .shift   = e->shiftKey != 0,
                                                         .system  = e->metaKey != 0});
            return EM_TRUE;
        }
        case EMSCRIPTEN_EVENT_KEYPRESS:
        {
            if (e->charCode == 0)
                return EM_TRUE;

            window->pushHtmlEvent(sf::Event::TextEntered{.unicode = e->charCode});
            return EM_TRUE;
        }
        default:
        {
            break;
        }
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL mouseCallback(int eventType, const EmscriptenMouseEvent* e, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    mousePosition = {e->targetX, e->targetY};

    const auto handleMouseEvent = [&]<typename TEvent, bool TDown>
    {
        const auto pushEventAndUpdateStatus = [&](sf::Mouse::Button sfButton)
        {
            window->pushHtmlEvent(sf::Event::MouseButtonPressed{.button = sfButton, .position = mousePosition});
            mouseStatus[static_cast<std::size_t>(sfButton)] = TDown;
        };

        // clang-format off
        if (e->button == 0) return pushEventAndUpdateStatus(sf::Mouse::Button::Left);
        if (e->button == 1) return pushEventAndUpdateStatus(sf::Mouse::Button::Middle);
        if (e->button == 2) return pushEventAndUpdateStatus(sf::Mouse::Button::Right);
        if (e->button == 3) return pushEventAndUpdateStatus(sf::Mouse::Button::Extra1);
        if (e->button == 4) return pushEventAndUpdateStatus(sf::Mouse::Button::Extra2);
        // clang-format on
    };

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        {
            if (fullscreenPending)
            {
                requestFullscreen();
                fullscreenPending = false;
            }

            handleMouseEvent.operator()<sf::Event::MouseButtonPressed, true>();
            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_MOUSEUP:
        {
            handleMouseEvent.operator()<sf::Event::MouseButtonReleased, false>();
            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_MOUSEMOVE:
        {
            window->pushHtmlEvent(sf::Event::MouseMoved{.position = mousePosition});
            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_MOUSEENTER:
        {
            window->pushHtmlEvent(sf::Event::MouseEntered{});
            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_MOUSELEAVE:
        {
            window->pushHtmlEvent(sf::Event::MouseLeft{});
            return EM_TRUE;
        }
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL wheelCallback(int eventType, const EmscriptenWheelEvent* e, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    if (eventType != EMSCRIPTEN_EVENT_WHEEL)
        return EM_FALSE;

    if (sf::base::fabs(e->deltaY) > 0.0)
        window->pushHtmlEvent(sf::Event::MouseWheelScrolled{.wheel    = sf::Mouse::Wheel::Vertical,
                                                            .delta    = -static_cast<float>(e->deltaY),
                                                            .position = {e->mouse.targetX, e->mouse.targetY}});

    if (sf::base::fabs(e->deltaX) > 0.0)
        window->pushHtmlEvent(sf::Event::MouseWheelScrolled{.wheel    = sf::Mouse::Wheel::Horizontal,
                                                            .delta    = static_cast<float>(e->deltaX),
                                                            .position = {e->mouse.targetX, e->mouse.targetY}});

    return EM_TRUE;
}

[[nodiscard]] EM_BOOL uieventCallback(int eventType, const EmscriptenUiEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    if (eventType != EMSCRIPTEN_EVENT_RESIZE)
        return EM_FALSE;

    int width{};
    int height{};
    if (!EMSCRIPTEN_TRY(emscripten_get_canvas_element_size("#canvas", &width, &height)))
        return EM_FALSE;

    window->pushHtmlEvent(sf::Event::Resized{.size = {static_cast<unsigned int>(width), static_cast<unsigned int>(height)}});
    return EM_TRUE;
}

[[nodiscard]] EM_BOOL focuseventCallback(int eventType, const EmscriptenFocusEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_FOCUS:
        {
            window->pushHtmlEvent(sf::Event::FocusGained{});
            windowHasFocus = true;
            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_BLUR:
        {
            window->pushHtmlEvent(sf::Event::FocusLost{});
            windowHasFocus = false;
            return EM_TRUE;
        }
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL deviceorientationCallback(int eventType, const EmscriptenDeviceOrientationEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        default:
            break;
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL devicemotionCallback(int eventType, const EmscriptenDeviceMotionEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        default:
            break;
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL orientationchangeCallback(int eventType, const EmscriptenOrientationChangeEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        default:
            break;
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL fullscreenchangeCallback(int eventType, const EmscriptenFullscreenChangeEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        default:
            break;
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL pointerlockchangeCallback(int eventType, const EmscriptenPointerlockChangeEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        default:
            break;
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL visibilitychangeCallback(int eventType, const EmscriptenVisibilityChangeEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        case EMSCRIPTEN_VISIBILITY_UNLOADED:
        {
            sf::Event::Closed event;
            window->pushHtmlEvent(event);

            return EM_TRUE;
        }
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL touchCallback(int eventType, const EmscriptenTouchEvent* e, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    const auto pushTouchEvent = [&]<typename TEvent>(int touchIdx)
    {
        sf::Event::TouchEnded event;
        event.finger   = static_cast<unsigned int>(e->touches[touchIdx].identifier);
        event.position = {e->touches[touchIdx].clientX, e->touches[touchIdx].clientY};
        window->pushHtmlEvent(event);
    };

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_TOUCHSTART:
        {
            for (int i = 0; i < e->numTouches; ++i)
            {
                pushTouchEvent.operator()<sf::Event::TouchBegan>(i);

                touchStatus.emplace(static_cast<unsigned int>(e->touches[i].identifier),
                                    sf::Vector2i{e->touches[i].clientX, e->touches[i].clientY});
            }

            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_TOUCHEND:
        {
            for (int i = 0; i < e->numTouches; ++i)
            {
                pushTouchEvent.operator()<sf::Event::TouchEnded>(i);

                touchStatus.erase(static_cast<unsigned int>(e->touches[i].identifier));
            }

            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_TOUCHMOVE:
        {
            for (int i = 0; i < e->numTouches; ++i)
            {
                pushTouchEvent.operator()<sf::Event::TouchMoved>(i);

                touchStatus[static_cast<unsigned int>(e->touches[i].identifier)] = {e->touches[i].clientX,
                                                                                    e->touches[i].clientY};
            }

            return EM_TRUE;
        }
    }

    return EM_FALSE;
}

[[nodiscard]] EM_BOOL gamepadCallback(int eventType, const EmscriptenGamepadEvent* /* e */, void* /* userData */)
{
    if (!window)
        return EM_FALSE;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_GAMEPADCONNECTED:
        {
            bool previousConnected[sf::Joystick::Count];
            std::memcpy(previousConnected, joysticksConnected, sizeof(previousConnected));

            updatePluggedList();

            for (unsigned int i = 0u; i < sf::Joystick::Count; ++i)
                if (!previousConnected[i] && joysticksConnected[i])
                    window->pushHtmlEvent(sf::Event::JoystickConnected{.joystickId = i});

            return EM_TRUE;
        }

        case EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED:
        {
            bool previousConnected[sf::Joystick::Count];
            std::memcpy(previousConnected, joysticksConnected, sizeof(previousConnected));

            updatePluggedList();

            for (unsigned int i = 0u; i < sf::Joystick::Count; ++i)
                if (previousConnected[i] && !joysticksConnected[i])
                    window->pushHtmlEvent(sf::Event::JoystickDisconnected{.joystickId = i});

            return EM_TRUE;
        }
    }

    return EM_FALSE;
}

void setCallbacks()
{
    static bool callbacksSet = false;

    if (callbacksSet)
        return;

    constexpr void* nullUserData = nullptr;
    constexpr bool  useCapture   = true;
    const auto*     windowTarget = EMSCRIPTEN_EVENT_TARGET_WINDOW;
    constexpr auto  canvasTarget = "#canvas";

    EMSCRIPTEN_TRY(emscripten_set_keypress_callback(windowTarget, nullUserData, useCapture, keyCallback));
    EMSCRIPTEN_TRY(emscripten_set_keydown_callback(windowTarget, nullUserData, useCapture, keyCallback));
    EMSCRIPTEN_TRY(emscripten_set_keyup_callback(windowTarget, nullUserData, useCapture, keyCallback));
    EMSCRIPTEN_TRY(emscripten_set_click_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_mousedown_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_mouseup_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_dblclick_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_mousemove_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_mouseenter_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_mouseleave_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_mouseover_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_mouseout_callback(canvasTarget, nullUserData, useCapture, mouseCallback));
    EMSCRIPTEN_TRY(emscripten_set_wheel_callback(canvasTarget, nullUserData, useCapture, wheelCallback));
    EMSCRIPTEN_TRY(emscripten_set_resize_callback(canvasTarget, nullUserData, useCapture, uieventCallback));
    EMSCRIPTEN_TRY(emscripten_set_scroll_callback(canvasTarget, nullUserData, useCapture, uieventCallback));
    EMSCRIPTEN_TRY(emscripten_set_blur_callback(canvasTarget, nullUserData, useCapture, focuseventCallback));
    EMSCRIPTEN_TRY(emscripten_set_focus_callback(windowTarget, nullUserData, useCapture, focuseventCallback));
    EMSCRIPTEN_TRY(emscripten_set_focusin_callback(windowTarget, nullUserData, useCapture, focuseventCallback));
    EMSCRIPTEN_TRY(emscripten_set_focusout_callback(windowTarget, nullUserData, useCapture, focuseventCallback));
    EMSCRIPTEN_TRY(emscripten_set_deviceorientation_callback(nullUserData, useCapture, deviceorientationCallback));
    EMSCRIPTEN_TRY(emscripten_set_devicemotion_callback(nullUserData, useCapture, devicemotionCallback));
    EMSCRIPTEN_TRY(emscripten_set_orientationchange_callback(nullUserData, useCapture, orientationchangeCallback));
    EMSCRIPTEN_TRY(emscripten_set_touchstart_callback(canvasTarget, nullUserData, useCapture, touchCallback));
    EMSCRIPTEN_TRY(emscripten_set_touchend_callback(canvasTarget, nullUserData, useCapture, touchCallback));
    EMSCRIPTEN_TRY(emscripten_set_touchmove_callback(canvasTarget, nullUserData, useCapture, touchCallback));
    EMSCRIPTEN_TRY(emscripten_set_touchcancel_callback(canvasTarget, nullUserData, useCapture, touchCallback));

    callbacksSet = true;
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
void EmscriptenImpl::setVSyncEnabler(const VSyncEnabler& vSyncEnabler)
{
    tlVSyncEnabler = vSyncEnabler;
}


////////////////////////////////////////////////////////////
void EmscriptenImpl::invokeAndClearVSyncEnabler()
{
    if (!tlVSyncEnabler) [[likely]]
        return;

    tlVSyncEnabler();
    tlVSyncEnabler = {};
}


////////////////////////////////////////////////////////////
void EmscriptenImpl::killWindow()
{
    glCheck(glClearColor(0.f, 0.f, 0.f, 1.f));
    glCheck(glClear(GL_COLOR_BUFFER_BIT));

    window = nullptr;
}


////////////////////////////////////////////////////////////
WindowImplEmscripten::WindowImplEmscripten(WindowHandle /* handle */) : m_keyRepeatEnabled(true)
{
    err() << "Creating a window from a WindowHandle unsupported";
    std::abort();
}


////////////////////////////////////////////////////////////
WindowImplEmscripten::WindowImplEmscripten(const WindowSettings& windowSettings) : m_keyRepeatEnabled(true)
{
    if (window != nullptr)
    {
        err() << "Creating multiple windows is unsupported";
        std::abort();
    }

    setCallbacks();

    window = this;

    setSize(windowSettings.size);

    if (windowSettings.state == State::Fullscreen)
        fullscreenPending = true;
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
    return {};
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
    return {};
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setPosition(Vector2i /* position */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
Vector2u WindowImplEmscripten::getSize() const
{
    int width{};
    int height{};
    EMSCRIPTEN_TRY(emscripten_get_canvas_element_size("#canvas", &width, &height));

    return {static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setSize(Vector2u size)
{
    emscripten_set_canvas_element_size("#canvas", static_cast<int>(size.x), static_cast<int>(size.y));
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setTitle(const String& /* title */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setIcon(Vector2u /* size */, const std::uint8_t* /* pixels */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setVisible(bool /* visible */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursorVisible(bool /* visible */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursorGrabbed(bool /* grabbed */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursor(const CursorImpl& /* cursor */)
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
    return keyStatus[static_cast<std::size_t>(key)];
}

////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(Keyboard::Scancode /* code */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
Keyboard::Key InputImpl::localize(Keyboard::Scancode /* code */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
Keyboard::Scancode InputImpl::delocalize(Keyboard::Key /* key */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
String InputImpl::getDescription(Keyboard::Scancode /* code */)
{
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
void InputImpl::setVirtualKeyboardVisible(bool /* visible */)
{
    // Not applicable
}

////////////////////////////////////////////////////////////
bool InputImpl::isMouseButtonPressed(Mouse::Button button)
{
    return mouseStatus[static_cast<std::size_t>(button)];
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition()
{
    return mousePosition;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition(const WindowBase& /* relativeTo */)
{
    return getMousePosition();
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(Vector2i /* position */)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(Vector2i position, const WindowBase& /* relativeTo */)
{
    setMousePosition(position);
}


////////////////////////////////////////////////////////////
bool InputImpl::isTouchDown(unsigned int finger)
{
    return touchStatus.find(finger) != touchStatus.end();
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int finger)
{
    const auto iter = touchStatus.find(finger);
    return iter == touchStatus.end() ? Vector2i() : iter->second;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int finger, const WindowBase& /* relativeTo */)
{
    return getTouchPosition(finger);
}


////////////////////////////////////////////////////////////
struct JoystickImpl::Impl
{
    int                      index{-1};      ///< Index of the joystick
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

    EMSCRIPTEN_TRY(emscripten_set_gamepadconnected_callback(nullptr, /* useCapture */ true, gamepadCallback));
    EMSCRIPTEN_TRY(emscripten_set_gamepaddisconnected_callback(nullptr, /* useCapture */ true, gamepadCallback));

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

    const int numJoysticks = emscripten_get_num_gamepads();

    if (numJoysticks == EMSCRIPTEN_RESULT_NOT_SUPPORTED)
        return false;

    if (static_cast<int>(index) >= numJoysticks)
        return false;

    EmscriptenGamepadEvent gamepadEvent;
    if (!EMSCRIPTEN_TRY(emscripten_get_gamepad_status(static_cast<int>(index), &gamepadEvent)) || !gamepadEvent.connected)
    {
        joysticksConnected[index] = false;
        return false;
    }

    m_impl->index = static_cast<int>(index);

    m_impl->identification.name      = StringUtfUtils::fromUtf8(gamepadEvent.id, gamepadEvent.id + 64);
    m_impl->identification.vendorId  = 0u;
    m_impl->identification.productId = 0u;

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
    if (!EMSCRIPTEN_TRY(emscripten_get_gamepad_status(m_impl->index, &gamepadEvent)))
    {
        joysticksConnected[m_impl->index] = false;
        return caps;
    }

    // Get the number of buttons
    caps.buttonCount = static_cast<unsigned int>(gamepadEvent.numButtons);

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
    if (!EMSCRIPTEN_TRY(emscripten_get_gamepad_status(m_impl->index, &gamepadEvent)))
    {
        joysticksConnected[m_impl->index] = false;
        return state;
    }

    for (int i = 0; (i < gamepadEvent.numButtons) && (i < static_cast<int>(Joystick::ButtonCount)); ++i)
        state.buttons[i] = gamepadEvent.digitalButton[i];

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
    return {getDesktopMode()};
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    return VideoMode{{static_cast<unsigned int>(emscripten_run_script_int("screen.width")),
                      static_cast<unsigned int>(emscripten_run_script_int("screen.height"))}};
}

} // namespace sf::priv
