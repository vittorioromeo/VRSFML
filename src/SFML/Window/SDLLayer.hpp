#pragma once
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_touch.h>
#include <SDL3/SDL_video.h>


namespace sf::priv
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] inline constexpr Keyboard::Scan mapSDLScancodeToSFML(const SDL_Scancode sdlCode) noexcept
{
    // clang-format off
    switch (sdlCode)
    {
        // Letters (SDL: A=4, B=5, ... Z=29)
        case SDL_SCANCODE_A: return Keyboard::Scan::A;
        case SDL_SCANCODE_B: return Keyboard::Scan::B;
        case SDL_SCANCODE_C: return Keyboard::Scan::C;
        case SDL_SCANCODE_D: return Keyboard::Scan::D;
        case SDL_SCANCODE_E: return Keyboard::Scan::E;
        case SDL_SCANCODE_F: return Keyboard::Scan::F;
        case SDL_SCANCODE_G: return Keyboard::Scan::G;
        case SDL_SCANCODE_H: return Keyboard::Scan::H;
        case SDL_SCANCODE_I: return Keyboard::Scan::I;
        case SDL_SCANCODE_J: return Keyboard::Scan::J;
        case SDL_SCANCODE_K: return Keyboard::Scan::K;
        case SDL_SCANCODE_L: return Keyboard::Scan::L;
        case SDL_SCANCODE_M: return Keyboard::Scan::M;
        case SDL_SCANCODE_N: return Keyboard::Scan::N;
        case SDL_SCANCODE_O: return Keyboard::Scan::O;
        case SDL_SCANCODE_P: return Keyboard::Scan::P;
        case SDL_SCANCODE_Q: return Keyboard::Scan::Q;
        case SDL_SCANCODE_R: return Keyboard::Scan::R;
        case SDL_SCANCODE_S: return Keyboard::Scan::S;
        case SDL_SCANCODE_T: return Keyboard::Scan::T;
        case SDL_SCANCODE_U: return Keyboard::Scan::U;
        case SDL_SCANCODE_V: return Keyboard::Scan::V;
        case SDL_SCANCODE_W: return Keyboard::Scan::W;
        case SDL_SCANCODE_X: return Keyboard::Scan::X;
        case SDL_SCANCODE_Y: return Keyboard::Scan::Y;
        case SDL_SCANCODE_Z: return Keyboard::Scan::Z;

        // Numbers
        case SDL_SCANCODE_1: return Keyboard::Scan::Num1;
        case SDL_SCANCODE_2: return Keyboard::Scan::Num2;
        case SDL_SCANCODE_3: return Keyboard::Scan::Num3;
        case SDL_SCANCODE_4: return Keyboard::Scan::Num4;
        case SDL_SCANCODE_5: return Keyboard::Scan::Num5;
        case SDL_SCANCODE_6: return Keyboard::Scan::Num6;
        case SDL_SCANCODE_7: return Keyboard::Scan::Num7;
        case SDL_SCANCODE_8: return Keyboard::Scan::Num8;
        case SDL_SCANCODE_9: return Keyboard::Scan::Num9;
        case SDL_SCANCODE_0: return Keyboard::Scan::Num0;

        // Control and special keys
        case SDL_SCANCODE_RETURN:       return Keyboard::Scan::Enter;
        case SDL_SCANCODE_ESCAPE:       return Keyboard::Scan::Escape;
        case SDL_SCANCODE_BACKSPACE:    return Keyboard::Scan::Backspace;
        case SDL_SCANCODE_TAB:          return Keyboard::Scan::Tab;
        case SDL_SCANCODE_SPACE:        return Keyboard::Scan::Space;
        case SDL_SCANCODE_MINUS:        return Keyboard::Scan::Hyphen;
        case SDL_SCANCODE_EQUALS:       return Keyboard::Scan::Equal;
        case SDL_SCANCODE_LEFTBRACKET:  return Keyboard::Scan::LBracket;
        case SDL_SCANCODE_RIGHTBRACKET: return Keyboard::Scan::RBracket;
        case SDL_SCANCODE_BACKSLASH:    return Keyboard::Scan::Backslash;
        case SDL_SCANCODE_SEMICOLON:    return Keyboard::Scan::Semicolon;
        case SDL_SCANCODE_APOSTROPHE:   return Keyboard::Scan::Apostrophe;
        case SDL_SCANCODE_GRAVE:        return Keyboard::Scan::Grave;
        case SDL_SCANCODE_COMMA:        return Keyboard::Scan::Comma;
        case SDL_SCANCODE_PERIOD:       return Keyboard::Scan::Period;
        case SDL_SCANCODE_SLASH:        return Keyboard::Scan::Slash;

        // Function keys
        case SDL_SCANCODE_F1:  return Keyboard::Scan::F1;
        case SDL_SCANCODE_F2:  return Keyboard::Scan::F2;
        case SDL_SCANCODE_F3:  return Keyboard::Scan::F3;
        case SDL_SCANCODE_F4:  return Keyboard::Scan::F4;
        case SDL_SCANCODE_F5:  return Keyboard::Scan::F5;
        case SDL_SCANCODE_F6:  return Keyboard::Scan::F6;
        case SDL_SCANCODE_F7:  return Keyboard::Scan::F7;
        case SDL_SCANCODE_F8:  return Keyboard::Scan::F8;
        case SDL_SCANCODE_F9:  return Keyboard::Scan::F9;
        case SDL_SCANCODE_F10: return Keyboard::Scan::F10;
        case SDL_SCANCODE_F11: return Keyboard::Scan::F11;
        case SDL_SCANCODE_F12: return Keyboard::Scan::F12;
        case SDL_SCANCODE_F13: return Keyboard::Scan::F13;
        case SDL_SCANCODE_F14: return Keyboard::Scan::F14;
        case SDL_SCANCODE_F15: return Keyboard::Scan::F15;
        case SDL_SCANCODE_F16: return Keyboard::Scan::F16;
        case SDL_SCANCODE_F17: return Keyboard::Scan::F17;
        case SDL_SCANCODE_F18: return Keyboard::Scan::F18;
        case SDL_SCANCODE_F19: return Keyboard::Scan::F19;
        case SDL_SCANCODE_F20: return Keyboard::Scan::F20;
        case SDL_SCANCODE_F21: return Keyboard::Scan::F21;
        case SDL_SCANCODE_F22: return Keyboard::Scan::F22;
        case SDL_SCANCODE_F23: return Keyboard::Scan::F23;
        case SDL_SCANCODE_F24: return Keyboard::Scan::F24;

        // Lock and navigation keys
        case SDL_SCANCODE_CAPSLOCK:     return Keyboard::Scan::CapsLock;
        case SDL_SCANCODE_PRINTSCREEN:  return Keyboard::Scan::PrintScreen;
        case SDL_SCANCODE_SCROLLLOCK:   return Keyboard::Scan::ScrollLock;
        case SDL_SCANCODE_PAUSE:        return Keyboard::Scan::Pause;
        case SDL_SCANCODE_INSERT:       return Keyboard::Scan::Insert;
        case SDL_SCANCODE_HOME:         return Keyboard::Scan::Home;
        case SDL_SCANCODE_PAGEUP:       return Keyboard::Scan::PageUp;
        case SDL_SCANCODE_DELETE:       return Keyboard::Scan::Delete;
        case SDL_SCANCODE_END:          return Keyboard::Scan::End;
        case SDL_SCANCODE_PAGEDOWN:     return Keyboard::Scan::PageDown;
        case SDL_SCANCODE_RIGHT:        return Keyboard::Scan::Right;
        case SDL_SCANCODE_LEFT:         return Keyboard::Scan::Left;
        case SDL_SCANCODE_DOWN:         return Keyboard::Scan::Down;
        case SDL_SCANCODE_UP:           return Keyboard::Scan::Up;
        case SDL_SCANCODE_NUMLOCKCLEAR: return Keyboard::Scan::NumLock;

        // Keypad
        case SDL_SCANCODE_KP_DIVIDE:   return Keyboard::Scan::NumpadDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return Keyboard::Scan::NumpadMultiply;
        case SDL_SCANCODE_KP_MINUS:    return Keyboard::Scan::NumpadMinus;
        case SDL_SCANCODE_KP_PLUS:     return Keyboard::Scan::NumpadPlus;
        case SDL_SCANCODE_KP_ENTER:    return Keyboard::Scan::NumpadEnter;
        case SDL_SCANCODE_KP_PERIOD:   return Keyboard::Scan::NumpadDecimal;
        case SDL_SCANCODE_KP_1:        return Keyboard::Scan::Numpad1;
        case SDL_SCANCODE_KP_2:        return Keyboard::Scan::Numpad2;
        case SDL_SCANCODE_KP_3:        return Keyboard::Scan::Numpad3;
        case SDL_SCANCODE_KP_4:        return Keyboard::Scan::Numpad4;
        case SDL_SCANCODE_KP_5:        return Keyboard::Scan::Numpad5;
        case SDL_SCANCODE_KP_6:        return Keyboard::Scan::Numpad6;
        case SDL_SCANCODE_KP_7:        return Keyboard::Scan::Numpad7;
        case SDL_SCANCODE_KP_8:        return Keyboard::Scan::Numpad8;
        case SDL_SCANCODE_KP_9:        return Keyboard::Scan::Numpad9;
        case SDL_SCANCODE_KP_0:        return Keyboard::Scan::Numpad0;

        // Additional keys
        case SDL_SCANCODE_NONUSBACKSLASH: return Keyboard::Scan::NonUsBackslash;
        case SDL_SCANCODE_APPLICATION:    return Keyboard::Scan::Application;
        case SDL_SCANCODE_HELP:           return Keyboard::Scan::Help;
        case SDL_SCANCODE_MENU:           return Keyboard::Scan::Menu;
        case SDL_SCANCODE_SELECT:         return Keyboard::Scan::Select;
        case SDL_SCANCODE_AGAIN:          return Keyboard::Scan::Redo;
        case SDL_SCANCODE_UNDO:           return Keyboard::Scan::Undo;
        case SDL_SCANCODE_CUT:            return Keyboard::Scan::Cut;
        case SDL_SCANCODE_COPY:           return Keyboard::Scan::Copy;
        case SDL_SCANCODE_PASTE:          return Keyboard::Scan::Paste;
        case SDL_SCANCODE_MUTE:           return Keyboard::Scan::VolumeMute;
        case SDL_SCANCODE_VOLUMEUP:       return Keyboard::Scan::VolumeUp;
        case SDL_SCANCODE_VOLUMEDOWN:     return Keyboard::Scan::VolumeDown;

        // Media keys
        case SDL_SCANCODE_MEDIA_PLAY:           return Keyboard::Scan::MediaPlayPause;
        case SDL_SCANCODE_MEDIA_PAUSE:          return Keyboard::Scan::MediaPlayPause;
        case SDL_SCANCODE_MEDIA_STOP:           return Keyboard::Scan::MediaStop;
        case SDL_SCANCODE_MEDIA_NEXT_TRACK:     return Keyboard::Scan::MediaNextTrack;
        case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK: return Keyboard::Scan::MediaPreviousTrack;

        case SDL_SCANCODE_LCTRL:  return Keyboard::Scan::LControl;
        case SDL_SCANCODE_LSHIFT: return Keyboard::Scan::LShift;
        case SDL_SCANCODE_LALT:   return Keyboard::Scan::LAlt;
        case SDL_SCANCODE_LGUI:   return Keyboard::Scan::LSystem;
        case SDL_SCANCODE_RCTRL:  return Keyboard::Scan::RControl;
        case SDL_SCANCODE_RSHIFT: return Keyboard::Scan::RShift;
        case SDL_SCANCODE_RALT:   return Keyboard::Scan::RAlt;
        case SDL_SCANCODE_RGUI:   return Keyboard::Scan::RSystem;
        case SDL_SCANCODE_MODE:   return Keyboard::Scan::ModeChange;

        case SDL_SCANCODE_KP_EQUALS:    return Keyboard::Scan::NumpadEqual;
        case SDL_SCANCODE_AC_BACK:      return Keyboard::Scan::Back;
        case SDL_SCANCODE_AC_FORWARD:   return Keyboard::Scan::Forward;
        case SDL_SCANCODE_AC_REFRESH:   return Keyboard::Scan::Refresh;
        case SDL_SCANCODE_AC_STOP:      return Keyboard::Scan::Stop;
        case SDL_SCANCODE_AC_SEARCH:    return Keyboard::Scan::Search;
        case SDL_SCANCODE_AC_BOOKMARKS: return Keyboard::Scan::Favorites;
        case SDL_SCANCODE_AC_HOME:      return Keyboard::Scan::HomePage;

        case SDL_SCANCODE_EXECUTE: return Keyboard::Scan::Execute;

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
        case SDL_SCANCODE_SOFTLEFT:
        case SDL_SCANCODE_SOFTRIGHT:
        case SDL_SCANCODE_CALL:
        case SDL_SCANCODE_ENDCALL:
        case SDL_SCANCODE_RESERVED:
        case SDL_SCANCODE_COUNT:
        case SDL_SCANCODE_NONUSHASH:
        case SDL_SCANCODE_POWER:
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
            return Keyboard::Scan::Unknown;
    }
    // clang-format on

    return Keyboard::Scan::Unknown;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] inline constexpr SDL_Scancode mapSFMLScancodeToSDL(const Keyboard::Scan scanCode) noexcept
{
    // clang-format off
    switch (scanCode)
    {
        // Letters (SDL: A=4, B=5, ... Z=29)
        case Keyboard::Scan::A: return SDL_SCANCODE_A;
        case Keyboard::Scan::B: return SDL_SCANCODE_B;
        case Keyboard::Scan::C: return SDL_SCANCODE_C;
        case Keyboard::Scan::D: return SDL_SCANCODE_D;
        case Keyboard::Scan::E: return SDL_SCANCODE_E;
        case Keyboard::Scan::F: return SDL_SCANCODE_F;
        case Keyboard::Scan::G: return SDL_SCANCODE_G;
        case Keyboard::Scan::H: return SDL_SCANCODE_H;
        case Keyboard::Scan::I: return SDL_SCANCODE_I;
        case Keyboard::Scan::J: return SDL_SCANCODE_J;
        case Keyboard::Scan::K: return SDL_SCANCODE_K;
        case Keyboard::Scan::L: return SDL_SCANCODE_L;
        case Keyboard::Scan::M: return SDL_SCANCODE_M;
        case Keyboard::Scan::N: return SDL_SCANCODE_N;
        case Keyboard::Scan::O: return SDL_SCANCODE_O;
        case Keyboard::Scan::P: return SDL_SCANCODE_P;
        case Keyboard::Scan::Q: return SDL_SCANCODE_Q;
        case Keyboard::Scan::R: return SDL_SCANCODE_R;
        case Keyboard::Scan::S: return SDL_SCANCODE_S;
        case Keyboard::Scan::T: return SDL_SCANCODE_T;
        case Keyboard::Scan::U: return SDL_SCANCODE_U;
        case Keyboard::Scan::V: return SDL_SCANCODE_V;
        case Keyboard::Scan::W: return SDL_SCANCODE_W;
        case Keyboard::Scan::X: return SDL_SCANCODE_X;
        case Keyboard::Scan::Y: return SDL_SCANCODE_Y;
        case Keyboard::Scan::Z: return SDL_SCANCODE_Z;

        // Numbers
        case Keyboard::Scan::Num1: return SDL_SCANCODE_1;
        case Keyboard::Scan::Num2: return SDL_SCANCODE_2;
        case Keyboard::Scan::Num3: return SDL_SCANCODE_3;
        case Keyboard::Scan::Num4: return SDL_SCANCODE_4;
        case Keyboard::Scan::Num5: return SDL_SCANCODE_5;
        case Keyboard::Scan::Num6: return SDL_SCANCODE_6;
        case Keyboard::Scan::Num7: return SDL_SCANCODE_7;
        case Keyboard::Scan::Num8: return SDL_SCANCODE_8;
        case Keyboard::Scan::Num9: return SDL_SCANCODE_9;
        case Keyboard::Scan::Num0: return SDL_SCANCODE_0;

        // Control and special keys
        case Keyboard::Scan::Enter:      return SDL_SCANCODE_RETURN;
        case Keyboard::Scan::Escape:     return SDL_SCANCODE_ESCAPE;
        case Keyboard::Scan::Backspace:  return SDL_SCANCODE_BACKSPACE;
        case Keyboard::Scan::Tab:        return SDL_SCANCODE_TAB;
        case Keyboard::Scan::Space:      return SDL_SCANCODE_SPACE;
        case Keyboard::Scan::Hyphen:     return SDL_SCANCODE_MINUS;
        case Keyboard::Scan::Equal:      return SDL_SCANCODE_EQUALS;
        case Keyboard::Scan::LBracket:   return SDL_SCANCODE_LEFTBRACKET;
        case Keyboard::Scan::RBracket:   return SDL_SCANCODE_RIGHTBRACKET;
        case Keyboard::Scan::Backslash:  return SDL_SCANCODE_BACKSLASH;
        case Keyboard::Scan::Semicolon:  return SDL_SCANCODE_SEMICOLON;
        case Keyboard::Scan::Apostrophe: return SDL_SCANCODE_APOSTROPHE;
        case Keyboard::Scan::Grave:      return SDL_SCANCODE_GRAVE;
        case Keyboard::Scan::Comma:      return SDL_SCANCODE_COMMA;
        case Keyboard::Scan::Period:     return SDL_SCANCODE_PERIOD;
        case Keyboard::Scan::Slash:      return SDL_SCANCODE_SLASH;

        // Function keys
        case Keyboard::Scan::F1:  return SDL_SCANCODE_F1;
        case Keyboard::Scan::F2:  return SDL_SCANCODE_F2;
        case Keyboard::Scan::F3:  return SDL_SCANCODE_F3;
        case Keyboard::Scan::F4:  return SDL_SCANCODE_F4;
        case Keyboard::Scan::F5:  return SDL_SCANCODE_F5;
        case Keyboard::Scan::F6:  return SDL_SCANCODE_F6;
        case Keyboard::Scan::F7:  return SDL_SCANCODE_F7;
        case Keyboard::Scan::F8:  return SDL_SCANCODE_F8;
        case Keyboard::Scan::F9:  return SDL_SCANCODE_F9;
        case Keyboard::Scan::F10: return SDL_SCANCODE_F10;
        case Keyboard::Scan::F11: return SDL_SCANCODE_F11;
        case Keyboard::Scan::F12: return SDL_SCANCODE_F12;
        case Keyboard::Scan::F13: return SDL_SCANCODE_F13;
        case Keyboard::Scan::F14: return SDL_SCANCODE_F14;
        case Keyboard::Scan::F15: return SDL_SCANCODE_F15;
        case Keyboard::Scan::F16: return SDL_SCANCODE_F16;
        case Keyboard::Scan::F17: return SDL_SCANCODE_F17;
        case Keyboard::Scan::F18: return SDL_SCANCODE_F18;
        case Keyboard::Scan::F19: return SDL_SCANCODE_F19;
        case Keyboard::Scan::F20: return SDL_SCANCODE_F20;
        case Keyboard::Scan::F21: return SDL_SCANCODE_F21;
        case Keyboard::Scan::F22: return SDL_SCANCODE_F22;
        case Keyboard::Scan::F23: return SDL_SCANCODE_F23;
        case Keyboard::Scan::F24: return SDL_SCANCODE_F24;

        // Lock and navigation keys
        case Keyboard::Scan::CapsLock:     return SDL_SCANCODE_CAPSLOCK;
        case Keyboard::Scan::PrintScreen:  return SDL_SCANCODE_PRINTSCREEN;
        case Keyboard::Scan::ScrollLock:   return SDL_SCANCODE_SCROLLLOCK;
        case Keyboard::Scan::Pause:        return SDL_SCANCODE_PAUSE;
        case Keyboard::Scan::Insert:       return SDL_SCANCODE_INSERT;
        case Keyboard::Scan::Home:         return SDL_SCANCODE_HOME;
        case Keyboard::Scan::PageUp:       return SDL_SCANCODE_PAGEUP;
        case Keyboard::Scan::Delete:       return SDL_SCANCODE_DELETE;
        case Keyboard::Scan::End:          return SDL_SCANCODE_END;
        case Keyboard::Scan::PageDown:     return SDL_SCANCODE_PAGEDOWN;
        case Keyboard::Scan::Right:        return SDL_SCANCODE_RIGHT;
        case Keyboard::Scan::Left:         return SDL_SCANCODE_LEFT;
        case Keyboard::Scan::Down:         return SDL_SCANCODE_DOWN;
        case Keyboard::Scan::Up:           return SDL_SCANCODE_UP;
        case Keyboard::Scan::NumLock:      return SDL_SCANCODE_NUMLOCKCLEAR;

        // Keypad
        case Keyboard::Scan::NumpadDivide:   return SDL_SCANCODE_KP_DIVIDE;
        case Keyboard::Scan::NumpadMultiply: return SDL_SCANCODE_KP_MULTIPLY;
        case Keyboard::Scan::NumpadMinus:    return SDL_SCANCODE_KP_MINUS;
        case Keyboard::Scan::NumpadPlus:     return SDL_SCANCODE_KP_PLUS;
        case Keyboard::Scan::NumpadEnter:    return SDL_SCANCODE_KP_ENTER;
        case Keyboard::Scan::NumpadDecimal:  return SDL_SCANCODE_KP_PERIOD;
        case Keyboard::Scan::Numpad1:        return SDL_SCANCODE_KP_1;
        case Keyboard::Scan::Numpad2:        return SDL_SCANCODE_KP_2;
        case Keyboard::Scan::Numpad3:        return SDL_SCANCODE_KP_3;
        case Keyboard::Scan::Numpad4:        return SDL_SCANCODE_KP_4;
        case Keyboard::Scan::Numpad5:        return SDL_SCANCODE_KP_5;
        case Keyboard::Scan::Numpad6:        return SDL_SCANCODE_KP_6;
        case Keyboard::Scan::Numpad7:        return SDL_SCANCODE_KP_7;
        case Keyboard::Scan::Numpad8:        return SDL_SCANCODE_KP_8;
        case Keyboard::Scan::Numpad9:        return SDL_SCANCODE_KP_9;
        case Keyboard::Scan::Numpad0:        return SDL_SCANCODE_KP_0;

        // Additional keys
        case Keyboard::Scan::NonUsBackslash: return SDL_SCANCODE_NONUSBACKSLASH;
        case Keyboard::Scan::Application:    return SDL_SCANCODE_APPLICATION;
        case Keyboard::Scan::Help:           return SDL_SCANCODE_HELP;
        case Keyboard::Scan::Menu:           return SDL_SCANCODE_MENU;
        case Keyboard::Scan::Select:         return SDL_SCANCODE_SELECT;
        case Keyboard::Scan::Redo:           return SDL_SCANCODE_AGAIN;
        case Keyboard::Scan::Undo:           return SDL_SCANCODE_UNDO;
        case Keyboard::Scan::Cut:            return SDL_SCANCODE_CUT;
        case Keyboard::Scan::Copy:           return SDL_SCANCODE_COPY;
        case Keyboard::Scan::Paste:          return SDL_SCANCODE_PASTE;
        case Keyboard::Scan::VolumeMute:     return SDL_SCANCODE_MUTE;
        case Keyboard::Scan::VolumeUp:       return SDL_SCANCODE_VOLUMEUP;
        case Keyboard::Scan::VolumeDown:     return SDL_SCANCODE_VOLUMEDOWN;

        // Media keys
        case Keyboard::Scan::MediaPlayPause:     return SDL_SCANCODE_MEDIA_PLAY;
        case Keyboard::Scan::MediaStop:          return SDL_SCANCODE_MEDIA_STOP;
        case Keyboard::Scan::MediaNextTrack:     return SDL_SCANCODE_MEDIA_NEXT_TRACK;
        case Keyboard::Scan::MediaPreviousTrack: return SDL_SCANCODE_MEDIA_PREVIOUS_TRACK;

        case Keyboard::Scan::LControl: return SDL_SCANCODE_LCTRL;
        case Keyboard::Scan::LShift:   return SDL_SCANCODE_LSHIFT;
        case Keyboard::Scan::LAlt:     return SDL_SCANCODE_LALT;
        case Keyboard::Scan::LSystem:  return SDL_SCANCODE_LGUI;
        case Keyboard::Scan::RControl: return SDL_SCANCODE_RCTRL;
        case Keyboard::Scan::RShift:   return SDL_SCANCODE_RSHIFT;
        case Keyboard::Scan::RAlt:     return SDL_SCANCODE_RALT;
        case Keyboard::Scan::RSystem:  return SDL_SCANCODE_RGUI;

        case Keyboard::Scan::NumpadEqual: return SDL_SCANCODE_KP_EQUALS;
        case Keyboard::Scan::Back:        return SDL_SCANCODE_AC_BACK;
        case Keyboard::Scan::Forward:     return SDL_SCANCODE_AC_FORWARD;
        case Keyboard::Scan::Refresh:     return SDL_SCANCODE_AC_REFRESH;
        case Keyboard::Scan::Stop:        return SDL_SCANCODE_AC_STOP;
        case Keyboard::Scan::Search:      return SDL_SCANCODE_AC_SEARCH;
        case Keyboard::Scan::Favorites:   return SDL_SCANCODE_AC_BOOKMARKS;
        case Keyboard::Scan::HomePage:    return SDL_SCANCODE_AC_HOME;

        case Keyboard::Scan::Execute:    return SDL_SCANCODE_EXECUTE;
        case Keyboard::Scan::ModeChange: return SDL_SCANCODE_MODE;

        case Keyboard::Scan::Unknown:
        case Keyboard::Scan::LaunchApplication1:
        case Keyboard::Scan::LaunchApplication2:
        case Keyboard::Scan::LaunchMail:
        case Keyboard::Scan::LaunchMediaSelect:
            break;
    }
    // clang-format on

    return SDL_SCANCODE_UNKNOWN;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] inline constexpr Keyboard::Key mapSDLKeycodeToSFML(const SDL_Keycode sdlKey) noexcept
{
    // clang-format off
    switch (sdlKey)
    {
        // Letters (SDL keycodes for letters are their lowercase ASCII values)
        case SDLK_A: return Keyboard::Key::A;
        case SDLK_B: return Keyboard::Key::B;
        case SDLK_C: return Keyboard::Key::C;
        case SDLK_D: return Keyboard::Key::D;
        case SDLK_E: return Keyboard::Key::E;
        case SDLK_F: return Keyboard::Key::F;
        case SDLK_G: return Keyboard::Key::G;
        case SDLK_H: return Keyboard::Key::H;
        case SDLK_I: return Keyboard::Key::I;
        case SDLK_J: return Keyboard::Key::J;
        case SDLK_K: return Keyboard::Key::K;
        case SDLK_L: return Keyboard::Key::L;
        case SDLK_M: return Keyboard::Key::M;
        case SDLK_N: return Keyboard::Key::N;
        case SDLK_O: return Keyboard::Key::O;
        case SDLK_P: return Keyboard::Key::P;
        case SDLK_Q: return Keyboard::Key::Q;
        case SDLK_R: return Keyboard::Key::R;
        case SDLK_S: return Keyboard::Key::S;
        case SDLK_T: return Keyboard::Key::T;
        case SDLK_U: return Keyboard::Key::U;
        case SDLK_V: return Keyboard::Key::V;
        case SDLK_W: return Keyboard::Key::W;
        case SDLK_X: return Keyboard::Key::X;
        case SDLK_Y: return Keyboard::Key::Y;
        case SDLK_Z: return Keyboard::Key::Z;

        // Numbers (top row)
        case SDLK_0: return Keyboard::Key::Num0;
        case SDLK_1: return Keyboard::Key::Num1;
        case SDLK_2: return Keyboard::Key::Num2;
        case SDLK_3: return Keyboard::Key::Num3;
        case SDLK_4: return Keyboard::Key::Num4;
        case SDLK_5: return Keyboard::Key::Num5;
        case SDLK_6: return Keyboard::Key::Num6;
        case SDLK_7: return Keyboard::Key::Num7;
        case SDLK_8: return Keyboard::Key::Num8;
        case SDLK_9: return Keyboard::Key::Num9;

        // Control and punctuation keys
        case SDLK_ESCAPE:       return Keyboard::Key::Escape;
        case SDLK_LCTRL:        return Keyboard::Key::LControl;
        case SDLK_LSHIFT:       return Keyboard::Key::LShift;
        case SDLK_LALT:         return Keyboard::Key::LAlt;
        case SDLK_LGUI:         return Keyboard::Key::LSystem;
        case SDLK_RCTRL:        return Keyboard::Key::RControl;
        case SDLK_RSHIFT:       return Keyboard::Key::RShift;
        case SDLK_RALT:         return Keyboard::Key::RAlt;
        case SDLK_RGUI:         return Keyboard::Key::RSystem;
        case SDLK_MENU:         return Keyboard::Key::Menu;
        case SDLK_LEFTBRACKET:  return Keyboard::Key::LBracket;
        case SDLK_RIGHTBRACKET: return Keyboard::Key::RBracket;
        case SDLK_SEMICOLON:    return Keyboard::Key::Semicolon;
        case SDLK_COMMA:        return Keyboard::Key::Comma;
        case SDLK_PERIOD:       return Keyboard::Key::Period;
        case SDLK_APOSTROPHE:   return Keyboard::Key::Apostrophe;
        case SDLK_SLASH:        return Keyboard::Key::Slash;
        case SDLK_BACKSLASH:    return Keyboard::Key::Backslash;
        case SDLK_GRAVE:        return Keyboard::Key::Grave;
        case SDLK_EQUALS:       return Keyboard::Key::Equal;
        case SDLK_MINUS:        return Keyboard::Key::Hyphen;
        case SDLK_SPACE:        return Keyboard::Key::Space;
        case SDLK_RETURN:       return Keyboard::Key::Enter;
        case SDLK_BACKSPACE:    return Keyboard::Key::Backspace;
        case SDLK_TAB:          return Keyboard::Key::Tab;

        // Navigation keys
        case SDLK_PAGEUP:   return Keyboard::Key::PageUp;
        case SDLK_PAGEDOWN: return Keyboard::Key::PageDown;
        case SDLK_END:      return Keyboard::Key::End;
        case SDLK_HOME:     return Keyboard::Key::Home;
        case SDLK_INSERT:   return Keyboard::Key::Insert;
        case SDLK_DELETE:   return Keyboard::Key::Delete;
        case SDLK_LEFT:     return Keyboard::Key::Left;
        case SDLK_RIGHT:    return Keyboard::Key::Right;
        case SDLK_UP:       return Keyboard::Key::Up;
        case SDLK_DOWN:     return Keyboard::Key::Down;

        // Keypad (numpad) keys
        case SDLK_KP_1: return Keyboard::Key::Numpad1;
        case SDLK_KP_2: return Keyboard::Key::Numpad2;
        case SDLK_KP_3: return Keyboard::Key::Numpad3;
        case SDLK_KP_4: return Keyboard::Key::Numpad4;
        case SDLK_KP_5: return Keyboard::Key::Numpad5;
        case SDLK_KP_6: return Keyboard::Key::Numpad6;
        case SDLK_KP_7: return Keyboard::Key::Numpad7;
        case SDLK_KP_8: return Keyboard::Key::Numpad8;
        case SDLK_KP_9: return Keyboard::Key::Numpad9;
        case SDLK_KP_0: return Keyboard::Key::Numpad0;

        // Arithmetic keypad keys
        case SDLK_KP_PLUS:     return Keyboard::Key::Add;
        case SDLK_KP_MINUS:    return Keyboard::Key::Subtract;
        case SDLK_KP_MULTIPLY: return Keyboard::Key::Multiply;
        case SDLK_KP_DIVIDE:   return Keyboard::Key::Divide;

        // Function keys
        case SDLK_F1:  return Keyboard::Key::F1;
        case SDLK_F2:  return Keyboard::Key::F2;
        case SDLK_F3:  return Keyboard::Key::F3;
        case SDLK_F4:  return Keyboard::Key::F4;
        case SDLK_F5:  return Keyboard::Key::F5;
        case SDLK_F6:  return Keyboard::Key::F6;
        case SDLK_F7:  return Keyboard::Key::F7;
        case SDLK_F8:  return Keyboard::Key::F8;
        case SDLK_F9:  return Keyboard::Key::F9;
        case SDLK_F10: return Keyboard::Key::F10;
        case SDLK_F11: return Keyboard::Key::F11;
        case SDLK_F12: return Keyboard::Key::F12;
        case SDLK_F13: return Keyboard::Key::F13;
        case SDLK_F14: return Keyboard::Key::F14;
        case SDLK_F15: return Keyboard::Key::F15;

        // Other keys
        case SDLK_PAUSE: return Keyboard::Key::Pause;
    }
    // clang-format on

    return Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] inline constexpr SDL_Keycode mapSFMLKeycodeToSDL(const Keyboard::Key key) noexcept
{
    // clang-format off
    switch (key)
    {
        // Letters (SDL keycodes for letters are their lowercase ASCII values)
        case Keyboard::Key::A: return SDLK_A;
        case Keyboard::Key::B: return SDLK_B;
        case Keyboard::Key::C: return SDLK_C;
        case Keyboard::Key::D: return SDLK_D;
        case Keyboard::Key::E: return SDLK_E;
        case Keyboard::Key::F: return SDLK_F;
        case Keyboard::Key::G: return SDLK_G;
        case Keyboard::Key::H: return SDLK_H;
        case Keyboard::Key::I: return SDLK_I;
        case Keyboard::Key::J: return SDLK_J;
        case Keyboard::Key::K: return SDLK_K;
        case Keyboard::Key::L: return SDLK_L;
        case Keyboard::Key::M: return SDLK_M;
        case Keyboard::Key::N: return SDLK_N;
        case Keyboard::Key::O: return SDLK_O;
        case Keyboard::Key::P: return SDLK_P;
        case Keyboard::Key::Q: return SDLK_Q;
        case Keyboard::Key::R: return SDLK_R;
        case Keyboard::Key::S: return SDLK_S;
        case Keyboard::Key::T: return SDLK_T;
        case Keyboard::Key::U: return SDLK_U;
        case Keyboard::Key::V: return SDLK_V;
        case Keyboard::Key::W: return SDLK_W;
        case Keyboard::Key::X: return SDLK_X;
        case Keyboard::Key::Y: return SDLK_Y;
        case Keyboard::Key::Z: return SDLK_Z;

        // Numbers (top row)
        case Keyboard::Key::Num0: return SDLK_0;
        case Keyboard::Key::Num1: return SDLK_1;
        case Keyboard::Key::Num2: return SDLK_2;
        case Keyboard::Key::Num3: return SDLK_3;
        case Keyboard::Key::Num4: return SDLK_4;
        case Keyboard::Key::Num5: return SDLK_5;
        case Keyboard::Key::Num6: return SDLK_6;
        case Keyboard::Key::Num7: return SDLK_7;
        case Keyboard::Key::Num8: return SDLK_8;
        case Keyboard::Key::Num9: return SDLK_9;

        // Control and punctuation keys
        case Keyboard::Key::Escape:     return SDLK_ESCAPE;
        case Keyboard::Key::LControl:   return SDLK_LCTRL;
        case Keyboard::Key::LShift:     return SDLK_LSHIFT;
        case Keyboard::Key::LAlt:       return SDLK_LALT;
        case Keyboard::Key::LSystem:    return SDLK_LGUI;
        case Keyboard::Key::RControl:   return SDLK_RCTRL;
        case Keyboard::Key::RShift:     return SDLK_RSHIFT;
        case Keyboard::Key::RAlt:       return SDLK_RALT;
        case Keyboard::Key::RSystem:    return SDLK_RGUI;
        case Keyboard::Key::Menu:       return SDLK_MENU;
        case Keyboard::Key::LBracket:   return SDLK_LEFTBRACKET;
        case Keyboard::Key::RBracket:   return SDLK_RIGHTBRACKET;
        case Keyboard::Key::Semicolon:  return SDLK_SEMICOLON;
        case Keyboard::Key::Comma:      return SDLK_COMMA;
        case Keyboard::Key::Period:     return SDLK_PERIOD;
        case Keyboard::Key::Apostrophe: return SDLK_APOSTROPHE;
        case Keyboard::Key::Slash:      return SDLK_SLASH;
        case Keyboard::Key::Backslash:  return SDLK_BACKSLASH;
        case Keyboard::Key::Grave:      return SDLK_GRAVE;
        case Keyboard::Key::Equal:      return SDLK_EQUALS;
        case Keyboard::Key::Hyphen:     return SDLK_MINUS;
        case Keyboard::Key::Space:      return SDLK_SPACE;
        case Keyboard::Key::Enter:      return SDLK_RETURN;
        case Keyboard::Key::Backspace:  return SDLK_BACKSPACE;
        case Keyboard::Key::Tab:        return SDLK_TAB;

        // Navigation keys
        case Keyboard::Key::PageUp:   return SDLK_PAGEUP;
        case Keyboard::Key::PageDown: return SDLK_PAGEDOWN;
        case Keyboard::Key::End:      return SDLK_END;
        case Keyboard::Key::Home:     return SDLK_HOME;
        case Keyboard::Key::Insert:   return SDLK_INSERT;
        case Keyboard::Key::Delete:   return SDLK_DELETE;
        case Keyboard::Key::Left:     return SDLK_LEFT;
        case Keyboard::Key::Right:    return SDLK_RIGHT;
        case Keyboard::Key::Up:       return SDLK_UP;
        case Keyboard::Key::Down:     return SDLK_DOWN;

        // Keypad (numpad) keys
        case Keyboard::Key::Numpad1: return SDLK_KP_1;
        case Keyboard::Key::Numpad2: return SDLK_KP_2;
        case Keyboard::Key::Numpad3: return SDLK_KP_3;
        case Keyboard::Key::Numpad4: return SDLK_KP_4;
        case Keyboard::Key::Numpad5: return SDLK_KP_5;
        case Keyboard::Key::Numpad6: return SDLK_KP_6;
        case Keyboard::Key::Numpad7: return SDLK_KP_7;
        case Keyboard::Key::Numpad8: return SDLK_KP_8;
        case Keyboard::Key::Numpad9: return SDLK_KP_9;
        case Keyboard::Key::Numpad0: return SDLK_KP_0;

        // Arithmetic keypad keys
        case Keyboard::Key::Add:      return SDLK_KP_PLUS;
        case Keyboard::Key::Subtract: return SDLK_KP_MINUS;
        case Keyboard::Key::Multiply: return SDLK_KP_MULTIPLY;
        case Keyboard::Key::Divide:   return SDLK_KP_DIVIDE;

        // Function keys
        case Keyboard::Key::F1:  return SDLK_F1;
        case Keyboard::Key::F2:  return SDLK_F2;
        case Keyboard::Key::F3:  return SDLK_F3;
        case Keyboard::Key::F4:  return SDLK_F4;
        case Keyboard::Key::F5:  return SDLK_F5;
        case Keyboard::Key::F6:  return SDLK_F6;
        case Keyboard::Key::F7:  return SDLK_F7;
        case Keyboard::Key::F8:  return SDLK_F8;
        case Keyboard::Key::F9:  return SDLK_F9;
        case Keyboard::Key::F10: return SDLK_F10;
        case Keyboard::Key::F11: return SDLK_F11;
        case Keyboard::Key::F12: return SDLK_F12;
        case Keyboard::Key::F13: return SDLK_F13;
        case Keyboard::Key::F14: return SDLK_F14;
        case Keyboard::Key::F15: return SDLK_F15;

        // Other keys
        case Keyboard::Key::Pause: return SDLK_PAUSE;

        case Keyboard::Key::Unknown:
            break;
    }
    // clang-format on

    return SDLK_UNKNOWN;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline SDL_PropertiesID makeSDLWindowPropertiesFromHandle(const WindowHandle handle)
{
    const SDL_PropertiesID props = SDL_CreateProperties();

#if defined(SFML_SYSTEM_WINDOWS)
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, handle);
#elif defined(SFML_SYSTEM_LINUX_OR_BSD)
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER, static_cast<Sint64>(handle));
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
[[nodiscard, gnu::pure]] inline constexpr SDL_WindowFlags makeSDLWindowFlagsFromWindowSettings(
    const WindowSettings& windowSettings) noexcept
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
[[nodiscard, gnu::const]] inline constexpr Mouse::Button getButtonFromSDLButton(const base::U8 sdlButton) noexcept
{
    // clang-format off
    switch (sdlButton)
    {
        case SDL_BUTTON_LEFT:   return Mouse::Button::Left;
        case SDL_BUTTON_MIDDLE: return Mouse::Button::Middle;
        case SDL_BUTTON_RIGHT:  return Mouse::Button::Right;
        case SDL_BUTTON_X1:     return Mouse::Button::Extra1;
        case SDL_BUTTON_X2:     return Mouse::Button::Extra2;
    }
    // clang-format on

    SFML_BASE_ASSERT(false);
    return Mouse::Button::Left;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] inline constexpr base::U8 getSDLButtonFromSFMLButton(const Mouse::Button button) noexcept
{
    // clang-format off
    switch (button)
    {
        case Mouse::Button::Left:   return SDL_BUTTON_LEFT;
        case Mouse::Button::Middle: return SDL_BUTTON_MIDDLE;
        case Mouse::Button::Right:  return SDL_BUTTON_RIGHT;
        case Mouse::Button::Extra1: return SDL_BUTTON_X1;
        case Mouse::Button::Extra2: return SDL_BUTTON_X2;
    }
    // clang-format on

    SFML_BASE_ASSERT(false);
    return SDL_BUTTON_LEFT;
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline Keyboard::Key localizeViaSDL(const Keyboard::Scancode code) noexcept
{
    const SDL_Scancode sdlScancode = mapSFMLScancodeToSDL(code);
    const SDL_Keycode  sdlKey      = SDL_GetKeyFromScancode(sdlScancode, SDL_GetModState(), true);

    return mapSDLKeycodeToSFML(sdlKey);
}


////////////////////////////////////////////////////////////
[[nodiscard]] inline Keyboard::Scancode delocalizeViaSDL(const Keyboard::Key key) noexcept
{
    const SDL_Keycode sdlKey = mapSFMLKeycodeToSDL(key);

    SDL_Keymod         mod{};
    const SDL_Scancode sdlScancode = SDL_GetScancodeFromKey(sdlKey, &mod);

    return static_cast<Keyboard::Scancode>(sdlScancode);
}


////////////////////////////////////////////////////////////
struct SFML_BASE_TRIVIAL_ABI UniquePtrSDLDeleter
{
    template <typename T>
    [[gnu::always_inline]] void operator()(T* const ptr) const noexcept
    {
        static_assert(!SFML_BASE_IS_SAME(T, void), "can't delete pointer to incomplete type");

        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        static_assert(sizeof(T) > 0u, "can't delete pointer to incomplete type");

        SDL_free(static_cast<void*>(ptr));
    }
};


////////////////////////////////////////////////////////////
template <typename T>
using SDLUPtr = base::UniquePtr<T, UniquePtrSDLDeleter>;


////////////////////////////////////////////////////////////
template <typename T>
struct SDLAllocatedArray
{
    ////////////////////////////////////////////////////////////
    SDLUPtr<T>  ptr;
    base::SizeT count;


    ////////////////////////////////////////////////////////////
    explicit SDLAllocatedArray(SDLUPtr<T>&& thePtr, const base::SizeT theCount) :
    ptr{SFML_BASE_MOVE(thePtr)},
    count{theCount}
    {
    }


    ////////////////////////////////////////////////////////////
    SDLAllocatedArray(decltype(nullptr)) : ptr{nullptr}, count{0u}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] T* get() noexcept
    {
        return ptr.get();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T* get() const noexcept
    {
        return ptr.get();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] T& operator[](const base::SizeT index) noexcept
    {
        SFML_BASE_ASSERT(ptr != nullptr);
        SFML_BASE_ASSERT(index < count);

        return ptr.get()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T& operator[](const base::SizeT index) const noexcept
    {
        SFML_BASE_ASSERT(ptr != nullptr);
        SFML_BASE_ASSERT(index < count);

        return ptr.get()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool valid() const noexcept
    {
        return ptr != nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT size() const noexcept
    {
        return count;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] T* begin() noexcept
    {
        return ptr.get();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] T* end() noexcept
    {
        return ptr.get() + count;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T* begin() const noexcept
    {
        return ptr.get();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T* end() const noexcept
    {
        return ptr.get() + count;
    }
};


////////////////////////////////////////////////////////////
class SDLLayer
{
public:
    ////////////////////////////////////////////////////////////
    explicit SDLLayer()
    {
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            err() << "`SDL_Init` failed: " << SDL_GetError();
            base::abort();
        }
    }


    ////////////////////////////////////////////////////////////
    ~SDLLayer()
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_DisplayID> getDisplays()
    {
        int            displayCount = 0;
        SDL_DisplayID* displays     = SDL_GetDisplays(&displayCount);

        if (displays == nullptr)
        {
            err() << "`SDL_GetDisplays` failed: " << SDL_GetError();
            return nullptr;
        }

        return SDLAllocatedArray<SDL_DisplayID>{SDLUPtr<SDL_DisplayID>{displays}, static_cast<base::SizeT>(displayCount)};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_DisplayMode*> getFullscreenDisplayModesForDisplay(const SDL_DisplayID displayId)
    {
        int               modeCount = 0;
        SDL_DisplayMode** modes     = SDL_GetFullscreenDisplayModes(displayId, &modeCount);

        if (modes == nullptr)
        {
            err() << "`SDL_GetFullscreenDisplayModes` failed for display " << displayId << ": " << SDL_GetError();
            return nullptr;
        }

        return SDLAllocatedArray<SDL_DisplayMode*>{SDLUPtr<SDL_DisplayMode*>(modes), static_cast<base::SizeT>(modeCount)};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_PixelFormatDetails* getPixelFormatDetails(const SDL_PixelFormat format)
    {
        const auto* result = SDL_GetPixelFormatDetails(format);

        if (result == nullptr)
        {
            err() << "`SDL_GetPixelFormatDetails` failed for format " << static_cast<int>(format) << ": "
                  << SDL_GetError();
            return nullptr;
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_DisplayMode* getDesktopDisplayMode(const SDL_DisplayID displayId)
    {
        const auto* result = SDL_GetDesktopDisplayMode(displayId);

        if (result == nullptr)
        {
            err() << "`SDL_GetDesktopDisplayMode` failed for display " << displayId << ": " << SDL_GetError();
            return nullptr;
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_TouchID> getTouchDevices()
    {
        int          idCount = 0;
        SDL_TouchID* ids     = SDL_GetTouchDevices(&idCount);

        if (ids == nullptr)
        {
            err() << "`getTouchDevices` failed: " << SDL_GetError();
            return nullptr;
        }

        return SDLAllocatedArray<SDL_TouchID>{SDLUPtr<SDL_TouchID>(ids), static_cast<base::SizeT>(idCount)};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_Finger*> getTouchFingers(const SDL_TouchID touchDeviceId)
    {
        if (touchDeviceId == 0)
        {
            err() << "`getTouchFingers` failed: invalid touch device ID";
            return nullptr;
        }

        int          fingerCount = 0;
        SDL_Finger** fingers     = SDL_GetTouchFingers(touchDeviceId, &fingerCount);

        if (fingers == nullptr)
        {
            err() << "`SDL_GetTouchFingers` failed for touch device " << touchDeviceId << ": " << SDL_GetError();
            return nullptr;
        }

        return SDLAllocatedArray<SDL_Finger*>{SDLUPtr<SDL_Finger*>(fingers), static_cast<base::SizeT>(fingerCount)};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDL_TouchDeviceType getTouchDeviceType(const SDL_TouchID touchDeviceId)
    {
        if (touchDeviceId == 0)
        {
            err() << "`SDL_GetTouchDeviceType` failed: invalid touch device ID";
            return SDL_TOUCH_DEVICE_INVALID;
        }

        return SDL_GetTouchDeviceType(touchDeviceId);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getTouchDeviceName(const SDL_TouchID touchDeviceId)
    {
        if (touchDeviceId == 0)
        {
            err() << "`SDL_GetTouchDeviceType` failed: invalid touch device ID";
            return "INVALID TOUCH DEVICE";
        }

        return SDL_GetTouchDeviceName(touchDeviceId);
    }
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline SDLLayer& getSDLLayerSingleton()
{
    static SDLLayer sdlLayer;
    return sdlLayer;
}

} // namespace sf::priv
