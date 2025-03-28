// TODO P0: cleanup, add error handling, add missing events, etc...

#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/WindowImpl.hpp"
#include "SFML/Window/WindowImplSDL.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/Utf.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

#include <string>
#include <unordered_map>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr sf::Keyboard::Scan mapSDLScancodeToSFML(const SDL_Scancode sdlCode) noexcept
{
    // clang-format off
    switch (sdlCode)
    {
        // Letters (SDL: A=4, B=5, ... Z=29)
        case SDL_SCANCODE_A: return sf::Keyboard::Scan::A;
        case SDL_SCANCODE_B: return sf::Keyboard::Scan::B;
        case SDL_SCANCODE_C: return sf::Keyboard::Scan::C;
        case SDL_SCANCODE_D: return sf::Keyboard::Scan::D;
        case SDL_SCANCODE_E: return sf::Keyboard::Scan::E;
        case SDL_SCANCODE_F: return sf::Keyboard::Scan::F;
        case SDL_SCANCODE_G: return sf::Keyboard::Scan::G;
        case SDL_SCANCODE_H: return sf::Keyboard::Scan::H;
        case SDL_SCANCODE_I: return sf::Keyboard::Scan::I;
        case SDL_SCANCODE_J: return sf::Keyboard::Scan::J;
        case SDL_SCANCODE_K: return sf::Keyboard::Scan::K;
        case SDL_SCANCODE_L: return sf::Keyboard::Scan::L;
        case SDL_SCANCODE_M: return sf::Keyboard::Scan::M;
        case SDL_SCANCODE_N: return sf::Keyboard::Scan::N;
        case SDL_SCANCODE_O: return sf::Keyboard::Scan::O;
        case SDL_SCANCODE_P: return sf::Keyboard::Scan::P;
        case SDL_SCANCODE_Q: return sf::Keyboard::Scan::Q;
        case SDL_SCANCODE_R: return sf::Keyboard::Scan::R;
        case SDL_SCANCODE_S: return sf::Keyboard::Scan::S;
        case SDL_SCANCODE_T: return sf::Keyboard::Scan::T;
        case SDL_SCANCODE_U: return sf::Keyboard::Scan::U;
        case SDL_SCANCODE_V: return sf::Keyboard::Scan::V;
        case SDL_SCANCODE_W: return sf::Keyboard::Scan::W;
        case SDL_SCANCODE_X: return sf::Keyboard::Scan::X;
        case SDL_SCANCODE_Y: return sf::Keyboard::Scan::Y;
        case SDL_SCANCODE_Z: return sf::Keyboard::Scan::Z;

        // Numbers
        case SDL_SCANCODE_1: return sf::Keyboard::Scan::Num1;
        case SDL_SCANCODE_2: return sf::Keyboard::Scan::Num2;
        case SDL_SCANCODE_3: return sf::Keyboard::Scan::Num3;
        case SDL_SCANCODE_4: return sf::Keyboard::Scan::Num4;
        case SDL_SCANCODE_5: return sf::Keyboard::Scan::Num5;
        case SDL_SCANCODE_6: return sf::Keyboard::Scan::Num6;
        case SDL_SCANCODE_7: return sf::Keyboard::Scan::Num7;
        case SDL_SCANCODE_8: return sf::Keyboard::Scan::Num8;
        case SDL_SCANCODE_9: return sf::Keyboard::Scan::Num9;
        case SDL_SCANCODE_0: return sf::Keyboard::Scan::Num0;

        // Control and special keys
        case SDL_SCANCODE_RETURN:       return sf::Keyboard::Scan::Enter;
        case SDL_SCANCODE_ESCAPE:       return sf::Keyboard::Scan::Escape;
        case SDL_SCANCODE_BACKSPACE:    return sf::Keyboard::Scan::Backspace;
        case SDL_SCANCODE_TAB:          return sf::Keyboard::Scan::Tab;
        case SDL_SCANCODE_SPACE:        return sf::Keyboard::Scan::Space;
        case SDL_SCANCODE_MINUS:        return sf::Keyboard::Scan::Hyphen;
        case SDL_SCANCODE_EQUALS:       return sf::Keyboard::Scan::Equal;
        case SDL_SCANCODE_LEFTBRACKET:  return sf::Keyboard::Scan::LBracket;
        case SDL_SCANCODE_RIGHTBRACKET: return sf::Keyboard::Scan::RBracket;
        case SDL_SCANCODE_BACKSLASH:    return sf::Keyboard::Scan::Backslash;
        case SDL_SCANCODE_SEMICOLON:    return sf::Keyboard::Scan::Semicolon;
        case SDL_SCANCODE_APOSTROPHE:   return sf::Keyboard::Scan::Apostrophe;
        case SDL_SCANCODE_GRAVE:        return sf::Keyboard::Scan::Grave;
        case SDL_SCANCODE_COMMA:        return sf::Keyboard::Scan::Comma;
        case SDL_SCANCODE_PERIOD:       return sf::Keyboard::Scan::Period;
        case SDL_SCANCODE_SLASH:        return sf::Keyboard::Scan::Slash;

        // Function keys
        case SDL_SCANCODE_F1:  return sf::Keyboard::Scan::F1;
        case SDL_SCANCODE_F2:  return sf::Keyboard::Scan::F2;
        case SDL_SCANCODE_F3:  return sf::Keyboard::Scan::F3;
        case SDL_SCANCODE_F4:  return sf::Keyboard::Scan::F4;
        case SDL_SCANCODE_F5:  return sf::Keyboard::Scan::F5;
        case SDL_SCANCODE_F6:  return sf::Keyboard::Scan::F6;
        case SDL_SCANCODE_F7:  return sf::Keyboard::Scan::F7;
        case SDL_SCANCODE_F8:  return sf::Keyboard::Scan::F8;
        case SDL_SCANCODE_F9:  return sf::Keyboard::Scan::F9;
        case SDL_SCANCODE_F10: return sf::Keyboard::Scan::F10;
        case SDL_SCANCODE_F11: return sf::Keyboard::Scan::F11;
        case SDL_SCANCODE_F12: return sf::Keyboard::Scan::F12;
        case SDL_SCANCODE_F13: return sf::Keyboard::Scan::F13;
        case SDL_SCANCODE_F14: return sf::Keyboard::Scan::F14;
        case SDL_SCANCODE_F15: return sf::Keyboard::Scan::F15;
        case SDL_SCANCODE_F16: return sf::Keyboard::Scan::F16;
        case SDL_SCANCODE_F17: return sf::Keyboard::Scan::F17;
        case SDL_SCANCODE_F18: return sf::Keyboard::Scan::F18;
        case SDL_SCANCODE_F19: return sf::Keyboard::Scan::F19;
        case SDL_SCANCODE_F20: return sf::Keyboard::Scan::F20;
        case SDL_SCANCODE_F21: return sf::Keyboard::Scan::F21;
        case SDL_SCANCODE_F22: return sf::Keyboard::Scan::F22;
        case SDL_SCANCODE_F23: return sf::Keyboard::Scan::F23;
        case SDL_SCANCODE_F24: return sf::Keyboard::Scan::F24;

        // Lock and navigation keys
        case SDL_SCANCODE_CAPSLOCK:     return sf::Keyboard::Scan::CapsLock;
        case SDL_SCANCODE_PRINTSCREEN:  return sf::Keyboard::Scan::PrintScreen;
        case SDL_SCANCODE_SCROLLLOCK:   return sf::Keyboard::Scan::ScrollLock;
        case SDL_SCANCODE_PAUSE:        return sf::Keyboard::Scan::Pause;
        case SDL_SCANCODE_INSERT:       return sf::Keyboard::Scan::Insert;
        case SDL_SCANCODE_HOME:         return sf::Keyboard::Scan::Home;
        case SDL_SCANCODE_PAGEUP:       return sf::Keyboard::Scan::PageUp;
        case SDL_SCANCODE_DELETE:       return sf::Keyboard::Scan::Delete;
        case SDL_SCANCODE_END:          return sf::Keyboard::Scan::End;
        case SDL_SCANCODE_PAGEDOWN:     return sf::Keyboard::Scan::PageDown;
        case SDL_SCANCODE_RIGHT:        return sf::Keyboard::Scan::Right;
        case SDL_SCANCODE_LEFT:         return sf::Keyboard::Scan::Left;
        case SDL_SCANCODE_DOWN:         return sf::Keyboard::Scan::Down;
        case SDL_SCANCODE_UP:           return sf::Keyboard::Scan::Up;
        case SDL_SCANCODE_NUMLOCKCLEAR: return sf::Keyboard::Scan::NumLock;

        // Keypad
        case SDL_SCANCODE_KP_DIVIDE:   return sf::Keyboard::Scan::NumpadDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return sf::Keyboard::Scan::NumpadMultiply;
        case SDL_SCANCODE_KP_MINUS:    return sf::Keyboard::Scan::NumpadMinus;
        case SDL_SCANCODE_KP_PLUS:     return sf::Keyboard::Scan::NumpadPlus;
        case SDL_SCANCODE_KP_ENTER:    return sf::Keyboard::Scan::NumpadEnter;
        case SDL_SCANCODE_KP_PERIOD:   return sf::Keyboard::Scan::NumpadDecimal;
        case SDL_SCANCODE_KP_1:        return sf::Keyboard::Scan::Numpad1;
        case SDL_SCANCODE_KP_2:        return sf::Keyboard::Scan::Numpad2;
        case SDL_SCANCODE_KP_3:        return sf::Keyboard::Scan::Numpad3;
        case SDL_SCANCODE_KP_4:        return sf::Keyboard::Scan::Numpad4;
        case SDL_SCANCODE_KP_5:        return sf::Keyboard::Scan::Numpad5;
        case SDL_SCANCODE_KP_6:        return sf::Keyboard::Scan::Numpad6;
        case SDL_SCANCODE_KP_7:        return sf::Keyboard::Scan::Numpad7;
        case SDL_SCANCODE_KP_8:        return sf::Keyboard::Scan::Numpad8;
        case SDL_SCANCODE_KP_9:        return sf::Keyboard::Scan::Numpad9;
        case SDL_SCANCODE_KP_0:        return sf::Keyboard::Scan::Numpad0;

        // Additional keys
        case SDL_SCANCODE_NONUSBACKSLASH: return sf::Keyboard::Scan::NonUsBackslash;
        case SDL_SCANCODE_APPLICATION:    return sf::Keyboard::Scan::Application;
        case SDL_SCANCODE_HELP:           return sf::Keyboard::Scan::Help;
        case SDL_SCANCODE_MENU:           return sf::Keyboard::Scan::Menu;
        case SDL_SCANCODE_SELECT:         return sf::Keyboard::Scan::Select;
        case SDL_SCANCODE_AGAIN:          return sf::Keyboard::Scan::Redo;
        case SDL_SCANCODE_UNDO:           return sf::Keyboard::Scan::Undo;
        case SDL_SCANCODE_CUT:            return sf::Keyboard::Scan::Cut;
        case SDL_SCANCODE_COPY:           return sf::Keyboard::Scan::Copy;
        case SDL_SCANCODE_PASTE:          return sf::Keyboard::Scan::Paste;
        case SDL_SCANCODE_MUTE:           return sf::Keyboard::Scan::VolumeMute;
        case SDL_SCANCODE_VOLUMEUP:       return sf::Keyboard::Scan::VolumeUp;
        case SDL_SCANCODE_VOLUMEDOWN:     return sf::Keyboard::Scan::VolumeDown;

        // Media keys
        case SDL_SCANCODE_MEDIA_PLAY:           return sf::Keyboard::Scan::MediaPlayPause;
        case SDL_SCANCODE_MEDIA_PAUSE:          return sf::Keyboard::Scan::MediaPlayPause;
        case SDL_SCANCODE_MEDIA_STOP:           return sf::Keyboard::Scan::MediaStop;
        case SDL_SCANCODE_MEDIA_NEXT_TRACK:     return sf::Keyboard::Scan::MediaNextTrack;
        case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK: return sf::Keyboard::Scan::MediaPreviousTrack;

        case SDL_SCANCODE_LCTRL:  return sf::Keyboard::Scan::LControl;
        case SDL_SCANCODE_LSHIFT: return sf::Keyboard::Scan::LShift;
        case SDL_SCANCODE_LALT:   return sf::Keyboard::Scan::LAlt;
        case SDL_SCANCODE_LGUI:   return sf::Keyboard::Scan::LSystem;
        case SDL_SCANCODE_RCTRL:  return sf::Keyboard::Scan::RControl;
        case SDL_SCANCODE_RSHIFT: return sf::Keyboard::Scan::RShift;
        case SDL_SCANCODE_RALT:   return sf::Keyboard::Scan::RAlt;
        case SDL_SCANCODE_RGUI:   return sf::Keyboard::Scan::RSystem;
        case SDL_SCANCODE_MODE:   return sf::Keyboard::Scan::Menu;

        // TODO
        case SDL_SCANCODE_SLEEP:
        case SDL_SCANCODE_WAKE:
        case SDL_SCANCODE_CHANNEL_INCREMENT:
        case SDL_SCANCODE_CHANNEL_DECREMENT:
        case SDL_SCANCODE_MEDIA_RECORD:
        case SDL_SCANCODE_MEDIA_FAST_FORWARD:
        case SDL_SCANCODE_MEDIA_REWIND:
        case SDL_SCANCODE_MEDIA_EJECT:
        case SDL_SCANCODE_MEDIA_PLAY_PAUSE:
        case SDL_SCANCODE_MEDIA_SELECT:
        case SDL_SCANCODE_AC_NEW:
        case SDL_SCANCODE_AC_OPEN:
        case SDL_SCANCODE_AC_CLOSE:
        case SDL_SCANCODE_AC_EXIT:
        case SDL_SCANCODE_AC_SAVE:
        case SDL_SCANCODE_AC_PRINT:
        case SDL_SCANCODE_AC_PROPERTIES:
        case SDL_SCANCODE_AC_SEARCH:
        case SDL_SCANCODE_AC_HOME:
        case SDL_SCANCODE_AC_BACK:
        case SDL_SCANCODE_AC_FORWARD:
        case SDL_SCANCODE_AC_STOP:
        case SDL_SCANCODE_AC_REFRESH:
        case SDL_SCANCODE_AC_BOOKMARKS:
        case SDL_SCANCODE_SOFTLEFT:
        case SDL_SCANCODE_SOFTRIGHT:
        case SDL_SCANCODE_CALL:
        case SDL_SCANCODE_ENDCALL:
        case SDL_SCANCODE_RESERVED:
        case SDL_SCANCODE_COUNT:
        case SDL_SCANCODE_NONUSHASH:
        case SDL_SCANCODE_POWER:
        case SDL_SCANCODE_KP_EQUALS:
        case SDL_SCANCODE_EXECUTE:
        case SDL_SCANCODE_STOP:
        case SDL_SCANCODE_FIND:
        case SDL_SCANCODE_KP_COMMA:
        case SDL_SCANCODE_KP_EQUALSAS400:
        case SDL_SCANCODE_INTERNATIONAL1:
        case SDL_SCANCODE_INTERNATIONAL2:
        case SDL_SCANCODE_INTERNATIONAL3:
        case SDL_SCANCODE_INTERNATIONAL4:
        case SDL_SCANCODE_INTERNATIONAL5:
        case SDL_SCANCODE_INTERNATIONAL6:
        case SDL_SCANCODE_INTERNATIONAL7:
        case SDL_SCANCODE_INTERNATIONAL8:
        case SDL_SCANCODE_INTERNATIONAL9:
        case SDL_SCANCODE_LANG1:
        case SDL_SCANCODE_LANG2:
        case SDL_SCANCODE_LANG3:
        case SDL_SCANCODE_LANG4:
        case SDL_SCANCODE_LANG5:
        case SDL_SCANCODE_LANG6:
        case SDL_SCANCODE_LANG7:
        case SDL_SCANCODE_LANG8:
        case SDL_SCANCODE_LANG9:
        case SDL_SCANCODE_ALTERASE:
        case SDL_SCANCODE_SYSREQ:
        case SDL_SCANCODE_CANCEL:
        case SDL_SCANCODE_CLEAR:
        case SDL_SCANCODE_PRIOR:
        case SDL_SCANCODE_RETURN2:
        case SDL_SCANCODE_SEPARATOR:
        case SDL_SCANCODE_OUT:
        case SDL_SCANCODE_OPER:
        case SDL_SCANCODE_CLEARAGAIN:
        case SDL_SCANCODE_CRSEL:
        case SDL_SCANCODE_EXSEL:
        case SDL_SCANCODE_KP_00:
        case SDL_SCANCODE_KP_000:
        case SDL_SCANCODE_THOUSANDSSEPARATOR:
        case SDL_SCANCODE_DECIMALSEPARATOR:
        case SDL_SCANCODE_CURRENCYUNIT:
        case SDL_SCANCODE_CURRENCYSUBUNIT:
        case SDL_SCANCODE_KP_LEFTPAREN:
        case SDL_SCANCODE_KP_RIGHTPAREN:
        case SDL_SCANCODE_KP_LEFTBRACE:
        case SDL_SCANCODE_KP_RIGHTBRACE:
        case SDL_SCANCODE_KP_TAB:
        case SDL_SCANCODE_KP_BACKSPACE:
        case SDL_SCANCODE_KP_A:
        case SDL_SCANCODE_KP_B:
        case SDL_SCANCODE_KP_C:
        case SDL_SCANCODE_KP_D:
        case SDL_SCANCODE_KP_E:
        case SDL_SCANCODE_KP_F:
        case SDL_SCANCODE_KP_XOR:
        case SDL_SCANCODE_KP_POWER:
        case SDL_SCANCODE_KP_PERCENT:
        case SDL_SCANCODE_KP_LESS:
        case SDL_SCANCODE_KP_GREATER:
        case SDL_SCANCODE_KP_AMPERSAND:
        case SDL_SCANCODE_KP_DBLAMPERSAND:
        case SDL_SCANCODE_KP_VERTICALBAR:
        case SDL_SCANCODE_KP_DBLVERTICALBAR:
        case SDL_SCANCODE_KP_COLON:
        case SDL_SCANCODE_KP_HASH:
        case SDL_SCANCODE_KP_SPACE:
        case SDL_SCANCODE_KP_AT:
        case SDL_SCANCODE_KP_EXCLAM:
        case SDL_SCANCODE_KP_MEMSTORE:
        case SDL_SCANCODE_KP_MEMRECALL:
        case SDL_SCANCODE_KP_MEMCLEAR:
        case SDL_SCANCODE_KP_MEMADD:
        case SDL_SCANCODE_KP_MEMSUBTRACT:
        case SDL_SCANCODE_KP_MEMMULTIPLY:
        case SDL_SCANCODE_KP_MEMDIVIDE:
        case SDL_SCANCODE_KP_PLUSMINUS:
        case SDL_SCANCODE_KP_CLEAR:
        case SDL_SCANCODE_KP_CLEARENTRY:
        case SDL_SCANCODE_KP_BINARY:
        case SDL_SCANCODE_KP_OCTAL:
        case SDL_SCANCODE_KP_DECIMAL:
        case SDL_SCANCODE_KP_HEXADECIMAL:
        case SDL_SCANCODE_UNKNOWN:
            return sf::Keyboard::Scan::Unknown;
    }
    // clang-format on

    return sf::Keyboard::Scan::Unknown;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr sf::Keyboard::Key mapSDLKeycodeToSFML(const SDL_Keycode sdlKey) noexcept
{
    // clang-format off
    switch (sdlKey)
    {
        // Letters (SDL keycodes for letters are their lowercase ASCII values)
        case SDLK_A: return sf::Keyboard::Key::A;
        case SDLK_B: return sf::Keyboard::Key::B;
        case SDLK_C: return sf::Keyboard::Key::C;
        case SDLK_D: return sf::Keyboard::Key::D;
        case SDLK_E: return sf::Keyboard::Key::E;
        case SDLK_F: return sf::Keyboard::Key::F;
        case SDLK_G: return sf::Keyboard::Key::G;
        case SDLK_H: return sf::Keyboard::Key::H;
        case SDLK_I: return sf::Keyboard::Key::I;
        case SDLK_J: return sf::Keyboard::Key::J;
        case SDLK_K: return sf::Keyboard::Key::K;
        case SDLK_L: return sf::Keyboard::Key::L;
        case SDLK_M: return sf::Keyboard::Key::M;
        case SDLK_N: return sf::Keyboard::Key::N;
        case SDLK_O: return sf::Keyboard::Key::O;
        case SDLK_P: return sf::Keyboard::Key::P;
        case SDLK_Q: return sf::Keyboard::Key::Q;
        case SDLK_R: return sf::Keyboard::Key::R;
        case SDLK_S: return sf::Keyboard::Key::S;
        case SDLK_T: return sf::Keyboard::Key::T;
        case SDLK_U: return sf::Keyboard::Key::U;
        case SDLK_V: return sf::Keyboard::Key::V;
        case SDLK_W: return sf::Keyboard::Key::W;
        case SDLK_X: return sf::Keyboard::Key::X;
        case SDLK_Y: return sf::Keyboard::Key::Y;
        case SDLK_Z: return sf::Keyboard::Key::Z;

        // Numbers (top row)
        case SDLK_0: return sf::Keyboard::Key::Num0;
        case SDLK_1: return sf::Keyboard::Key::Num1;
        case SDLK_2: return sf::Keyboard::Key::Num2;
        case SDLK_3: return sf::Keyboard::Key::Num3;
        case SDLK_4: return sf::Keyboard::Key::Num4;
        case SDLK_5: return sf::Keyboard::Key::Num5;
        case SDLK_6: return sf::Keyboard::Key::Num6;
        case SDLK_7: return sf::Keyboard::Key::Num7;
        case SDLK_8: return sf::Keyboard::Key::Num8;
        case SDLK_9: return sf::Keyboard::Key::Num9;

        // Control and punctuation keys
        case SDLK_ESCAPE:       return sf::Keyboard::Key::Escape;
        case SDLK_LCTRL:        return sf::Keyboard::Key::LControl;
        case SDLK_LSHIFT:       return sf::Keyboard::Key::LShift;
        case SDLK_LALT:         return sf::Keyboard::Key::LAlt;
        case SDLK_LGUI:         return sf::Keyboard::Key::LSystem;
        case SDLK_RCTRL:        return sf::Keyboard::Key::RControl;
        case SDLK_RSHIFT:       return sf::Keyboard::Key::RShift;
        case SDLK_RALT:         return sf::Keyboard::Key::RAlt;
        case SDLK_RGUI:         return sf::Keyboard::Key::RSystem;
        case SDLK_MENU:         return sf::Keyboard::Key::Menu;
        case SDLK_LEFTBRACKET:  return sf::Keyboard::Key::LBracket;
        case SDLK_RIGHTBRACKET: return sf::Keyboard::Key::RBracket;
        case SDLK_SEMICOLON:    return sf::Keyboard::Key::Semicolon;
        case SDLK_COMMA:        return sf::Keyboard::Key::Comma;
        case SDLK_PERIOD:       return sf::Keyboard::Key::Period;
        case SDLK_APOSTROPHE:   return sf::Keyboard::Key::Apostrophe;
        case SDLK_SLASH:        return sf::Keyboard::Key::Slash;
        case SDLK_BACKSLASH:    return sf::Keyboard::Key::Backslash;
        case SDLK_GRAVE:        return sf::Keyboard::Key::Grave;
        case SDLK_EQUALS:       return sf::Keyboard::Key::Equal;
        case SDLK_MINUS:        return sf::Keyboard::Key::Hyphen;
        case SDLK_SPACE:        return sf::Keyboard::Key::Space;
        case SDLK_RETURN:       return sf::Keyboard::Key::Enter;
        case SDLK_BACKSPACE:    return sf::Keyboard::Key::Backspace;
        case SDLK_TAB:          return sf::Keyboard::Key::Tab;

        // Navigation keys
        case SDLK_PAGEUP:   return sf::Keyboard::Key::PageUp;
        case SDLK_PAGEDOWN: return sf::Keyboard::Key::PageDown;
        case SDLK_END:      return sf::Keyboard::Key::End;
        case SDLK_HOME:     return sf::Keyboard::Key::Home;
        case SDLK_INSERT:   return sf::Keyboard::Key::Insert;
        case SDLK_DELETE:   return sf::Keyboard::Key::Delete;
        case SDLK_LEFT:     return sf::Keyboard::Key::Left;
        case SDLK_RIGHT:    return sf::Keyboard::Key::Right;
        case SDLK_UP:       return sf::Keyboard::Key::Up;
        case SDLK_DOWN:     return sf::Keyboard::Key::Down;

        // Keypad (numpad) keys
        case SDLK_KP_1: return sf::Keyboard::Key::Numpad1;
        case SDLK_KP_2: return sf::Keyboard::Key::Numpad2;
        case SDLK_KP_3: return sf::Keyboard::Key::Numpad3;
        case SDLK_KP_4: return sf::Keyboard::Key::Numpad4;
        case SDLK_KP_5: return sf::Keyboard::Key::Numpad5;
        case SDLK_KP_6: return sf::Keyboard::Key::Numpad6;
        case SDLK_KP_7: return sf::Keyboard::Key::Numpad7;
        case SDLK_KP_8: return sf::Keyboard::Key::Numpad8;
        case SDLK_KP_9: return sf::Keyboard::Key::Numpad9;
        case SDLK_KP_0: return sf::Keyboard::Key::Numpad0;

        // Arithmetic keypad keys
        case SDLK_KP_PLUS:     return sf::Keyboard::Key::Add;
        case SDLK_KP_MINUS:    return sf::Keyboard::Key::Subtract;
        case SDLK_KP_MULTIPLY: return sf::Keyboard::Key::Multiply;
        case SDLK_KP_DIVIDE:   return sf::Keyboard::Key::Divide;

        // Function keys
        case SDLK_F1:  return sf::Keyboard::Key::F1;
        case SDLK_F2:  return sf::Keyboard::Key::F2;
        case SDLK_F3:  return sf::Keyboard::Key::F3;
        case SDLK_F4:  return sf::Keyboard::Key::F4;
        case SDLK_F5:  return sf::Keyboard::Key::F5;
        case SDLK_F6:  return sf::Keyboard::Key::F6;
        case SDLK_F7:  return sf::Keyboard::Key::F7;
        case SDLK_F8:  return sf::Keyboard::Key::F8;
        case SDLK_F9:  return sf::Keyboard::Key::F9;
        case SDLK_F10: return sf::Keyboard::Key::F10;
        case SDLK_F11: return sf::Keyboard::Key::F11;
        case SDLK_F12: return sf::Keyboard::Key::F12;
        case SDLK_F13: return sf::Keyboard::Key::F13;
        case SDLK_F14: return sf::Keyboard::Key::F14;
        case SDLK_F15: return sf::Keyboard::Key::F15;

        // Other keys
        case SDLK_PAUSE: return sf::Keyboard::Key::Pause;
    }
    // clang-format on

    return sf::Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
[[nodiscard]] SDL_PropertiesID makeSDLWindowPropertiesFromHandle(const sf::WindowHandle handle)
{
    const SDL_PropertiesID props = SDL_CreateProperties();

#if defined(SFML_SYSTEM_WINDOWS)
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, handle);
#elif defined(SFML_SYSTEM_LINUX_OR_BSD)
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER, handle);
#elif defined(SFML_SYSTEM_MACOS)
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, handle);
#elif defined(SFML_SYSTEM_IOS)
    // TODO P0: doesn't seem to be implemented in SDL
