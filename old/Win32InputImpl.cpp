#include <Zancle/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/InputImpl.hpp"
#include "Zancle/Window/WindowBase.hpp"
#include "Zancle/Window/WindowHandle.hpp"

#include "Zancle/System/String.hpp"
#include "Zancle/System/Win32/WindowsHeader.hpp"

#include "ZancleBase/EnumArray.hpp"
#include "ZancleBase/Optional.hpp"


namespace
{
zb::EnumArray<za::Keyboard::Key, za::Keyboard::Scancode, za::Keyboard::KeyCount> keyToScancodeMapping; ///< Mapping from Key to Scancode
zb::EnumArray<za::Keyboard::Scancode, za::Keyboard::Key, za::Keyboard::ScancodeCount> scancodeToKeyMapping; ///< Mapping from Scancode to Key

[[nodiscard]] za::Keyboard::Key virtualKeyToSfKey(UINT virtualKey)
{
    // clang-format off
    switch (virtualKey)
    {
        case 'A':           return za::Keyboard::Key::A;
        case 'B':           return za::Keyboard::Key::B;
        case 'C':           return za::Keyboard::Key::C;
        case 'D':           return za::Keyboard::Key::D;
        case 'E':           return za::Keyboard::Key::E;
        case 'F':           return za::Keyboard::Key::F;
        case 'G':           return za::Keyboard::Key::G;
        case 'H':           return za::Keyboard::Key::H;
        case 'I':           return za::Keyboard::Key::I;
        case 'J':           return za::Keyboard::Key::J;
        case 'K':           return za::Keyboard::Key::K;
        case 'L':           return za::Keyboard::Key::L;
        case 'M':           return za::Keyboard::Key::M;
        case 'N':           return za::Keyboard::Key::N;
        case 'O':           return za::Keyboard::Key::O;
        case 'P':           return za::Keyboard::Key::P;
        case 'Q':           return za::Keyboard::Key::Q;
        case 'R':           return za::Keyboard::Key::R;
        case 'S':           return za::Keyboard::Key::S;
        case 'T':           return za::Keyboard::Key::T;
        case 'U':           return za::Keyboard::Key::U;
        case 'V':           return za::Keyboard::Key::V;
        case 'W':           return za::Keyboard::Key::W;
        case 'X':           return za::Keyboard::Key::X;
        case 'Y':           return za::Keyboard::Key::Y;
        case 'Z':           return za::Keyboard::Key::Z;
        case '0':           return za::Keyboard::Key::Num0;
        case '1':           return za::Keyboard::Key::Num1;
        case '2':           return za::Keyboard::Key::Num2;
        case '3':           return za::Keyboard::Key::Num3;
        case '4':           return za::Keyboard::Key::Num4;
        case '5':           return za::Keyboard::Key::Num5;
        case '6':           return za::Keyboard::Key::Num6;
        case '7':           return za::Keyboard::Key::Num7;
        case '8':           return za::Keyboard::Key::Num8;
        case '9':           return za::Keyboard::Key::Num9;
        case VK_ESCAPE:     return za::Keyboard::Key::Escape;
        case VK_LCONTROL:   return za::Keyboard::Key::LControl;
        case VK_LSHIFT:     return za::Keyboard::Key::LShift;
        case VK_LMENU:      return za::Keyboard::Key::LAlt;
        case VK_LWIN:       return za::Keyboard::Key::LSystem;
        case VK_RCONTROL:   return za::Keyboard::Key::RControl;
        case VK_RSHIFT:     return za::Keyboard::Key::RShift;
        case VK_RMENU:      return za::Keyboard::Key::RAlt;
        case VK_RWIN:       return za::Keyboard::Key::RSystem;
        case VK_APPS:       return za::Keyboard::Key::Menu;
        case VK_OEM_4:      return za::Keyboard::Key::LBracket;
        case VK_OEM_6:      return za::Keyboard::Key::RBracket;
        case VK_OEM_1:      return za::Keyboard::Key::Semicolon;
        case VK_OEM_COMMA:  return za::Keyboard::Key::Comma;
        case VK_OEM_PERIOD: return za::Keyboard::Key::Period;
        case VK_OEM_7:      return za::Keyboard::Key::Apostrophe;
        case VK_OEM_2:      return za::Keyboard::Key::Slash;
        case VK_OEM_5:      return za::Keyboard::Key::Backslash;
        case VK_OEM_3:      return za::Keyboard::Key::Grave;
        case VK_OEM_PLUS:   return za::Keyboard::Key::Equal;
        case VK_OEM_MINUS:  return za::Keyboard::Key::Hyphen;
        case VK_SPACE:      return za::Keyboard::Key::Space;
        case VK_RETURN:     return za::Keyboard::Key::Enter;
        case VK_BACK:       return za::Keyboard::Key::Backspace;
        case VK_TAB:        return za::Keyboard::Key::Tab;
        case VK_PRIOR:      return za::Keyboard::Key::PageUp;
        case VK_NEXT:       return za::Keyboard::Key::PageDown;
        case VK_END:        return za::Keyboard::Key::End;
        case VK_HOME:       return za::Keyboard::Key::Home;
        case VK_INSERT:     return za::Keyboard::Key::Insert;
        case VK_DELETE:     return za::Keyboard::Key::Delete;
        case VK_ADD:        return za::Keyboard::Key::Add;
        case VK_SUBTRACT:   return za::Keyboard::Key::Subtract;
        case VK_MULTIPLY:   return za::Keyboard::Key::Multiply;
        case VK_DIVIDE:     return za::Keyboard::Key::Divide;
        case VK_LEFT:       return za::Keyboard::Key::Left;
        case VK_RIGHT:      return za::Keyboard::Key::Right;
        case VK_UP:         return za::Keyboard::Key::Up;
        case VK_DOWN:       return za::Keyboard::Key::Down;
        case VK_NUMPAD0:    return za::Keyboard::Key::Numpad0;
        case VK_NUMPAD1:    return za::Keyboard::Key::Numpad1;
        case VK_NUMPAD2:    return za::Keyboard::Key::Numpad2;
        case VK_NUMPAD3:    return za::Keyboard::Key::Numpad3;
        case VK_NUMPAD4:    return za::Keyboard::Key::Numpad4;
        case VK_NUMPAD5:    return za::Keyboard::Key::Numpad5;
        case VK_NUMPAD6:    return za::Keyboard::Key::Numpad6;
        case VK_NUMPAD7:    return za::Keyboard::Key::Numpad7;
        case VK_NUMPAD8:    return za::Keyboard::Key::Numpad8;
        case VK_NUMPAD9:    return za::Keyboard::Key::Numpad9;
        case VK_F1:         return za::Keyboard::Key::F1;
        case VK_F2:         return za::Keyboard::Key::F2;
        case VK_F3:         return za::Keyboard::Key::F3;
        case VK_F4:         return za::Keyboard::Key::F4;
        case VK_F5:         return za::Keyboard::Key::F5;
        case VK_F6:         return za::Keyboard::Key::F6;
        case VK_F7:         return za::Keyboard::Key::F7;
        case VK_F8:         return za::Keyboard::Key::F8;
        case VK_F9:         return za::Keyboard::Key::F9;
        case VK_F10:        return za::Keyboard::Key::F10;
        case VK_F11:        return za::Keyboard::Key::F11;
        case VK_F12:        return za::Keyboard::Key::F12;
        case VK_F13:        return za::Keyboard::Key::F13;
        case VK_F14:        return za::Keyboard::Key::F14;
        case VK_F15:        return za::Keyboard::Key::F15;
        case VK_PAUSE:      return za::Keyboard::Key::Pause;
        default:            return za::Keyboard::Key::Unknown;
    }
    // clang-format on
}

[[nodiscard]] int sfKeyToVirtualKey(za::Keyboard::Key key)
{
    // clang-format off
    switch (key)
    {
        case za::Keyboard::Key::A:          return 'A';
        case za::Keyboard::Key::B:          return 'B';
        case za::Keyboard::Key::C:          return 'C';
        case za::Keyboard::Key::D:          return 'D';
        case za::Keyboard::Key::E:          return 'E';
        case za::Keyboard::Key::F:          return 'F';
        case za::Keyboard::Key::G:          return 'G';
        case za::Keyboard::Key::H:          return 'H';
        case za::Keyboard::Key::I:          return 'I';
        case za::Keyboard::Key::J:          return 'J';
        case za::Keyboard::Key::K:          return 'K';
        case za::Keyboard::Key::L:          return 'L';
        case za::Keyboard::Key::M:          return 'M';
        case za::Keyboard::Key::N:          return 'N';
        case za::Keyboard::Key::O:          return 'O';
        case za::Keyboard::Key::P:          return 'P';
        case za::Keyboard::Key::Q:          return 'Q';
        case za::Keyboard::Key::R:          return 'R';
        case za::Keyboard::Key::S:          return 'S';
        case za::Keyboard::Key::T:          return 'T';
        case za::Keyboard::Key::U:          return 'U';
        case za::Keyboard::Key::V:          return 'V';
        case za::Keyboard::Key::W:          return 'W';
        case za::Keyboard::Key::X:          return 'X';
        case za::Keyboard::Key::Y:          return 'Y';
        case za::Keyboard::Key::Z:          return 'Z';
        case za::Keyboard::Key::Num0:       return '0';
        case za::Keyboard::Key::Num1:       return '1';
        case za::Keyboard::Key::Num2:       return '2';
        case za::Keyboard::Key::Num3:       return '3';
        case za::Keyboard::Key::Num4:       return '4';
        case za::Keyboard::Key::Num5:       return '5';
        case za::Keyboard::Key::Num6:       return '6';
        case za::Keyboard::Key::Num7:       return '7';
        case za::Keyboard::Key::Num8:       return '8';
        case za::Keyboard::Key::Num9:       return '9';
        case za::Keyboard::Key::Escape:     return VK_ESCAPE;
        case za::Keyboard::Key::LControl:   return VK_LCONTROL;
        case za::Keyboard::Key::LShift:     return VK_LSHIFT;
        case za::Keyboard::Key::LAlt:       return VK_LMENU;
        case za::Keyboard::Key::LSystem:    return VK_LWIN;
        case za::Keyboard::Key::RControl:   return VK_RCONTROL;
        case za::Keyboard::Key::RShift:     return VK_RSHIFT;
        case za::Keyboard::Key::RAlt:       return VK_RMENU;
        case za::Keyboard::Key::RSystem:    return VK_RWIN;
        case za::Keyboard::Key::Menu:       return VK_APPS;
        case za::Keyboard::Key::LBracket:   return VK_OEM_4;
        case za::Keyboard::Key::RBracket:   return VK_OEM_6;
        case za::Keyboard::Key::Semicolon:  return VK_OEM_1;
        case za::Keyboard::Key::Comma:      return VK_OEM_COMMA;
        case za::Keyboard::Key::Period:     return VK_OEM_PERIOD;
        case za::Keyboard::Key::Apostrophe: return VK_OEM_7;
        case za::Keyboard::Key::Slash:      return VK_OEM_2;
        case za::Keyboard::Key::Backslash:  return VK_OEM_5;
        case za::Keyboard::Key::Grave:      return VK_OEM_3;
        case za::Keyboard::Key::Equal:      return VK_OEM_PLUS;
        case za::Keyboard::Key::Hyphen:     return VK_OEM_MINUS;
        case za::Keyboard::Key::Space:      return VK_SPACE;
        case za::Keyboard::Key::Enter:      return VK_RETURN;
        case za::Keyboard::Key::Backspace:  return VK_BACK;
        case za::Keyboard::Key::Tab:        return VK_TAB;
        case za::Keyboard::Key::PageUp:     return VK_PRIOR;
        case za::Keyboard::Key::PageDown:   return VK_NEXT;
        case za::Keyboard::Key::End:        return VK_END;
        case za::Keyboard::Key::Home:       return VK_HOME;
        case za::Keyboard::Key::Insert:     return VK_INSERT;
        case za::Keyboard::Key::Delete:     return VK_DELETE;
        case za::Keyboard::Key::Add:        return VK_ADD;
        case za::Keyboard::Key::Subtract:   return VK_SUBTRACT;
        case za::Keyboard::Key::Multiply:   return VK_MULTIPLY;
        case za::Keyboard::Key::Divide:     return VK_DIVIDE;
        case za::Keyboard::Key::Left:       return VK_LEFT;
        case za::Keyboard::Key::Right:      return VK_RIGHT;
        case za::Keyboard::Key::Up:         return VK_UP;
        case za::Keyboard::Key::Down:       return VK_DOWN;
        case za::Keyboard::Key::Numpad0:    return VK_NUMPAD0;
        case za::Keyboard::Key::Numpad1:    return VK_NUMPAD1;
        case za::Keyboard::Key::Numpad2:    return VK_NUMPAD2;
        case za::Keyboard::Key::Numpad3:    return VK_NUMPAD3;
        case za::Keyboard::Key::Numpad4:    return VK_NUMPAD4;
        case za::Keyboard::Key::Numpad5:    return VK_NUMPAD5;
        case za::Keyboard::Key::Numpad6:    return VK_NUMPAD6;
        case za::Keyboard::Key::Numpad7:    return VK_NUMPAD7;
        case za::Keyboard::Key::Numpad8:    return VK_NUMPAD8;
        case za::Keyboard::Key::Numpad9:    return VK_NUMPAD9;
        case za::Keyboard::Key::F1:         return VK_F1;
        case za::Keyboard::Key::F2:         return VK_F2;
        case za::Keyboard::Key::F3:         return VK_F3;
        case za::Keyboard::Key::F4:         return VK_F4;
        case za::Keyboard::Key::F5:         return VK_F5;
        case za::Keyboard::Key::F6:         return VK_F6;
        case za::Keyboard::Key::F7:         return VK_F7;
        case za::Keyboard::Key::F8:         return VK_F8;
        case za::Keyboard::Key::F9:         return VK_F9;
        case za::Keyboard::Key::F10:        return VK_F10;
        case za::Keyboard::Key::F11:        return VK_F11;
        case za::Keyboard::Key::F12:        return VK_F12;
        case za::Keyboard::Key::F13:        return VK_F13;
        case za::Keyboard::Key::F14:        return VK_F14;
        case za::Keyboard::Key::F15:        return VK_F15;
        case za::Keyboard::Key::Pause:      return VK_PAUSE;
        default:                            return 0;
    }
    // clang-format on
}

[[nodiscard]] WORD sfScanToWinScan(za::Keyboard::Scancode code)
{
    // Convert an SFML scancode to a Windows scancode
    // Reference: https://msdn.microsoft.com/en-us/library/aa299374(v=vs.60).aspx
    // clang-format off
    switch (code)
    {
        case za::Keyboard::Scan::A: return 0x1E;
        case za::Keyboard::Scan::B: return 0x30;
        case za::Keyboard::Scan::C: return 0x2E;
        case za::Keyboard::Scan::D: return 0x20;
        case za::Keyboard::Scan::E: return 0x12;
        case za::Keyboard::Scan::F: return 0x21;
        case za::Keyboard::Scan::G: return 0x22;
        case za::Keyboard::Scan::H: return 0x23;
        case za::Keyboard::Scan::I: return 0x17;
        case za::Keyboard::Scan::J: return 0x24;
        case za::Keyboard::Scan::K: return 0x25;
        case za::Keyboard::Scan::L: return 0x26;
        case za::Keyboard::Scan::M: return 0x32;
        case za::Keyboard::Scan::N: return 0x31;
        case za::Keyboard::Scan::O: return 0x18;
        case za::Keyboard::Scan::P: return 0x19;
        case za::Keyboard::Scan::Q: return 0x10;
        case za::Keyboard::Scan::R: return 0x13;
        case za::Keyboard::Scan::S: return 0x1F;
        case za::Keyboard::Scan::T: return 0x14;
        case za::Keyboard::Scan::U: return 0x16;
        case za::Keyboard::Scan::V: return 0x2F;
        case za::Keyboard::Scan::W: return 0x11;
        case za::Keyboard::Scan::X: return 0x2D;
        case za::Keyboard::Scan::Y: return 0x15;
        case za::Keyboard::Scan::Z: return 0x2C;

        case za::Keyboard::Scan::Num1: return 0x02;
        case za::Keyboard::Scan::Num2: return 0x03;
        case za::Keyboard::Scan::Num3: return 0x04;
        case za::Keyboard::Scan::Num4: return 0x05;
        case za::Keyboard::Scan::Num5: return 0x06;
        case za::Keyboard::Scan::Num6: return 0x07;
        case za::Keyboard::Scan::Num7: return 0x08;
        case za::Keyboard::Scan::Num8: return 0x09;
        case za::Keyboard::Scan::Num9: return 0x0A;
        case za::Keyboard::Scan::Num0: return 0x0B;

        case za::Keyboard::Scan::Enter:      return 0x1C;
        case za::Keyboard::Scan::Escape:     return 0x01;
        case za::Keyboard::Scan::Backspace:  return 0x0E;
        case za::Keyboard::Scan::Tab:        return 0x0F;
        case za::Keyboard::Scan::Space:      return 0x39;
        case za::Keyboard::Scan::Hyphen:     return 0x0C;
        case za::Keyboard::Scan::Equal:      return 0x0D;
        case za::Keyboard::Scan::LBracket:   return 0x1A;
        case za::Keyboard::Scan::RBracket:   return 0x1B;
        case za::Keyboard::Scan::Backslash:  return 0x2B;
        case za::Keyboard::Scan::Semicolon:  return 0x27;
        case za::Keyboard::Scan::Apostrophe: return 0x28;
        case za::Keyboard::Scan::Grave:      return 0x29;
        case za::Keyboard::Scan::Comma:      return 0x33;
        case za::Keyboard::Scan::Period:     return 0x34;
        case za::Keyboard::Scan::Slash:      return 0x35;

        case za::Keyboard::Scan::F1:  return 0x3B;
        case za::Keyboard::Scan::F2:  return 0x3C;
        case za::Keyboard::Scan::F3:  return 0x3D;
        case za::Keyboard::Scan::F4:  return 0x3E;
        case za::Keyboard::Scan::F5:  return 0x3F;
        case za::Keyboard::Scan::F6:  return 0x40;
        case za::Keyboard::Scan::F7:  return 0x41;
        case za::Keyboard::Scan::F8:  return 0x42;
        case za::Keyboard::Scan::F9:  return 0x43;
        case za::Keyboard::Scan::F10: return 0x44;
        case za::Keyboard::Scan::F11: return 0x57;
        case za::Keyboard::Scan::F12: return 0x58;
        case za::Keyboard::Scan::F13: return 0x64;
        case za::Keyboard::Scan::F14: return 0x65;
        case za::Keyboard::Scan::F15: return 0x66;
        case za::Keyboard::Scan::F16: return 0x67;
        case za::Keyboard::Scan::F17: return 0x68;
        case za::Keyboard::Scan::F18: return 0x69;
        case za::Keyboard::Scan::F19: return 0x6A;
        case za::Keyboard::Scan::F20: return 0x6B;
        case za::Keyboard::Scan::F21: return 0x6C;
        case za::Keyboard::Scan::F22: return 0x6D;
        case za::Keyboard::Scan::F23: return 0x6E;
        case za::Keyboard::Scan::F24: return 0x76;

        case za::Keyboard::Scan::CapsLock:    return 0x3A;
        case za::Keyboard::Scan::PrintScreen: return 0xE037;
        case za::Keyboard::Scan::ScrollLock:  return 0x46;
        case za::Keyboard::Scan::Pause:       return 0x45;
        case za::Keyboard::Scan::Insert:      return 0xE052;
        case za::Keyboard::Scan::Home:        return 0xE047;
        case za::Keyboard::Scan::PageUp:      return 0xE049;
        case za::Keyboard::Scan::Delete:      return 0xE053;
        case za::Keyboard::Scan::End:         return 0xE04F;
        case za::Keyboard::Scan::PageDown:    return 0xE051;
        case za::Keyboard::Scan::Right:       return 0xE04D;
        case za::Keyboard::Scan::Left:        return 0xE04B;
        case za::Keyboard::Scan::Down:        return 0xE050;
        case za::Keyboard::Scan::Up:          return 0xE048;
        case za::Keyboard::Scan::NumLock:     return 0xE045;

        case za::Keyboard::Scan::NumpadDivide:   return 0xE035;
        case za::Keyboard::Scan::NumpadMultiply: return 0x37;
        case za::Keyboard::Scan::NumpadMinus:    return 0x4A;
        case za::Keyboard::Scan::NumpadPlus:     return 0x4E;
        case za::Keyboard::Scan::NumpadEqual:    return 0x7E;
        case za::Keyboard::Scan::NumpadEnter:    return 0xE01C;
        case za::Keyboard::Scan::NumpadDecimal:  return 0x53;

        case za::Keyboard::Scan::Numpad1:        return 0x4F;
        case za::Keyboard::Scan::Numpad2:        return 0x50;
        case za::Keyboard::Scan::Numpad3:        return 0x51;
        case za::Keyboard::Scan::Numpad4:        return 0x4B;
        case za::Keyboard::Scan::Numpad5:        return 0x4C;
        case za::Keyboard::Scan::Numpad6:        return 0x4D;
        case za::Keyboard::Scan::Numpad7:        return 0x47;
        case za::Keyboard::Scan::Numpad8:        return 0x48;
        case za::Keyboard::Scan::Numpad9:        return 0x49;
        case za::Keyboard::Scan::Numpad0:        return 0x52;

        case za::Keyboard::Scan::NonUsBackslash: return 0x56;
        // No known scancode for Keyboard::Scan::Application
        // No known scancode for Keyboard::Scan::Execute
        // No known scancode for Keyboard::Scan::ModeChange
        case za::Keyboard::Scan::Help:           return 0xE061;
        case za::Keyboard::Scan::Menu:           return 0xE05D;
        case za::Keyboard::Scan::Select:         return 0xE01E;
        // No known scancode for Keyboard::Scan::Redo
        // No known scancode for Keyboard::Scan::Undo
        // No known scancode for Keyboard::Scan::Cut
        // No known scancode for Keyboard::Scan::Copy
        // No known scancode for Keyboard::Scan::Paste

        case za::Keyboard::Scan::VolumeMute:         return 0xE020;
        case za::Keyboard::Scan::VolumeUp:           return 0xE02E;
        case za::Keyboard::Scan::VolumeDown:         return 0xE02C;
        case za::Keyboard::Scan::MediaPlayPause:     return 0xE022;
        case za::Keyboard::Scan::MediaStop:          return 0xE024;
        case za::Keyboard::Scan::MediaNextTrack:     return 0xE019;
        case za::Keyboard::Scan::MediaPreviousTrack: return 0xE010;

        case za::Keyboard::Scan::LControl: return 0x1D;
        case za::Keyboard::Scan::LShift:   return 0x2A;
        case za::Keyboard::Scan::LAlt:     return 0x38;
        case za::Keyboard::Scan::LSystem:  return 0xE05B;
        case za::Keyboard::Scan::RControl: return 0xE01D;
        case za::Keyboard::Scan::RShift:   return 0x36;
        case za::Keyboard::Scan::RAlt:     return 0xE038;
        case za::Keyboard::Scan::RSystem:  return 0xE05C;

        case za::Keyboard::Scan::Back:      return 0xE06A;
        case za::Keyboard::Scan::Forward:   return 0xE069;
        case za::Keyboard::Scan::Refresh:   return 0xE067;
        case za::Keyboard::Scan::Stop:      return 0xE068;
        case za::Keyboard::Scan::Search:    return 0xE065;
        case za::Keyboard::Scan::Favorites: return 0xE066;
        case za::Keyboard::Scan::HomePage:  return 0xE030;

        case za::Keyboard::Scan::LaunchApplication1: return 0xE06B;
        case za::Keyboard::Scan::LaunchApplication2: return 0xE021;
        case za::Keyboard::Scan::LaunchMail:         return 0xE06C;
        case za::Keyboard::Scan::LaunchMediaSelect:  return 0xE06D;

        // Unable to map to a scancode
        default: return 0x0;
    }
    // clang-format on
}

[[nodiscard]] WORD sfScanToWinScanExtended(za::Keyboard::Scancode code)
{
    // Convert an SFML scancode to a Windows scancode
    // Reference: https://msdn.microsoft.com/en-us/library/aa299374(v=vs.60).aspx
    // clang-format off
    switch (code)
    {
        case za::Keyboard::Scan::PrintScreen:        return 55  | 0xE100;
        case za::Keyboard::Scan::Insert:             return 82  | 0xE100;
        case za::Keyboard::Scan::Home:               return 71  | 0xE100;
        case za::Keyboard::Scan::PageUp:             return 73  | 0xE100;
        case za::Keyboard::Scan::Delete:             return 83  | 0xE100;
        case za::Keyboard::Scan::End:                return 79  | 0xE100;
        case za::Keyboard::Scan::PageDown:           return 81  | 0xE100;
        case za::Keyboard::Scan::Right:              return 77  | 0xE100;
        case za::Keyboard::Scan::Left:               return 75  | 0xE100;
        case za::Keyboard::Scan::Down:               return 80  | 0xE100;
        case za::Keyboard::Scan::Up:                 return 72  | 0xE100;
        case za::Keyboard::Scan::NumLock:            return 69  | 0xE100;
        case za::Keyboard::Scan::NumpadEnter:        return 28  | 0xE100;
        case za::Keyboard::Scan::NumpadDivide:       return 53  | 0xE100;
        case za::Keyboard::Scan::Help:               return 97  | 0xE100;
        case za::Keyboard::Scan::Menu:               return 93  | 0xE100;
        case za::Keyboard::Scan::Select:             return 30  | 0xE100;
        case za::Keyboard::Scan::VolumeMute:         return 32  | 0xE100;
        case za::Keyboard::Scan::VolumeUp:           return 46  | 0xE100;
        case za::Keyboard::Scan::VolumeDown:         return 44  | 0xE100;
        case za::Keyboard::Scan::MediaPlayPause:     return 34  | 0xE100;
        case za::Keyboard::Scan::MediaStop:          return 36  | 0xE100;
        case za::Keyboard::Scan::MediaNextTrack:     return 25  | 0xE100;
        case za::Keyboard::Scan::MediaPreviousTrack: return 16  | 0xE100;
        case za::Keyboard::Scan::LSystem:            return 91  | 0xE100;
        case za::Keyboard::Scan::RControl:           return 29  | 0xE100;
        case za::Keyboard::Scan::RAlt:               return 56  | 0xE100;
        case za::Keyboard::Scan::RSystem:            return 92  | 0xE100;
        case za::Keyboard::Scan::Back:               return 106 | 0xE100;
        case za::Keyboard::Scan::Forward:            return 105 | 0xE100;
        case za::Keyboard::Scan::Refresh:            return 103 | 0xE100;
        case za::Keyboard::Scan::Stop:               return 104 | 0xE100;
        case za::Keyboard::Scan::Search:             return 101 | 0xE100;
        case za::Keyboard::Scan::Favorites:          return 102 | 0xE100;
        case za::Keyboard::Scan::HomePage:           return 48  | 0xE100;
        case za::Keyboard::Scan::LaunchApplication1: return 107 | 0xE100;
        case za::Keyboard::Scan::LaunchApplication2: return 33  | 0xE100;
        case za::Keyboard::Scan::LaunchMail:         return 108 | 0xE100;
        case za::Keyboard::Scan::LaunchMediaSelect:  return 109 | 0xE100;

        // Use non-extended mapping
        default: return sfScanToWinScan(code);
    }
    // clang-format on
}

[[nodiscard]] UINT sfScanToVirtualKey(za::Keyboard::Scancode code)
{
    const WORD winScancode = sfScanToWinScan(code);

    // Manually map non-extended key codes
    // clang-format off
    switch (code)
    {
        case za::Keyboard::Scan::Numpad0:       return VK_NUMPAD0;
        case za::Keyboard::Scan::Numpad1:       return VK_NUMPAD1;
        case za::Keyboard::Scan::Numpad2:       return VK_NUMPAD2;
        case za::Keyboard::Scan::Numpad3:       return VK_NUMPAD3;
        case za::Keyboard::Scan::Numpad4:       return VK_NUMPAD4;
        case za::Keyboard::Scan::Numpad5:       return VK_NUMPAD5;
        case za::Keyboard::Scan::Numpad6:       return VK_NUMPAD6;
        case za::Keyboard::Scan::Numpad7:       return VK_NUMPAD7;
        case za::Keyboard::Scan::Numpad8:       return VK_NUMPAD8;
        case za::Keyboard::Scan::Numpad9:       return VK_NUMPAD9;
        case za::Keyboard::Scan::NumpadMinus:   return VK_SUBTRACT;
        case za::Keyboard::Scan::NumpadDecimal: return VK_DECIMAL;
        case za::Keyboard::Scan::NumpadDivide:  return VK_DIVIDE;
        case za::Keyboard::Scan::Pause:         return VK_PAUSE;
        case za::Keyboard::Scan::RControl:      return VK_RCONTROL;
        case za::Keyboard::Scan::RAlt:          return VK_RMENU;
        default:                            return MapVirtualKey(winScancode, MAPVK_VSC_TO_VK_EX);
    }
    // clang-format on
}

[[nodiscard]] zb::Optional<za::String> sfScanToConsumerKeyName(za::Keyboard::Scancode code)
{
    // Convert an SFML scancode to a Windows consumer keyboard key name
    // Reference: https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-messages
    // clang-format off
    switch (code)
    {
        case za::Keyboard::Scan::MediaNextTrack:     return zb::makeOptional<za::String>("Next Track");
        case za::Keyboard::Scan::MediaPreviousTrack: return zb::makeOptional<za::String>("Previous Track");
        case za::Keyboard::Scan::MediaStop:          return zb::makeOptional<za::String>("Stop");
        case za::Keyboard::Scan::MediaPlayPause:     return zb::makeOptional<za::String>("Play/Pause");
        case za::Keyboard::Scan::VolumeMute:         return zb::makeOptional<za::String>("Mute");
        case za::Keyboard::Scan::VolumeUp:           return zb::makeOptional<za::String>("Volume Increment");
        case za::Keyboard::Scan::VolumeDown:         return zb::makeOptional<za::String>("Volume Decrement");
        case za::Keyboard::Scan::LaunchMediaSelect:  return zb::makeOptional<za::String>("Consumer Control Configuration");
        case za::Keyboard::Scan::LaunchMail:         return zb::makeOptional<za::String>("Email Reader");
        case za::Keyboard::Scan::LaunchApplication2: return zb::makeOptional<za::String>("Calculator");
        case za::Keyboard::Scan::LaunchApplication1: return zb::makeOptional<za::String>("Local Machine Browser");
        case za::Keyboard::Scan::Search:             return zb::makeOptional<za::String>("Search");
        case za::Keyboard::Scan::HomePage:           return zb::makeOptional<za::String>("Home");
        case za::Keyboard::Scan::Back:               return zb::makeOptional<za::String>("Back");
        case za::Keyboard::Scan::Forward:            return zb::makeOptional<za::String>("Forward");
        case za::Keyboard::Scan::Stop:               return zb::makeOptional<za::String>("Stop");
        case za::Keyboard::Scan::Refresh:            return zb::makeOptional<za::String>("Refresh");
        case za::Keyboard::Scan::Favorites:          return zb::makeOptional<za::String>("Bookmarks");

        // Not a consumer key
        default: return zb::nullOpt;
    }
    // clang-format on
}

/// Ensure the mappings are generated from/to Key and Scancode.
void ensureMappings()
{
    static bool isMappingInitialized = false;

    if (isMappingInitialized)
        return;

    // Phase 1: Initialize mappings with default values
    keyToScancodeMapping.fill(za::Keyboard::Scan::Unknown);
    scancodeToKeyMapping.fill(za::Keyboard::Key::Unknown);

    // Phase 2: Translate scancode to virtual code to key names
    for (unsigned int i = 0; i < za::Keyboard::ScancodeCount; ++i)
    {
        const auto              scan       = static_cast<za::Keyboard::Scancode>(i);
        const UINT              virtualKey = sfScanToVirtualKey(scan);
        const za::Keyboard::Key key        = virtualKeyToSfKey(virtualKey);
        if (key != za::Keyboard::Key::Unknown && keyToScancodeMapping[key] == za::Keyboard::Scan::Unknown)
            keyToScancodeMapping[key] = scan;
        scancodeToKeyMapping[scan] = key;
    }

    isMappingInitialized = true;
}

[[nodiscard]] bool isValidScancode(za::Keyboard::Scancode code)
{
    return code > za::Keyboard::Scan::Unknown && static_cast<unsigned int>(code) < za::Keyboard::ScancodeCount;
}

[[nodiscard]] bool isValidKey(za::Keyboard::Key key)
{
    return key > za::Keyboard::Key::Unknown && static_cast<unsigned int>(key) < za::Keyboard::KeyCount;
}

} // namespace


