#include <Zancle/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Unix/Display.hpp"
#include "Zancle/Window/Unix/KeySymToKeyMapping.hpp"
#include "Zancle/Window/Unix/KeySymToUnicodeMapping.hpp"
#include "Zancle/Window/Unix/KeyboardImpl.hpp"

#include "Zancle/System/String.hpp"
#include "Zancle/System/Utf.hpp"

#include "ZancleBase/Builtins/Memcpy.hpp"
#include "ZancleBase/EnumArray.hpp"

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <array>
#include <string>
#include <unordered_map>
#include <utility>


namespace
{
////////////////////////////////////////////////////////////
constexpr KeyCode nullKeyCode = 0;
constexpr int     maxKeyCode  = 256;
zb::EnumArray<za::Keyboard::Scancode, KeyCode, za::Keyboard::ScancodeCount>
                                               scancodeToKeycode; ///< Mapping of SFML scancode to X11 KeyCode
std::array<za::Keyboard::Scancode, maxKeyCode> keycodeToScancode; ///< Mapping of X11 KeyCode to SFML scancode

////////////////////////////////////////////////////////////
bool isValidKeycode(KeyCode keycode)
{
    // Valid key code range is [8,255], according to the Xlib manual
    return keycode >= 8;
}


////////////////////////////////////////////////////////////
za::Keyboard::Scancode translateKeyCode(Display* display, KeyCode keycode)
{
    if (!isValidKeycode(keycode))
        return za::Keyboard::Scan::Unknown;

    // Try secondary keysym, for numeric keypad keys
    // Note: This way we always force "NumLock = ON", which is intentional
    // since the returned key code should correspond to a physical location.
    KeySym keySym = XkbKeycodeToKeysym(display, keycode, 0, 1);

    // clang-format off
    switch (keySym)
    {
        case XK_KP_0:         return za::Keyboard::Scan::Numpad0;
        case XK_KP_1:         return za::Keyboard::Scan::Numpad1;
        case XK_KP_2:         return za::Keyboard::Scan::Numpad2;
        case XK_KP_3:         return za::Keyboard::Scan::Numpad3;
        case XK_KP_4:         return za::Keyboard::Scan::Numpad4;
        case XK_KP_5:         return za::Keyboard::Scan::Numpad5;
        case XK_KP_6:         return za::Keyboard::Scan::Numpad6;
        case XK_KP_7:         return za::Keyboard::Scan::Numpad7;
        case XK_KP_8:         return za::Keyboard::Scan::Numpad8;
        case XK_KP_9:         return za::Keyboard::Scan::Numpad9;
        case XK_KP_Separator: return za::Keyboard::Scan::NumpadDecimal;
        case XK_KP_Decimal:   return za::Keyboard::Scan::NumpadDecimal;
        case XK_KP_Equal:     return za::Keyboard::Scan::NumpadEqual;
        case XK_KP_Enter:     return za::Keyboard::Scan::NumpadEnter;
        default:              break;
    }
    // clang-format on

    // Now try primary keysym for function keys (non-printable keys)
    // These should not depend on the current keyboard layout
    keySym = XkbKeycodeToKeysym(display, keycode, 0, 0);

    // clang-format off
    switch (keySym)
    {
        case XK_Return:           return za::Keyboard::Scan::Enter;
        case XK_Escape:           return za::Keyboard::Scan::Escape;
        case XK_BackSpace:        return za::Keyboard::Scan::Backspace;
        case XK_Tab:              return za::Keyboard::Scan::Tab;
        case XK_Shift_L:          return za::Keyboard::Scan::LShift;
        case XK_Shift_R:          return za::Keyboard::Scan::RShift;
        case XK_Control_L:        return za::Keyboard::Scan::LControl;
        case XK_Control_R:        return za::Keyboard::Scan::RControl;
        case XK_Alt_L:            return za::Keyboard::Scan::LAlt;
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Alt_R:            return za::Keyboard::Scan::RAlt;
        case XK_Meta_L:
        case XK_Super_L:          return za::Keyboard::Scan::LSystem;
        case XK_Meta_R:
        case XK_Super_R:          return za::Keyboard::Scan::RSystem;
        case XK_Menu:             return za::Keyboard::Scan::Menu;

        case XK_Num_Lock:         return za::Keyboard::Scan::NumLock;
        case XK_Caps_Lock:        return za::Keyboard::Scan::CapsLock;
        case XK_Execute:          return za::Keyboard::Scan::Execute;
        case XK_Hyper_R:          return za::Keyboard::Scan::Application;
        case XK_Select:           return za::Keyboard::Scan::Select;
        case XK_Cancel:           return za::Keyboard::Scan::Stop;
        case XK_Redo:             return za::Keyboard::Scan::Redo;
        case XK_Undo:             return za::Keyboard::Scan::Undo;
        case XK_Find:             return za::Keyboard::Scan::Search;
        case XK_Mode_switch:      return za::Keyboard::Scan::ModeChange;

        case XK_Print:            return za::Keyboard::Scan::PrintScreen;
        case XK_Scroll_Lock:      return za::Keyboard::Scan::ScrollLock;
        case XK_Pause:
        case XK_Break:            return za::Keyboard::Scan::Pause;

        case XK_Delete:
        case XK_Clear:            return za::Keyboard::Scan::Delete;
        case XK_Home:             return za::Keyboard::Scan::Home;
        case XK_End:              return za::Keyboard::Scan::End;
        case XK_Page_Up:          return za::Keyboard::Scan::PageUp;
        case XK_Page_Down:        return za::Keyboard::Scan::PageDown;
        case XK_Insert:           return za::Keyboard::Scan::Insert;

        case XK_Left:             return za::Keyboard::Scan::Left;
        case XK_Right:            return za::Keyboard::Scan::Right;
        case XK_Down:             return za::Keyboard::Scan::Down;
        case XK_Up:               return za::Keyboard::Scan::Up;

        case XK_F1:               return za::Keyboard::Scan::F1;
        case XK_F2:               return za::Keyboard::Scan::F2;
        case XK_F3:               return za::Keyboard::Scan::F3;
        case XK_F4:               return za::Keyboard::Scan::F4;
        case XK_F5:               return za::Keyboard::Scan::F5;
        case XK_F6:               return za::Keyboard::Scan::F6;
        case XK_F7:               return za::Keyboard::Scan::F7;
        case XK_F8:               return za::Keyboard::Scan::F8;
        case XK_F9:               return za::Keyboard::Scan::F9;
        case XK_F10:              return za::Keyboard::Scan::F10;
        case XK_F11:              return za::Keyboard::Scan::F11;
        case XK_F12:              return za::Keyboard::Scan::F12;
        case XK_F13:              return za::Keyboard::Scan::F13;
        case XK_F14:              return za::Keyboard::Scan::F14;
        case XK_F15:              return za::Keyboard::Scan::F15;
        case XK_F16:              return za::Keyboard::Scan::F16;
        case XK_F17:              return za::Keyboard::Scan::F17;
        case XK_F18:              return za::Keyboard::Scan::F18;
        case XK_F19:              return za::Keyboard::Scan::F19;
        case XK_F20:              return za::Keyboard::Scan::F20;
        case XK_F21:              return za::Keyboard::Scan::F21;
        case XK_F22:              return za::Keyboard::Scan::F22;
        case XK_F23:              return za::Keyboard::Scan::F23;
        case XK_F24:              return za::Keyboard::Scan::F24;

        // Numeric keypad
        case XK_KP_Divide:        return za::Keyboard::Scan::NumpadDivide;
        case XK_KP_Multiply:      return za::Keyboard::Scan::NumpadMultiply;
        case XK_KP_Subtract:      return za::Keyboard::Scan::NumpadMinus;
        case XK_KP_Add:           return za::Keyboard::Scan::NumpadPlus;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:        return za::Keyboard::Scan::Numpad0;
        case XK_KP_End:           return za::Keyboard::Scan::Numpad1;
        case XK_KP_Down:          return za::Keyboard::Scan::Numpad2;
        case XK_KP_Page_Down:     return za::Keyboard::Scan::Numpad3;
        case XK_KP_Left:          return za::Keyboard::Scan::Numpad4;
        case XK_KP_Right:         return za::Keyboard::Scan::Numpad6;
        case XK_KP_Home:          return za::Keyboard::Scan::Numpad7;
        case XK_KP_Up:            return za::Keyboard::Scan::Numpad8;
        case XK_KP_Page_Up:       return za::Keyboard::Scan::Numpad9;
        case XK_KP_Delete:        return za::Keyboard::Scan::NumpadDecimal;
        case XK_KP_Equal:         return za::Keyboard::Scan::NumpadEqual;
        case XK_KP_Enter:         return za::Keyboard::Scan::NumpadEnter;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:
        case XK_A:                return za::Keyboard::Scan::A;
        case XK_b:
        case XK_B:                return za::Keyboard::Scan::B;
        case XK_c:
        case XK_C:                return za::Keyboard::Scan::C;
        case XK_d:
        case XK_D:                return za::Keyboard::Scan::D;
        case XK_e:
        case XK_E:                return za::Keyboard::Scan::E;
        case XK_f:
        case XK_F:                return za::Keyboard::Scan::F;
        case XK_g:
        case XK_G:                return za::Keyboard::Scan::G;
        case XK_h:
        case XK_H:                return za::Keyboard::Scan::H;
        case XK_i:
        case XK_I:                return za::Keyboard::Scan::I;
        case XK_j:
        case XK_J:                return za::Keyboard::Scan::J;
        case XK_k:
        case XK_K:                return za::Keyboard::Scan::K;
        case XK_l:
        case XK_L:                return za::Keyboard::Scan::L;
        case XK_m:
        case XK_M:                return za::Keyboard::Scan::M;
        case XK_n:
        case XK_N:                return za::Keyboard::Scan::N;
        case XK_o:
        case XK_O:                return za::Keyboard::Scan::O;
        case XK_p:
        case XK_P:                return za::Keyboard::Scan::P;
        case XK_q:
        case XK_Q:                return za::Keyboard::Scan::Q;
        case XK_r:
        case XK_R:                return za::Keyboard::Scan::R;
        case XK_s:
        case XK_S:                return za::Keyboard::Scan::S;
        case XK_t:
        case XK_T:                return za::Keyboard::Scan::T;
        case XK_u:
        case XK_U:                return za::Keyboard::Scan::U;
        case XK_v:
        case XK_V:                return za::Keyboard::Scan::V;
        case XK_w:
        case XK_W:                return za::Keyboard::Scan::W;
        case XK_x:
        case XK_X:                return za::Keyboard::Scan::X;
        case XK_y:
        case XK_Y:                return za::Keyboard::Scan::Y;
        case XK_z:
        case XK_Z:                return za::Keyboard::Scan::Z;

        case XK_1:                return za::Keyboard::Scan::Num1;
        case XK_2:                return za::Keyboard::Scan::Num2;
        case XK_3:                return za::Keyboard::Scan::Num3;
        case XK_4:                return za::Keyboard::Scan::Num4;
        case XK_5:                return za::Keyboard::Scan::Num5;
        case XK_6:                return za::Keyboard::Scan::Num6;
        case XK_7:                return za::Keyboard::Scan::Num7;
        case XK_8:                return za::Keyboard::Scan::Num8;
        case XK_9:                return za::Keyboard::Scan::Num9;
        case XK_0:                return za::Keyboard::Scan::Num0;

        case XK_space:            return za::Keyboard::Scan::Space;
        case XK_minus:            return za::Keyboard::Scan::Hyphen;
        case XK_equal:            return za::Keyboard::Scan::Equal;
        case XK_bracketleft:      return za::Keyboard::Scan::LBracket;
        case XK_bracketright:     return za::Keyboard::Scan::RBracket;
        case XK_backslash:        return za::Keyboard::Scan::Backslash;
        case XK_semicolon:        return za::Keyboard::Scan::Semicolon;
        case XK_apostrophe:       return za::Keyboard::Scan::Apostrophe;
        case XK_grave:            return za::Keyboard::Scan::Grave;
        case XK_comma:            return za::Keyboard::Scan::Comma;
        case XK_period:           return za::Keyboard::Scan::Period;
        case XK_slash:            return za::Keyboard::Scan::Slash;
        case XK_less:             return za::Keyboard::Scan::NonUsBackslash;

        default:                  return za::Keyboard::Scan::Unknown;
    }
    // clang-format on
}


////////////////////////////////////////////////////////////
std::unordered_map<std::string, za::Keyboard::Scancode> getNameScancodeMap()
{
    return {

        {"LSGT", za::Keyboard::Scan::NonUsBackslash},

        {"TLDE", za::Keyboard::Scan::Grave},
        {"AE01", za::Keyboard::Scan::Num1},
        {"AE02", za::Keyboard::Scan::Num2},
        {"AE03", za::Keyboard::Scan::Num3},
        {"AE04", za::Keyboard::Scan::Num4},
        {"AE05", za::Keyboard::Scan::Num5},
        {"AE06", za::Keyboard::Scan::Num6},
        {"AE07", za::Keyboard::Scan::Num7},
        {"AE08", za::Keyboard::Scan::Num8},
        {"AE09", za::Keyboard::Scan::Num9},
        {"AE10", za::Keyboard::Scan::Num0},
        {"AE11", za::Keyboard::Scan::Hyphen},
        {"AE12", za::Keyboard::Scan::Equal},
        {"BKSP", za::Keyboard::Scan::Backspace},
        {"TAB", za::Keyboard::Scan::Tab},
        {"AD01", za::Keyboard::Scan::Q},
        {"AD02", za::Keyboard::Scan::W},
        {"AD03", za::Keyboard::Scan::E},
        {"AD04", za::Keyboard::Scan::R},
        {"AD05", za::Keyboard::Scan::T},
        {"AD06", za::Keyboard::Scan::Y},
        {"AD07", za::Keyboard::Scan::U},
        {"AD08", za::Keyboard::Scan::I},
        {"AD09", za::Keyboard::Scan::O},
        {"AD10", za::Keyboard::Scan::P},
        {"AD11", za::Keyboard::Scan::LBracket},
        {"AD12", za::Keyboard::Scan::RBracket},
        {"BKSL", za::Keyboard::Scan::Backslash},
        {"RTRN", za::Keyboard::Scan::Enter},

        {"CAPS", za::Keyboard::Scan::CapsLock},
        {"AC01", za::Keyboard::Scan::A},
        {"AC02", za::Keyboard::Scan::S},
        {"AC03", za::Keyboard::Scan::D},
        {"AC04", za::Keyboard::Scan::F},
        {"AC05", za::Keyboard::Scan::G},
        {"AC06", za::Keyboard::Scan::H},
        {"AC07", za::Keyboard::Scan::J},
        {"AC08", za::Keyboard::Scan::K},
        {"AC09", za::Keyboard::Scan::L},
        {"AC10", za::Keyboard::Scan::Semicolon},
        {"AC11", za::Keyboard::Scan::Apostrophe},
        {"AC12", za::Keyboard::Scan::Backslash},

        {"LFSH", za::Keyboard::Scan::LShift},
        {"AB01", za::Keyboard::Scan::Z},
        {"AB02", za::Keyboard::Scan::X},
        {"AB03", za::Keyboard::Scan::C},
        {"AB04", za::Keyboard::Scan::V},
        {"AB05", za::Keyboard::Scan::B},
        {"AB06", za::Keyboard::Scan::N},
        {"AB07", za::Keyboard::Scan::M},
        {"AB08", za::Keyboard::Scan::Comma},
        {"AB09", za::Keyboard::Scan::Period},
        {"AB10", za::Keyboard::Scan::Slash},
        {"RTSH", za::Keyboard::Scan::RShift},

        {"LCTL", za::Keyboard::Scan::LControl},
        {"LALT", za::Keyboard::Scan::LAlt},
        {"SPCE", za::Keyboard::Scan::Space},
        {"RCTL", za::Keyboard::Scan::RControl},
        {"RALT", za::Keyboard::Scan::RAlt},
        {"LVL3", za::Keyboard::Scan::RAlt},
        {"ALGR", za::Keyboard::Scan::RAlt},
        {"LWIN", za::Keyboard::Scan::LSystem},
        {"RWIN", za::Keyboard::Scan::RSystem},

        {"HYPR", za::Keyboard::Scan::Application},
        {"EXEC", za::Keyboard::Scan::Execute},
        {"MDSW", za::Keyboard::Scan::ModeChange},
        {"MENU", za::Keyboard::Scan::Menu},
        {"COMP", za::Keyboard::Scan::Menu},
        {"SELE", za::Keyboard::Scan::Select},

        {"ESC", za::Keyboard::Scan::Escape},
        {"FK01", za::Keyboard::Scan::F1},
        {"FK02", za::Keyboard::Scan::F2},
        {"FK03", za::Keyboard::Scan::F3},
        {"FK04", za::Keyboard::Scan::F4},
        {"FK05", za::Keyboard::Scan::F5},
        {"FK06", za::Keyboard::Scan::F6},
        {"FK07", za::Keyboard::Scan::F7},
        {"FK08", za::Keyboard::Scan::F8},
        {"FK09", za::Keyboard::Scan::F9},
        {"FK10", za::Keyboard::Scan::F10},
        {"FK11", za::Keyboard::Scan::F11},
        {"FK12", za::Keyboard::Scan::F12},

        {"PRSC", za::Keyboard::Scan::PrintScreen},
        {"SCLK", za::Keyboard::Scan::ScrollLock},
        {"PAUS", za::Keyboard::Scan::Pause},

        {"INS", za::Keyboard::Scan::Insert},
        {"HOME", za::Keyboard::Scan::Home},
        {"PGUP", za::Keyboard::Scan::PageUp},
        {"DELE", za::Keyboard::Scan::Delete},
        {"END", za::Keyboard::Scan::End},
        {"PGDN", za::Keyboard::Scan::PageDown},

        {"UP", za::Keyboard::Scan::Up},
        {"RGHT", za::Keyboard::Scan::Right},
        {"DOWN", za::Keyboard::Scan::Down},
        {"LEFT", za::Keyboard::Scan::Left},

        {"NMLK", za::Keyboard::Scan::NumLock},
        {"KPDV", za::Keyboard::Scan::NumpadDivide},
        {"KPMU", za::Keyboard::Scan::NumpadMultiply},
        {"KPSU", za::Keyboard::Scan::NumpadMinus},

        {"KP7", za::Keyboard::Scan::Numpad7},
        {"KP8", za::Keyboard::Scan::Numpad8},
        {"KP9", za::Keyboard::Scan::Numpad9},
        {"KPAD", za::Keyboard::Scan::NumpadPlus},
        {"KP4", za::Keyboard::Scan::Numpad4},
        {"KP5", za::Keyboard::Scan::Numpad5},
        {"KP6", za::Keyboard::Scan::Numpad6},
        {"KP1", za::Keyboard::Scan::Numpad1},
        {"KP2", za::Keyboard::Scan::Numpad2},
        {"KP3", za::Keyboard::Scan::Numpad3},
        {"KPEN", za::Keyboard::Scan::NumpadEnter},
        {"KP0", za::Keyboard::Scan::Numpad0},
        {"KPDL", za::Keyboard::Scan::NumpadDecimal},
        {"KPEQ", za::Keyboard::Scan::NumpadEqual},

        {"FK13", za::Keyboard::Scan::F13},
        {"FK14", za::Keyboard::Scan::F14},
        {"FK15", za::Keyboard::Scan::F15},
        {"FK16", za::Keyboard::Scan::F16},
        {"FK17", za::Keyboard::Scan::F17},
        {"FK18", za::Keyboard::Scan::F18},
        {"FK19", za::Keyboard::Scan::F19},
        {"FK20", za::Keyboard::Scan::F20},
        {"FK21", za::Keyboard::Scan::F21},
        {"FK22", za::Keyboard::Scan::F22},
        {"FK23", za::Keyboard::Scan::F23},
        {"FK24", za::Keyboard::Scan::F24},
        {"LMTA", za::Keyboard::Scan::LSystem},
        {"RMTA", za::Keyboard::Scan::RSystem},
        {"MUTE", za::Keyboard::Scan::VolumeMute},
        {"VOL-", za::Keyboard::Scan::VolumeDown},
        {"VOL+", za::Keyboard::Scan::VolumeUp},
        {"STOP", za::Keyboard::Scan::Stop},
        {"REDO", za::Keyboard::Scan::Redo},
        {"AGAI", za::Keyboard::Scan::Redo},
        {"UNDO", za::Keyboard::Scan::Undo},
        {"COPY", za::Keyboard::Scan::Copy},
        {"PAST", za::Keyboard::Scan::Paste},
        {"FIND", za::Keyboard::Scan::Search},
        {"CUT", za::Keyboard::Scan::Cut},
        {"HELP", za::Keyboard::Scan::Help},

        // {"I156", za::Keyboard::Scan::LaunchApplication1},
        // {"I157", za::Keyboard::Scan::LaunchApplication2},
        {"I164", za::Keyboard::Scan::Favorites},
        {"I166", za::Keyboard::Scan::Back},
        {"I167", za::Keyboard::Scan::Forward},
        {"I171", za::Keyboard::Scan::MediaNextTrack},
        {"I172", za::Keyboard::Scan::MediaPlayPause},
        {"I173", za::Keyboard::Scan::MediaPreviousTrack},
        {"I174", za::Keyboard::Scan::MediaStop},
        {"I180", za::Keyboard::Scan::HomePage},
        {"I181", za::Keyboard::Scan::Refresh},
        // {"I223", za::Keyboard::Scan::LaunchMail},
        {"I234", za::Keyboard::Scan::LaunchMediaSelect}

    };
}


////////////////////////////////////////////////////////////
void ensureMapping()
{
    static bool isMappingInitialized = false;

    if (isMappingInitialized)
        return;

    // Phase 1: Initialize mappings with default values
    scancodeToKeycode.fill(nullKeyCode);
    keycodeToScancode.fill(za::Keyboard::Scan::Unknown);

    // Phase 2: Get XKB names with key code
    const auto display = za::priv::openDisplay();

    char       name[XkbKeyNameLength + 1];
    XkbDescPtr descriptor = XkbGetMap(display.get(), 0, XkbUseCoreKbd);
    XkbGetNames(display.get(), XkbKeyNamesMask, descriptor);

    std::unordered_map<std::string, za::Keyboard::Scancode> nameScancodeMap = getNameScancodeMap();

    for (int keycode = descriptor->min_key_code; keycode <= descriptor->max_key_code; ++keycode)
    {
        if (!isValidKeycode(static_cast<KeyCode>(keycode)))
        {
            continue;
        }

        ZB_MEMCPY(name, descriptor->names->keys[keycode].name, XkbKeyNameLength);
        name[XkbKeyNameLength] = '\0';

        const auto mappedScancode = nameScancodeMap.find(std::string(name));
        auto       scancode       = za::Keyboard::Scan::Unknown;

        if (mappedScancode != nameScancodeMap.end())
            scancode = mappedScancode->second;

        if (scancode != za::Keyboard::Scan::Unknown)
            scancodeToKeycode[scancode] = static_cast<KeyCode>(keycode);

        keycodeToScancode[static_cast<KeyCode>(keycode)] = scancode;
    }

    XkbFreeNames(descriptor, XkbKeyNamesMask, True);
    XkbFreeKeyboard(descriptor, 0, True);

    // Phase 3: Translate un-translated keycodes using traditional X11 KeySym lookups
    for (int keycode = 8; keycode < maxKeyCode; ++keycode)
    {
        if (keycodeToScancode[static_cast<KeyCode>(keycode)] == za::Keyboard::Scan::Unknown)
        {
            const auto scancode = translateKeyCode(display.get(), static_cast<KeyCode>(keycode));

            if (scancode != za::Keyboard::Scan::Unknown && scancodeToKeycode[scancode] == nullKeyCode)
                scancodeToKeycode[scancode] = static_cast<KeyCode>(keycode);

            keycodeToScancode[static_cast<KeyCode>(keycode)] = scancode;
        }
    }

    isMappingInitialized = true;
}


////////////////////////////////////////////////////////////
KeyCode scancodeToKeyCode(za::Keyboard::Scancode code)
{
    ensureMapping();

    if (code != za::Keyboard::Scan::Unknown)
        return scancodeToKeycode[code];

    return nullKeyCode;
}


////////////////////////////////////////////////////////////
za::Keyboard::Scancode keyCodeToScancode(KeyCode code)
{
    ensureMapping();

    if (isValidKeycode(code))
        return keycodeToScancode[code];

    return za::Keyboard::Scan::Unknown;
}


////////////////////////////////////////////////////////////
KeyCode keyToKeyCode(za::Keyboard::Key key)
{
    const KeySym keysym = za::priv::keyToKeySym(key);

    if (keysym != NoSymbol)
    {
        const auto    display = za::priv::openDisplay();
        const KeyCode keycode = XKeysymToKeycode(display.get(), keysym);

        if (keycode != nullKeyCode)
            return keycode;
    }

    // Fallback for when XKeysymToKeycode cannot tell the KeyCode for XK_Alt_R
    if (key == za::Keyboard::Key::RAlt)
        return scancodeToKeycode[za::Keyboard::Scan::RAlt];

    return nullKeyCode;
}


////////////////////////////////////////////////////////////
KeySym scancodeToKeySym(za::Keyboard::Scancode code)
{
    const auto display = za::priv::openDisplay();

    KeySym        keysym  = NoSymbol;
    const KeyCode keycode = scancodeToKeyCode(code);

    if (keycode != nullKeyCode) // ensure that this Scancode is mapped to keycode
        keysym = XkbKeycodeToKeysym(display.get(), keycode, 0, 0);

    return keysym;
}


////////////////////////////////////////////////////////////
bool isKeyPressedImpl(KeyCode keycode)
{
    if (keycode != nullKeyCode)
    {
        const auto display = za::priv::openDisplay();

        // Get the whole keyboard state
        char keys[32];
        XQueryKeymap(display.get(), keys);

        // Check our keycode
        return (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
    }

    return false;
}

} // anonymous namespace

namespace za::priv
{

////////////////////////////////////////////////////////////
bool KeyboardImpl::isKeyPressed(Keyboard::Key key)
{
    const KeyCode keycode = keyToKeyCode(key);
    return isKeyPressedImpl(keycode);
}


////////////////////////////////////////////////////////////
bool KeyboardImpl::isKeyPressed(Keyboard::Scancode code)
{
    const KeyCode keycode = scancodeToKeyCode(code);
    return isKeyPressedImpl(keycode);
}


////////////////////////////////////////////////////////////
Keyboard::Scancode KeyboardImpl::delocalize(Keyboard::Key key)
{
    const KeyCode keycode = keyToKeyCode(key);
    return keyCodeToScancode(keycode);
}


////////////////////////////////////////////////////////////
Keyboard::Key KeyboardImpl::localize(Keyboard::Scancode code)
{
    const KeySym keysym = scancodeToKeySym(code);
    return keySymToKey(keysym);
}


////////////////////////////////////////////////////////////
String KeyboardImpl::getDescription(Keyboard::Scancode code)
{
    bool checkInput = true;

    // these scancodes actually correspond to keys with input
    // but we want to return their description, not their behaviour
    // clang-format off
    if (code == Keyboard::Scan::Enter ||
        code == Keyboard::Scan::Escape ||
        code == Keyboard::Scan::Backspace ||
        code == Keyboard::Scan::Tab ||
        code == Keyboard::Scan::Space ||
        code == Keyboard::Scan::ScrollLock ||
        code == Keyboard::Scan::Pause ||
        code == Keyboard::Scan::Delete ||
        code == Keyboard::Scan::NumpadDivide ||
        code == Keyboard::Scan::NumpadMultiply ||
        code == Keyboard::Scan::NumpadMinus ||
        code == Keyboard::Scan::NumpadPlus ||
        code == Keyboard::Scan::NumpadEqual ||
        code == Keyboard::Scan::NumpadEnter ||
        code == Keyboard::Scan::NumpadDecimal)
    // clang-format on
    {
        checkInput = false;
    }

    if (checkInput)
    {
        const KeySym   keysym  = scancodeToKeySym(code);
        const char32_t unicode = keysymToUnicode(keysym);

        if (unicode != 0)
            return {unicode};
    }

    // Fallback to our best guess for the keys that are known to be independent of the layout.
    // clang-format off
    switch (code)
    {
        case Keyboard::Scan::Enter:              return "Enter";
        case Keyboard::Scan::Escape:             return "Escape";
        case Keyboard::Scan::Backspace:          return "Backspace";
        case Keyboard::Scan::Tab:                return "Tab";
        case Keyboard::Scan::Space:              return "Space";

        case Keyboard::Scan::F1:                 return "F1";
        case Keyboard::Scan::F2:                 return "F2";
        case Keyboard::Scan::F3:                 return "F3";
        case Keyboard::Scan::F4:                 return "F4";
        case Keyboard::Scan::F5:                 return "F5";
        case Keyboard::Scan::F6:                 return "F6";
        case Keyboard::Scan::F7:                 return "F7";
        case Keyboard::Scan::F8:                 return "F8";
        case Keyboard::Scan::F9:                 return "F9";
        case Keyboard::Scan::F10:                return "F10";
        case Keyboard::Scan::F11:                return "F11";
        case Keyboard::Scan::F12:                return "F12";
        case Keyboard::Scan::F13:                return "F13";
        case Keyboard::Scan::F14:                return "F14";
        case Keyboard::Scan::F15:                return "F15";
        case Keyboard::Scan::F16:                return "F16";
        case Keyboard::Scan::F17:                return "F17";
        case Keyboard::Scan::F18:                return "F18";
        case Keyboard::Scan::F19:                return "F19";
        case Keyboard::Scan::F20:                return "F20";
        case Keyboard::Scan::F21:                return "F21";
        case Keyboard::Scan::F22:                return "F22";
        case Keyboard::Scan::F23:                return "F23";
        case Keyboard::Scan::F24:                return "F24";

        case Keyboard::Scan::CapsLock:           return "Caps Lock";
        case Keyboard::Scan::PrintScreen:        return "Print Screen";
        case Keyboard::Scan::ScrollLock:         return "Scroll Lock";

        case Keyboard::Scan::Pause:              return "Pause";
        case Keyboard::Scan::Insert:             return "Insert";
        case Keyboard::Scan::Home:               return "Home";
        case Keyboard::Scan::PageUp:             return "Page Up";
        case Keyboard::Scan::Delete:             return "Delete";
        case Keyboard::Scan::End:                return "End";
        case Keyboard::Scan::PageDown:           return "Page Down";

        case Keyboard::Scan::Left:               return "Left Arrow";
        case Keyboard::Scan::Right:              return "Right Arrow";
        case Keyboard::Scan::Down:               return "Down Arrow";
        case Keyboard::Scan::Up:                 return "Up Arrow";

        case Keyboard::Scan::NumLock:            return "Num Lock";
        case Keyboard::Scan::NumpadDivide:       return "Divide (Numpad)";
        case Keyboard::Scan::NumpadMultiply:     return "Multiply (Numpad)";
        case Keyboard::Scan::NumpadMinus:        return "Minus (Numpad)";
        case Keyboard::Scan::NumpadPlus:         return "Plus (Numpad)";
        case Keyboard::Scan::NumpadEqual:        return "Equal (Numpad)";
        case Keyboard::Scan::NumpadEnter:        return "Enter (Numpad)";
        case Keyboard::Scan::NumpadDecimal:      return "Decimal (Numpad)";

        case Keyboard::Scan::Numpad0:            return "0 (Numpad)";
        case Keyboard::Scan::Numpad1:            return "1 (Numpad)";
        case Keyboard::Scan::Numpad2:            return "2 (Numpad)";
        case Keyboard::Scan::Numpad3:            return "3 (Numpad)";
        case Keyboard::Scan::Numpad4:            return "4 (Numpad)";
        case Keyboard::Scan::Numpad5:            return "5 (Numpad)";
        case Keyboard::Scan::Numpad6:            return "6 (Numpad)";
        case Keyboard::Scan::Numpad7:            return "7 (Numpad)";
        case Keyboard::Scan::Numpad8:            return "8 (Numpad)";
        case Keyboard::Scan::Numpad9:            return "9 (Numpad)";

        case Keyboard::Scan::Application:        return "Application";
        case Keyboard::Scan::Execute:            return "Execute";
        case Keyboard::Scan::Help:               return "Help";
        case Keyboard::Scan::Menu:               return "Menu";
        case Keyboard::Scan::Select:             return "Select";
        case Keyboard::Scan::Stop:               return "Stop";
        case Keyboard::Scan::Redo:               return "Redo";
        case Keyboard::Scan::Undo:               return "Undo";
        case Keyboard::Scan::Cut:                return "Cut";
        case Keyboard::Scan::Copy:               return "Copy";
        case Keyboard::Scan::Paste:              return "Paste";
        case Keyboard::Scan::Search:             return "Search";

        case Keyboard::Scan::VolumeMute:         return "Volume Mute";
        case Keyboard::Scan::VolumeUp:           return "Volume Up";
        case Keyboard::Scan::VolumeDown:         return "Volume Down";

        case Keyboard::Scan::LControl:           return "Left Control";
        case Keyboard::Scan::LShift:             return "Left Shift";
        case Keyboard::Scan::LAlt:               return "Left Alt";
        case Keyboard::Scan::LSystem:            return "Left System";
        case Keyboard::Scan::RControl:           return "Right Control";
        case Keyboard::Scan::RShift:             return "Right Shift";
        case Keyboard::Scan::RAlt:               return "Right Alt";
        case Keyboard::Scan::RSystem:            return "Right System";

        // case Keyboard::Scan::LaunchApplication1: return "Launch Application 1";
        // case Keyboard::Scan::LaunchApplication2: return "Launch Application 2";
        case Keyboard::Scan::Favorites:          return "Favorites";
        case Keyboard::Scan::Back:               return "Back";
        case Keyboard::Scan::Forward:            return "Forward";
        case Keyboard::Scan::MediaNextTrack:     return "Media Next Track";
        case Keyboard::Scan::MediaPlayPause:     return "Media Play Pause";
        case Keyboard::Scan::MediaPreviousTrack: return "Media Previous Track";
        case Keyboard::Scan::MediaStop:          return "Media Stop";
        case Keyboard::Scan::HomePage:           return "Home Page";
        case Keyboard::Scan::Refresh:            return "Refresh";
        // case Keyboard::Scan::LaunchMail:         return "Launch Mail";
        case Keyboard::Scan::LaunchMediaSelect:  return "Launch Media Select";

        default:                                 return "Unknown Scancode";
    }
    // clang-format on
}


////////////////////////////////////////////////////////////
Keyboard::Key KeyboardImpl::getKeyFromEvent(XKeyEvent& event)
{
    // Try each KeySym index (modifier group) until we get a match
    for (int i = 0; i < 4; ++i)
    {
        // Get the SFML keyboard code from the keysym of the key that has been pressed
        const KeySym        keysym = XLookupKeysym(&event, i);
        const Keyboard::Key key    = keySymToKey(keysym);
        if (key != Keyboard::Key::Unknown)
            return key;
    }

    return Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
Keyboard::Scancode KeyboardImpl::getScancodeFromEvent(XKeyEvent& event)
{
    return keyCodeToScancode(static_cast<KeyCode>(event.keycode));
}

} // namespace za::priv