#elif defined(SFML_SYSTEM_ANDROID)
    // TODO P0: doesn't seem to be implemented in SDL
#elif defined(SFML_SYSTEM_EMSCRIPTEN)
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_EMSCRIPTEN_CANVAS_ID, handle);
#endif

    return props;
}


////////////////////////////////////////////////////////////
[[nodiscard]] constexpr SDL_WindowFlags makeSDLWindowFlagsFromWindowSettings(const sf::WindowSettings& windowSettings)
{
    SDL_WindowFlags flags{};

    if (windowSettings.fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;

    if (windowSettings.resizable)
        flags |= SDL_WINDOW_RESIZABLE;

    if (!windowSettings.hasTitlebar)
        flags |= SDL_WINDOW_BORDERLESS;

    return flags;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr sf::Mouse::Button getButtonFromSDLButton(const sf::base::U8 sdlButton)
{
    switch (sdlButton)
    {
        case SDL_BUTTON_LEFT:
            return sf::Mouse::Button::Left;
        case SDL_BUTTON_MIDDLE:
            return sf::Mouse::Button::Middle;
        case SDL_BUTTON_RIGHT:
            return sf::Mouse::Button::Right;
        case SDL_BUTTON_X1:
            return sf::Mouse::Button::Extra1;
        case SDL_BUTTON_X2:
            return sf::Mouse::Button::Extra2;
    }

    SFML_BASE_ASSERT(false);
    return sf::Mouse::Button::Left;
}


////////////////////////////////////////////////////////////
struct TouchInfo
{
    unsigned int     normalizedIndex;
    sf::Vector2i     position;
    sf::WindowHandle handle;
};


////////////////////////////////////////////////////////////
bool touchIndexPool[32]{}; // Keeps track of which finger indices are in use


////////////////////////////////////////////////////////////
[[nodiscard]] int findFirstNormalizedTouchIndex()
{
    for (sf::base::SizeT i = 0u; i < 32u; ++i)
        if (!touchIndexPool[i])
            return static_cast<int>(i);

    sf::priv::err() << "No available touch index\n";
    return -1;
}


////////////////////////////////////////////////////////////
std::unordered_map<SDL_FingerID, TouchInfo> touchMap;

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
WindowImplSDL::WindowImplSDL(const char* context, SDL_Window* const sdlWindow) : m_sdlWindow(sdlWindow)
{
    if (!m_sdlWindow)
    {
        err() << "Failed to create window created from " << context << ": " << SDL_GetError();
        return;
    }

    if (!SDL_StartTextInput(m_sdlWindow)) // TODO P1: might not want to call this on mobiles
        err() << "Failed to start text input for window created from " << context << ": " << SDL_GetError();
}


////////////////////////////////////////////////////////////
WindowImplSDL::WindowImplSDL(WindowHandle handle) :
WindowImplSDL{"handle", SDL_CreateWindowWithProperties(makeSDLWindowPropertiesFromHandle(handle))}
{
}


////////////////////////////////////////////////////////////
WindowImplSDL::WindowImplSDL(const WindowSettings& windowSettings) :
WindowImplSDL{"window settings",
              SDL_CreateWindow(windowSettings.title.toAnsiString<std::string>().data(),
                               static_cast<int>(windowSettings.size.x),
                               static_cast<int>(windowSettings.size.y),
                               makeSDLWindowFlagsFromWindowSettings(windowSettings))}
{
}


////////////////////////////////////////////////////////////
WindowImplSDL::~WindowImplSDL()
{
    if (!SDL_StopTextInput(m_sdlWindow))
        err() << "Failed to stop text input for window: " << SDL_GetError();

    SDL_DestroyWindow(m_sdlWindow);
}


////////////////////////////////////////////////////////////
WindowHandle WindowImplSDL::getNativeHandle() const
{
    const auto props = SDL_GetWindowProperties(m_sdlWindow);

    return static_cast<WindowHandle>(
#if defined(SFML_SYSTEM_WINDOWS)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)
#elif defined(SFML_SYSTEM_LINUX_OR_BSD)
        SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)
#elif defined(SFML_SYSTEM_MACOS)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr)
#elif defined(SFML_SYSTEM_IOS)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr)
#elif defined(SFML_SYSTEM_ANDROID)
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr)
#elif defined(SFML_SYSTEM_EMSCRIPTEN)
    // TODO P0: doesn't seem to be implemented in SDL
#endif
    );
}