namespace za::priv::InputImpl
{
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Key key)
{
    const int virtualKey = sfKeyToVirtualKey(key);
    return (GetAsyncKeyState(virtualKey) & 0x80'00) != 0;
}


////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Scancode code)
{
    const UINT virtualKey = sfScanToVirtualKey(code);
    return (GetAsyncKeyState(static_cast<int>(virtualKey)) & KF_UP) != 0;
}


////////////////////////////////////////////////////////////
Keyboard::Key localize(Keyboard::Scancode code)
{
    if (!isValidScancode(code))
        return Keyboard::Key::Unknown;

    ensureMappings();

    return scancodeToKeyMapping[code];
}


////////////////////////////////////////////////////////////
Keyboard::Scancode delocalize(Keyboard::Key key)
{
    if (!isValidKey(key))
        return Keyboard::Scan::Unknown;

    ensureMappings();

    return keyToScancodeMapping[key];
}


////////////////////////////////////////////////////////////
String getDescription(Keyboard::Scancode code)
{
    // Try to translate the scan code to a consumer key
    if (const zb::Optional consumerKeyName = sfScanToConsumerKeyName(code))
        return *consumerKeyName;

    WORD      winCode = sfScanToWinScanExtended(code);
    const int bufSize = 1024;
    WCHAR     name[bufSize];

    // Remap F13-F23 to values supported by GetKeyNameText
    if ((winCode >= 0x64) && (winCode <= 0x6E))
        winCode += 0x18;

    // Remap F24 to value supported by GetKeyNameText
    if (winCode == 0x76)
        winCode = 0x87;

    const int result = GetKeyNameTextW(winCode << 16, name, bufSize);
    if (result > 0)
    {
        return name;
    }
    return "Unknown";
}


////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(bool /*visible*/)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool isMouseButtonPressed(Mouse::Button button)
{
    const int virtualKey = [&]
    {
        switch (button)
        {
            case Mouse::Button::Left:
                return GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
            case Mouse::Button::Right:
                return GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON;
            case Mouse::Button::Middle:
                return VK_MBUTTON;
            case Mouse::Button::Extra1:
                return VK_XBUTTON1;
            case Mouse::Button::Extra2:
                return VK_XBUTTON2;
            default:
                return 0;
        }
    }();

    return (GetAsyncKeyState(virtualKey) & 0x80'00) != 0;
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition()
{
    POINT point;
    GetCursorPos(&point);
    return {point.x, point.y};
}


////////////////////////////////////////////////////////////
Vector2i getMousePosition(const WindowBase& relativeTo)
{
    if (const WindowHandle handle = relativeTo.getNativeHandle())
    {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(handle, &point);
        return {point.x, point.y};
    }

    return {};
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position)
{
    SetCursorPos(position.x, position.y);
}


////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position, const WindowBase& relativeTo)
{
    if (const WindowHandle handle = relativeTo.getNativeHandle())
    {
        POINT point = {position.x, position.y};
        ClientToScreen(handle, &point);
        SetCursorPos(point.x, point.y);
    }
}


////////////////////////////////////////////////////////////
// TODO P0:
bool isTouchDown(unsigned int /* finger */)
{
    // TODO P1: implement via WM_ pointer messages
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
// TODO P0:
Vector2i getTouchPosition(unsigned int /* finger */)
{
    // TODO P1: implement via WM_ pointer messages
    // Not applicable
    return {};
}


////////////////////////////////////////////////////////////
// TODO P0:
Vector2i getTouchPosition(unsigned int /* finger */, const WindowBase& /* relativeTo */)
{
    // TODO P1: implement via WM_ pointer messages
    // Not applicable
    return {};
}

} // namespace za::priv::InputImpl