////////////////////////////////////////////////////////////
void WindowImplSDL::processEvents()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        const auto et = static_cast<SDL_EventType>(e.type);

        switch (et)
        {
            case SDL_EVENT_QUIT:
            {
                pushEvent(Event::Closed{});
                break;
            }

            case SDL_EVENT_WINDOW_RESIZED:
            {
                pushEvent(Event::Resized{
                    Vector2u{static_cast<unsigned int>(e.window.data1), static_cast<unsigned int>(e.window.data2)}});
                break;
            }

            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            {
                pushEvent(Event::MouseEntered{});
                break;
            }

            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            {
                pushEvent(Event::MouseLeft{});
                break;
            }

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            {
                pushEvent(Event::FocusGained{});
                break;
            }

            case SDL_EVENT_WINDOW_FOCUS_LOST:
            {
                pushEvent(Event::FocusLost{});
                break;
            }

            case SDL_EVENT_KEY_DOWN:
            {
                pushEvent(Event::KeyPressed{.code     = mapSDLKeycodeToSFML(e.key.key),
                                            .scancode = mapSDLScancodeToSFML(e.key.scancode),
                                            .alt      = static_cast<bool>(e.key.mod & SDL_KMOD_ALT),
                                            .control  = static_cast<bool>(e.key.mod & SDL_KMOD_CTRL),
                                            .shift    = static_cast<bool>(e.key.mod & SDL_KMOD_SHIFT),
                                            .system   = static_cast<bool>(e.key.mod & SDL_KMOD_GUI)});
                break;
            }

            case SDL_EVENT_KEY_UP:
            {
                pushEvent(Event::KeyReleased{.code     = mapSDLKeycodeToSFML(e.key.key),
                                             .scancode = mapSDLScancodeToSFML(e.key.scancode),
                                             .alt      = static_cast<bool>(e.key.mod & SDL_KMOD_ALT),
                                             .control  = static_cast<bool>(e.key.mod & SDL_KMOD_CTRL),
                                             .shift    = static_cast<bool>(e.key.mod & SDL_KMOD_SHIFT),
                                             .system   = static_cast<bool>(e.key.mod & SDL_KMOD_GUI)});
                break;
            }
            case SDL_EVENT_TEXT_INPUT:
            {
                char32_t     unicode   = 0;
                const char*  keyBuffer = e.text.text;
                const size_t length    = SFML_BASE_STRLEN(keyBuffer);
                const auto*  iter      = keyBuffer;
                while (iter < keyBuffer + length)
                {
                    iter = Utf8::decode(iter, keyBuffer + length, unicode, 0);
                    if (unicode != 0)
                        pushEvent(Event::TextEntered{unicode});
                }

                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
            {
                pushEvent(Event::MouseMoved{
                    .position = {static_cast<int>(e.motion.x), static_cast<int>(e.motion.y)},
                });
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                pushEvent(Event::MouseButtonPressed{
                    .button   = getButtonFromSDLButton(e.button.button),
                    .position = {static_cast<int>(e.button.x), static_cast<int>(e.button.y)},
                });
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                pushEvent(Event::MouseButtonReleased{
                    .button   = getButtonFromSDLButton(e.button.button),
                    .position = {static_cast<int>(e.button.x), static_cast<int>(e.button.y)},
                });
                break;
            }

            case SDL_EVENT_MOUSE_WHEEL:
            {
                pushEvent(Event::MouseWheelScrolled{
                    .wheel    = Mouse::Wheel::Vertical, // TODO P0: horizontal wheel support?
                    .delta    = static_cast<float>(e.wheel.y),
                    .position = {static_cast<int>(e.wheel.x), static_cast<int>(e.wheel.y)},
                });
                break;
            }

            case SDL_EVENT_FINGER_DOWN:
            {
                const SDL_TouchFingerEvent& fingerEvent = e.tfinger;
                const Vector2i touchPos = {static_cast<int>(fingerEvent.x * static_cast<float>(getSize().x)),
                                           static_cast<int>(fingerEvent.y * static_cast<float>(getSize().y))};

                SFML_BASE_ASSERT(!touchMap.contains(fingerEvent.fingerID));

                const int normalizedIndex = findFirstNormalizedTouchIndex();
                if (normalizedIndex == -1)
                    break;

                const auto fingerIdx      = static_cast<unsigned int>(normalizedIndex);
                touchIndexPool[fingerIdx] = true;
                touchMap.emplace(fingerEvent.fingerID, TouchInfo{fingerIdx, touchPos, getNativeHandle()});

                pushEvent(sf::Event::TouchBegan{fingerIdx, touchPos});
                break;
            }

            case SDL_EVENT_FINGER_UP:
            {
                const SDL_TouchFingerEvent& fingerEvent = e.tfinger;
                const Vector2i touchPos = {static_cast<int>(fingerEvent.x * static_cast<float>(getSize().x)),
                                           static_cast<int>(fingerEvent.y * static_cast<float>(getSize().y))};

                SFML_BASE_ASSERT(touchMap.contains(fingerEvent.fingerID));

                const auto [fingerIdx, pos, handle] = touchMap[fingerEvent.fingerID];

                touchIndexPool[fingerIdx] = false;
                touchMap.erase(fingerEvent.fingerID);

                pushEvent(sf::Event::TouchEnded{fingerIdx, touchPos});
                break;
            }

            case SDL_EVENT_FINGER_MOTION:
            {
                const SDL_TouchFingerEvent& fingerEvent = e.tfinger;
                const Vector2i touchPos = {static_cast<int>(fingerEvent.x * static_cast<float>(getSize().x)),
                                           static_cast<int>(fingerEvent.y * static_cast<float>(getSize().y))};

                SFML_BASE_ASSERT(touchMap.contains(fingerEvent.fingerID));
                const auto [fingerIdx, pos, handle] = touchMap[fingerEvent.fingerID];

                pushEvent(sf::Event::TouchMoved{fingerIdx, touchPos});
                break;
            }

            case SDL_EVENT_FINGER_CANCELED: // TODO
            {
                break;
            }

                // unused
            case SDL_EVENT_FIRST:
            case SDL_EVENT_TERMINATING:
            case SDL_EVENT_LOW_MEMORY:
            case SDL_EVENT_WILL_ENTER_BACKGROUND:
            case SDL_EVENT_DID_ENTER_BACKGROUND:
            case SDL_EVENT_WILL_ENTER_FOREGROUND:
            case SDL_EVENT_DID_ENTER_FOREGROUND:
            case SDL_EVENT_LOCALE_CHANGED:
            case SDL_EVENT_SYSTEM_THEME_CHANGED:
            case SDL_EVENT_DISPLAY_ORIENTATION:
            case SDL_EVENT_DISPLAY_ADDED:
            case SDL_EVENT_DISPLAY_REMOVED:
            case SDL_EVENT_DISPLAY_MOVED:
            case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
            case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
            case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
            case SDL_EVENT_WINDOW_SHOWN:
            case SDL_EVENT_WINDOW_HIDDEN:
            case SDL_EVENT_WINDOW_EXPOSED:
            case SDL_EVENT_WINDOW_MOVED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_WINDOW_HIT_TEST:
            case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
            case SDL_EVENT_WINDOW_OCCLUDED:
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_WINDOW_DESTROYED:
            case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
            case SDL_EVENT_TEXT_EDITING:
            case SDL_EVENT_KEYMAP_CHANGED:
            case SDL_EVENT_KEYBOARD_ADDED:
            case SDL_EVENT_KEYBOARD_REMOVED:
            case SDL_EVENT_TEXT_EDITING_CANDIDATES:
            case SDL_EVENT_MOUSE_ADDED:
            case SDL_EVENT_MOUSE_REMOVED:
            case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            case SDL_EVENT_JOYSTICK_BALL_MOTION:
            case SDL_EVENT_JOYSTICK_HAT_MOTION:
            case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            case SDL_EVENT_JOYSTICK_BUTTON_UP:
            case SDL_EVENT_JOYSTICK_ADDED:
            case SDL_EVENT_JOYSTICK_REMOVED:
            case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
            case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
            case SDL_EVENT_GAMEPAD_REMAPPED:
            case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
            case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
            case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
            case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
            case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
            case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
            case SDL_EVENT_CLIPBOARD_UPDATE:
            case SDL_EVENT_DROP_FILE:
            case SDL_EVENT_DROP_TEXT:
            case SDL_EVENT_DROP_BEGIN:
            case SDL_EVENT_DROP_COMPLETE:
            case SDL_EVENT_DROP_POSITION:
            case SDL_EVENT_AUDIO_DEVICE_ADDED:
            case SDL_EVENT_AUDIO_DEVICE_REMOVED:
            case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
            case SDL_EVENT_SENSOR_UPDATE:
            case SDL_EVENT_PEN_PROXIMITY_IN:
            case SDL_EVENT_PEN_PROXIMITY_OUT:
            case SDL_EVENT_PEN_DOWN:
            case SDL_EVENT_PEN_UP:
            case SDL_EVENT_PEN_BUTTON_DOWN:
            case SDL_EVENT_PEN_BUTTON_UP:
            case SDL_EVENT_PEN_MOTION:
            case SDL_EVENT_PEN_AXIS:
            case SDL_EVENT_CAMERA_DEVICE_ADDED:
            case SDL_EVENT_CAMERA_DEVICE_REMOVED:
            case SDL_EVENT_CAMERA_DEVICE_APPROVED:
            case SDL_EVENT_CAMERA_DEVICE_DENIED:
            case SDL_EVENT_RENDER_TARGETS_RESET:
            case SDL_EVENT_RENDER_DEVICE_RESET:
            case SDL_EVENT_RENDER_DEVICE_LOST:
            case SDL_EVENT_PRIVATE0:
            case SDL_EVENT_PRIVATE1:
            case SDL_EVENT_PRIVATE2:
            case SDL_EVENT_PRIVATE3:
            case SDL_EVENT_POLL_SENTINEL:
            case SDL_EVENT_USER:
            case SDL_EVENT_LAST:
            case SDL_EVENT_ENUM_PADDING:
                break;
        }
    }
}


////////////////////////////////////////////////////////////
Vector2i WindowImplSDL::getPosition() const
{
    int x{};
    int y{};

    if (!SDL_GetWindowPosition(m_sdlWindow, &x, &y))
        err() << "Failed to get window position: " << SDL_GetError();

    return {x, y};
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setPosition(Vector2i position)
{
    SDL_SetWindowPosition(m_sdlWindow, position.x, position.y);
}


////////////////////////////////////////////////////////////
Vector2u WindowImplSDL::getSize() const
{
    int w{};
    int h{};

    if (!SDL_GetWindowSize(m_sdlWindow, &w, &h))
        err() << "Failed to get window size: " << SDL_GetError();

    return {static_cast<unsigned int>(w), static_cast<unsigned int>(h)};
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setSize(Vector2u size)
{
    if (!SDL_SetWindowSize(m_sdlWindow, static_cast<int>(size.x), static_cast<int>(size.y)))
        err() << "Failed to set window size: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setTitle(const String& title)
{
    if (!SDL_SetWindowTitle(m_sdlWindow, title.toAnsiString<std::string>().data()))
        err() << "Failed to set window title: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setIcon(Vector2u size, const base::U8* pixels)
{
    SDL_Surface* iconSurface = SDL_CreateSurfaceFrom(static_cast<int>(size.x),
                                                     static_cast<int>(size.y),
                                                     SDL_PIXELFORMAT_RGBA32,
                                                     const_cast<void*>(static_cast<const void*>(pixels)),
                                                     static_cast<int>(size.x * 4));

    if (iconSurface == nullptr)
    {
        err() << "Failed to create icon surface: " << SDL_GetError();
        return;
    }

    if (!SDL_SetWindowIcon(m_sdlWindow, iconSurface))
        err() << "Failed to set window icon: " << SDL_GetError();

    SDL_DestroySurface(iconSurface);
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setVisible(const bool visible)
{
    if (visible)
    {
        if (!SDL_ShowWindow(m_sdlWindow))
            err() << "Failed to show window: " << SDL_GetError();
    }
    else
    {
        if (!SDL_HideWindow(m_sdlWindow))
            err() << "Failed to hide window: " << SDL_GetError();
    }
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setMouseCursorVisible(const bool visible)
{
    if (visible)
    {
        if (!SDL_ShowCursor())
            err() << "Failed to show cursor: " << SDL_GetError();
    }
    else
    {
        if (!SDL_HideCursor())
            err() << "Failed to hide cursor: " << SDL_GetError();
    }
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setMouseCursorGrabbed(const bool grabbed)
{
    if (!SDL_SetWindowMouseGrab(m_sdlWindow, grabbed))
        err() << "Failed to set window mouse grab: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setMouseCursor(const CursorImpl& cursor)
{
    // TODO P0: implement
    err() << "Setting custom mouse cursor is not implemented";
}


////////////////////////////////////////////////////////////
void WindowImplSDL::setKeyRepeatEnabled(bool enabled)
{
    // TODO P0: implement
    err() << "Setting key repeat is not implemented";
}


////////////////////////////////////////////////////////////
void WindowImplSDL::requestFocus()
{
    if (!SDL_RaiseWindow(m_sdlWindow))
        err() << "Failed to raise window: " << SDL_GetError();
}


////////////////////////////////////////////////////////////
bool WindowImplSDL::hasFocus() const
{
    return SDL_GetWindowFlags(m_sdlWindow) & SDL_WINDOW_INPUT_FOCUS;
}


////////////////////////////////////////////////////////////
float WindowImplSDL::getDPIAwareScalingFactor() const
{
    const float displayScale = SDL_GetWindowDisplayScale(m_sdlWindow);

    if (displayScale == 0.f)
    {
        err() << "Failed to get window display scale: " << SDL_GetError();
        return 1.f;
    }

    return displayScale;
}

} // namespace sf::priv
