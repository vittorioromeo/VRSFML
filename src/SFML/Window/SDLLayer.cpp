#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/StringUtfUtils.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Builtins/Memcmp.hpp"
#include "SFML/Base/Builtins/Strcmp.hpp"
#include "SFML/Base/Builtins/Strlen.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/ScopeGuard.hpp"

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_touch.h>
#include <SDL3/SDL_video.h>


////////////////////////////////////////////////////////////
#define SFML_PRIV_SFML_SDL_KEYCODE_MAPPING                                  \
    X(SDLK_UNKNOWN, ::sf::Keyboard::Key::Unknown)                           \
    X(SDLK_RETURN, ::sf::Keyboard::Key::Enter)                              \
    X(SDLK_ESCAPE, ::sf::Keyboard::Key::Escape)                             \
    X(SDLK_BACKSPACE, ::sf::Keyboard::Key::Backspace)                       \
    X(SDLK_TAB, ::sf::Keyboard::Key::Tab)                                   \
    X(SDLK_SPACE, ::sf::Keyboard::Key::Space)                               \
    X(SDLK_EXCLAIM, ::sf::Keyboard::Key::Exclaim)                           \
    X(SDLK_DBLAPOSTROPHE, ::sf::Keyboard::Key::DoubleApostrophe)            \
    X(SDLK_HASH, ::sf::Keyboard::Key::Hash)                                 \
    X(SDLK_DOLLAR, ::sf::Keyboard::Key::Dollar)                             \
    X(SDLK_PERCENT, ::sf::Keyboard::Key::Percent)                           \
    X(SDLK_AMPERSAND, ::sf::Keyboard::Key::Ampersand)                       \
    X(SDLK_APOSTROPHE, ::sf::Keyboard::Key::Apostrophe)                     \
    X(SDLK_LEFTPAREN, ::sf::Keyboard::Key::LParenthesis)                    \
    X(SDLK_RIGHTPAREN, ::sf::Keyboard::Key::RParenthesis)                   \
    X(SDLK_ASTERISK, ::sf::Keyboard::Key::Asterisk)                         \
    X(SDLK_PLUS, ::sf::Keyboard::Key::Plus)                                 \
    X(SDLK_COMMA, ::sf::Keyboard::Key::Comma)                               \
    X(SDLK_MINUS, ::sf::Keyboard::Key::Hyphen)                              \
    X(SDLK_PERIOD, ::sf::Keyboard::Key::Period)                             \
    X(SDLK_SLASH, ::sf::Keyboard::Key::Slash)                               \
    X(SDLK_0, ::sf::Keyboard::Key::Num0)                                    \
    X(SDLK_1, ::sf::Keyboard::Key::Num1)                                    \
    X(SDLK_2, ::sf::Keyboard::Key::Num2)                                    \
    X(SDLK_3, ::sf::Keyboard::Key::Num3)                                    \
    X(SDLK_4, ::sf::Keyboard::Key::Num4)                                    \
    X(SDLK_5, ::sf::Keyboard::Key::Num5)                                    \
    X(SDLK_6, ::sf::Keyboard::Key::Num6)                                    \
    X(SDLK_7, ::sf::Keyboard::Key::Num7)                                    \
    X(SDLK_8, ::sf::Keyboard::Key::Num8)                                    \
    X(SDLK_9, ::sf::Keyboard::Key::Num9)                                    \
    X(SDLK_COLON, ::sf::Keyboard::Key::Colon)                               \
    X(SDLK_SEMICOLON, ::sf::Keyboard::Key::Semicolon)                       \
    X(SDLK_LESS, ::sf::Keyboard::Key::Less)                                 \
    X(SDLK_EQUALS, ::sf::Keyboard::Key::Equal)                              \
    X(SDLK_GREATER, ::sf::Keyboard::Key::Greater)                           \
    X(SDLK_QUESTION, ::sf::Keyboard::Key::Question)                         \
    X(SDLK_AT, ::sf::Keyboard::Key::At)                                     \
    X(SDLK_LEFTBRACKET, ::sf::Keyboard::Key::LBracket)                      \
    X(SDLK_BACKSLASH, ::sf::Keyboard::Key::Backslash)                       \
    X(SDLK_RIGHTBRACKET, ::sf::Keyboard::Key::RBracket)                     \
    X(SDLK_CARET, ::sf::Keyboard::Key::Caret)                               \
    X(SDLK_UNDERSCORE, ::sf::Keyboard::Key::Underscore)                     \
    X(SDLK_GRAVE, ::sf::Keyboard::Key::Grave)                               \
    X(SDLK_A, ::sf::Keyboard::Key::A)                                       \
    X(SDLK_B, ::sf::Keyboard::Key::B)                                       \
    X(SDLK_C, ::sf::Keyboard::Key::C)                                       \
    X(SDLK_D, ::sf::Keyboard::Key::D)                                       \
    X(SDLK_E, ::sf::Keyboard::Key::E)                                       \
    X(SDLK_F, ::sf::Keyboard::Key::F)                                       \
    X(SDLK_G, ::sf::Keyboard::Key::G)                                       \
    X(SDLK_H, ::sf::Keyboard::Key::H)                                       \
    X(SDLK_I, ::sf::Keyboard::Key::I)                                       \
    X(SDLK_J, ::sf::Keyboard::Key::J)                                       \
    X(SDLK_K, ::sf::Keyboard::Key::K)                                       \
    X(SDLK_L, ::sf::Keyboard::Key::L)                                       \
    X(SDLK_M, ::sf::Keyboard::Key::M)                                       \
    X(SDLK_N, ::sf::Keyboard::Key::N)                                       \
    X(SDLK_O, ::sf::Keyboard::Key::O)                                       \
    X(SDLK_P, ::sf::Keyboard::Key::P)                                       \
    X(SDLK_Q, ::sf::Keyboard::Key::Q)                                       \
    X(SDLK_R, ::sf::Keyboard::Key::R)                                       \
    X(SDLK_S, ::sf::Keyboard::Key::S)                                       \
    X(SDLK_T, ::sf::Keyboard::Key::T)                                       \
    X(SDLK_U, ::sf::Keyboard::Key::U)                                       \
    X(SDLK_V, ::sf::Keyboard::Key::V)                                       \
    X(SDLK_W, ::sf::Keyboard::Key::W)                                       \
    X(SDLK_X, ::sf::Keyboard::Key::X)                                       \
    X(SDLK_Y, ::sf::Keyboard::Key::Y)                                       \
    X(SDLK_Z, ::sf::Keyboard::Key::Z)                                       \
    X(SDLK_LEFTBRACE, ::sf::Keyboard::Key::LBrace)                          \
    X(SDLK_PIPE, ::sf::Keyboard::Key::Pipe)                                 \
    X(SDLK_RIGHTBRACE, ::sf::Keyboard::Key::RBrace)                         \
    X(SDLK_TILDE, ::sf::Keyboard::Key::Tilde)                               \
    X(SDLK_DELETE, ::sf::Keyboard::Key::Delete)                             \
    X(SDLK_PLUSMINUS, ::sf::Keyboard::Key::PlusMinus)                       \
    X(SDLK_CAPSLOCK, ::sf::Keyboard::Key::CapsLock)                         \
    X(SDLK_F1, ::sf::Keyboard::Key::F1)                                     \
    X(SDLK_F2, ::sf::Keyboard::Key::F2)                                     \
    X(SDLK_F3, ::sf::Keyboard::Key::F3)                                     \
    X(SDLK_F4, ::sf::Keyboard::Key::F4)                                     \
    X(SDLK_F5, ::sf::Keyboard::Key::F5)                                     \
    X(SDLK_F6, ::sf::Keyboard::Key::F6)                                     \
    X(SDLK_F7, ::sf::Keyboard::Key::F7)                                     \
    X(SDLK_F8, ::sf::Keyboard::Key::F8)                                     \
    X(SDLK_F9, ::sf::Keyboard::Key::F9)                                     \
    X(SDLK_F10, ::sf::Keyboard::Key::F10)                                   \
    X(SDLK_F11, ::sf::Keyboard::Key::F11)                                   \
    X(SDLK_F12, ::sf::Keyboard::Key::F12)                                   \
    X(SDLK_PRINTSCREEN, ::sf::Keyboard::Key::PrintScreen)                   \
    X(SDLK_SCROLLLOCK, ::sf::Keyboard::Key::ScrollLock)                     \
    X(SDLK_PAUSE, ::sf::Keyboard::Key::Pause)                               \
    X(SDLK_INSERT, ::sf::Keyboard::Key::Insert)                             \
    X(SDLK_HOME, ::sf::Keyboard::Key::Home)                                 \
    X(SDLK_PAGEUP, ::sf::Keyboard::Key::PageUp)                             \
    X(SDLK_END, ::sf::Keyboard::Key::End)                                   \
    X(SDLK_PAGEDOWN, ::sf::Keyboard::Key::PageDown)                         \
    X(SDLK_RIGHT, ::sf::Keyboard::Key::Right)                               \
    X(SDLK_LEFT, ::sf::Keyboard::Key::Left)                                 \
    X(SDLK_DOWN, ::sf::Keyboard::Key::Down)                                 \
    X(SDLK_UP, ::sf::Keyboard::Key::Up)                                     \
    X(SDLK_NUMLOCKCLEAR, ::sf::Keyboard::Key::NumLock)                      \
    X(SDLK_KP_DIVIDE, ::sf::Keyboard::Key::Divide)                          \
    X(SDLK_KP_MULTIPLY, ::sf::Keyboard::Key::Multiply)                      \
    X(SDLK_KP_MINUS, ::sf::Keyboard::Key::Subtract)                         \
    X(SDLK_KP_PLUS, ::sf::Keyboard::Key::Add)                               \
    X(SDLK_KP_ENTER, ::sf::Keyboard::Key::NumpadEnter)                      \
    X(SDLK_KP_1, ::sf::Keyboard::Key::Numpad1)                              \
    X(SDLK_KP_2, ::sf::Keyboard::Key::Numpad2)                              \
    X(SDLK_KP_3, ::sf::Keyboard::Key::Numpad3)                              \
    X(SDLK_KP_4, ::sf::Keyboard::Key::Numpad4)                              \
    X(SDLK_KP_5, ::sf::Keyboard::Key::Numpad5)                              \
    X(SDLK_KP_6, ::sf::Keyboard::Key::Numpad6)                              \
    X(SDLK_KP_7, ::sf::Keyboard::Key::Numpad7)                              \
    X(SDLK_KP_8, ::sf::Keyboard::Key::Numpad8)                              \
    X(SDLK_KP_9, ::sf::Keyboard::Key::Numpad9)                              \
    X(SDLK_KP_0, ::sf::Keyboard::Key::Numpad0)                              \
    X(SDLK_KP_PERIOD, ::sf::Keyboard::Key::NumpadPeriod)                    \
    X(SDLK_APPLICATION, ::sf::Keyboard::Key::Application)                   \
    X(SDLK_POWER, ::sf::Keyboard::Key::Power)                               \
    X(SDLK_KP_EQUALS, ::sf::Keyboard::Key::NumpadEqual)                     \
    X(SDLK_F13, ::sf::Keyboard::Key::F13)                                   \
    X(SDLK_F14, ::sf::Keyboard::Key::F14)                                   \
    X(SDLK_F15, ::sf::Keyboard::Key::F15)                                   \
    X(SDLK_F16, ::sf::Keyboard::Key::F16)                                   \
    X(SDLK_F17, ::sf::Keyboard::Key::F17)                                   \
    X(SDLK_F18, ::sf::Keyboard::Key::F18)                                   \
    X(SDLK_F19, ::sf::Keyboard::Key::F19)                                   \
    X(SDLK_F20, ::sf::Keyboard::Key::F20)                                   \
    X(SDLK_F21, ::sf::Keyboard::Key::F21)                                   \
    X(SDLK_F22, ::sf::Keyboard::Key::F22)                                   \
    X(SDLK_F23, ::sf::Keyboard::Key::F23)                                   \
    X(SDLK_F24, ::sf::Keyboard::Key::F24)                                   \
    X(SDLK_EXECUTE, ::sf::Keyboard::Key::Execute)                           \
    X(SDLK_HELP, ::sf::Keyboard::Key::Help)                                 \
    X(SDLK_MENU, ::sf::Keyboard::Key::Menu)                                 \
    X(SDLK_SELECT, ::sf::Keyboard::Key::Select)                             \
    X(SDLK_STOP, ::sf::Keyboard::Key::Stop)                                 \
    X(SDLK_AGAIN, ::sf::Keyboard::Key::Redo)                                \
    X(SDLK_UNDO, ::sf::Keyboard::Key::Undo)                                 \
    X(SDLK_CUT, ::sf::Keyboard::Key::Cut)                                   \
    X(SDLK_COPY, ::sf::Keyboard::Key::Copy)                                 \
    X(SDLK_PASTE, ::sf::Keyboard::Key::Paste)                               \
    X(SDLK_FIND, ::sf::Keyboard::Key::Find)                                 \
    X(SDLK_MUTE, ::sf::Keyboard::Key::VolumeMute)                           \
    X(SDLK_VOLUMEUP, ::sf::Keyboard::Key::VolumeUp)                         \
    X(SDLK_VOLUMEDOWN, ::sf::Keyboard::Key::VolumeDown)                     \
    X(SDLK_KP_COMMA, ::sf::Keyboard::Key::NumpadComma)                      \
    X(SDLK_KP_EQUALSAS400, ::sf::Keyboard::Key::NumpadEqualAs400)           \
    X(SDLK_ALTERASE, ::sf::Keyboard::Key::AltErase)                         \
    X(SDLK_SYSREQ, ::sf::Keyboard::Key::SysReq)                             \
    X(SDLK_CANCEL, ::sf::Keyboard::Key::Cancel)                             \
    X(SDLK_CLEAR, ::sf::Keyboard::Key::Clear)                               \
    X(SDLK_PRIOR, ::sf::Keyboard::Key::Prior)                               \
    X(SDLK_RETURN2, ::sf::Keyboard::Key::Enter2)                            \
    X(SDLK_SEPARATOR, ::sf::Keyboard::Key::Separator)                       \
    X(SDLK_OUT, ::sf::Keyboard::Key::Out)                                   \
    X(SDLK_OPER, ::sf::Keyboard::Key::Oper)                                 \
    X(SDLK_CLEARAGAIN, ::sf::Keyboard::Key::ClearAgain)                     \
    X(SDLK_CRSEL, ::sf::Keyboard::Key::CrSel)                               \
    X(SDLK_EXSEL, ::sf::Keyboard::Key::ExSel)                               \
    X(SDLK_KP_00, ::sf::Keyboard::Key::Numpad00)                            \
    X(SDLK_KP_000, ::sf::Keyboard::Key::Numpad000)                          \
    X(SDLK_THOUSANDSSEPARATOR, ::sf::Keyboard::Key::ThousandsSeparator)     \
    X(SDLK_DECIMALSEPARATOR, ::sf::Keyboard::Key::DecimalSeparator)         \
    X(SDLK_CURRENCYUNIT, ::sf::Keyboard::Key::CurrencyUnit)                 \
    X(SDLK_CURRENCYSUBUNIT, ::sf::Keyboard::Key::CurrencySubUnit)           \
    X(SDLK_KP_LEFTPAREN, ::sf::Keyboard::Key::NumpadLParenthesis)           \
    X(SDLK_KP_RIGHTPAREN, ::sf::Keyboard::Key::NumpadRParenthesis)          \
    X(SDLK_KP_LEFTBRACE, ::sf::Keyboard::Key::NumpadLBrace)                 \
    X(SDLK_KP_RIGHTBRACE, ::sf::Keyboard::Key::NumpadRBrace)                \
    X(SDLK_KP_TAB, ::sf::Keyboard::Key::NumpadTab)                          \
    X(SDLK_KP_BACKSPACE, ::sf::Keyboard::Key::NumpadBackspace)              \
    X(SDLK_KP_A, ::sf::Keyboard::Key::NumpadA)                              \
    X(SDLK_KP_B, ::sf::Keyboard::Key::NumpadB)                              \
    X(SDLK_KP_C, ::sf::Keyboard::Key::NumpadC)                              \
    X(SDLK_KP_D, ::sf::Keyboard::Key::NumpadD)                              \
    X(SDLK_KP_E, ::sf::Keyboard::Key::NumpadE)                              \
    X(SDLK_KP_F, ::sf::Keyboard::Key::NumpadF)                              \
    X(SDLK_KP_XOR, ::sf::Keyboard::Key::NumpadXor)                          \
    X(SDLK_KP_POWER, ::sf::Keyboard::Key::NumpadPower)                      \
    X(SDLK_KP_PERCENT, ::sf::Keyboard::Key::NumpadPercent)                  \
    X(SDLK_KP_LESS, ::sf::Keyboard::Key::NumpadLess)                        \
    X(SDLK_KP_GREATER, ::sf::Keyboard::Key::NumpadGreater)                  \
    X(SDLK_KP_AMPERSAND, ::sf::Keyboard::Key::NumpadAmpersand)              \
    X(SDLK_KP_DBLAMPERSAND, ::sf::Keyboard::Key::NumpadDoubleAmpersand)     \
    X(SDLK_KP_VERTICALBAR, ::sf::Keyboard::Key::NumpadVerticalBar)          \
    X(SDLK_KP_DBLVERTICALBAR, ::sf::Keyboard::Key::NumpadDoubleVerticalBar) \
    X(SDLK_KP_COLON, ::sf::Keyboard::Key::NumpadColon)                      \
    X(SDLK_KP_HASH, ::sf::Keyboard::Key::NumpadHash)                        \
    X(SDLK_KP_SPACE, ::sf::Keyboard::Key::NumpadSpace)                      \
    X(SDLK_KP_AT, ::sf::Keyboard::Key::NumpadAt)                            \
    X(SDLK_KP_EXCLAM, ::sf::Keyboard::Key::NumpadExclamation)               \
    X(SDLK_KP_MEMSTORE, ::sf::Keyboard::Key::NumpadMemStore)                \
    X(SDLK_KP_MEMRECALL, ::sf::Keyboard::Key::NumpadMemRecall)              \
    X(SDLK_KP_MEMCLEAR, ::sf::Keyboard::Key::NumpadMemClear)                \
    X(SDLK_KP_MEMADD, ::sf::Keyboard::Key::NumpadMemAdd)                    \
    X(SDLK_KP_MEMSUBTRACT, ::sf::Keyboard::Key::NumpadMemSubtract)          \
    X(SDLK_KP_MEMMULTIPLY, ::sf::Keyboard::Key::NumpadMemMultiply)          \
    X(SDLK_KP_MEMDIVIDE, ::sf::Keyboard::Key::NumpadMemDivide)              \
    X(SDLK_KP_PLUSMINUS, ::sf::Keyboard::Key::NumpadPlusMinus)              \
    X(SDLK_KP_CLEAR, ::sf::Keyboard::Key::NumpadClear)                      \
    X(SDLK_KP_CLEARENTRY, ::sf::Keyboard::Key::NumpadClearEntry)            \
    X(SDLK_KP_BINARY, ::sf::Keyboard::Key::NumpadBinary)                    \
    X(SDLK_KP_OCTAL, ::sf::Keyboard::Key::NumpadOctal)                      \
    X(SDLK_KP_DECIMAL, ::sf::Keyboard::Key::NumpadDecimal)                  \
    X(SDLK_KP_HEXADECIMAL, ::sf::Keyboard::Key::NumpadHexadecimal)          \
    X(SDLK_LCTRL, ::sf::Keyboard::Key::LControl)                            \
    X(SDLK_LSHIFT, ::sf::Keyboard::Key::LShift)                             \
    X(SDLK_LALT, ::sf::Keyboard::Key::LAlt)                                 \
    X(SDLK_LGUI, ::sf::Keyboard::Key::LSystem)                              \
    X(SDLK_RCTRL, ::sf::Keyboard::Key::RControl)                            \
    X(SDLK_RSHIFT, ::sf::Keyboard::Key::RShift)                             \
    X(SDLK_RALT, ::sf::Keyboard::Key::RAlt)                                 \
    X(SDLK_RGUI, ::sf::Keyboard::Key::RSystem)                              \
    X(SDLK_MODE, ::sf::Keyboard::Key::ModeChange)                           \
    X(SDLK_SLEEP, ::sf::Keyboard::Key::Sleep)                               \
    X(SDLK_WAKE, ::sf::Keyboard::Key::Wake)                                 \
    X(SDLK_CHANNEL_INCREMENT, ::sf::Keyboard::Key::ChannelIncrement)        \
    X(SDLK_CHANNEL_DECREMENT, ::sf::Keyboard::Key::ChannelDecrement)        \
    X(SDLK_MEDIA_PLAY, ::sf::Keyboard::Key::MediaPlay)                      \
    X(SDLK_MEDIA_PAUSE, ::sf::Keyboard::Key::MediaPause)                    \
    X(SDLK_MEDIA_RECORD, ::sf::Keyboard::Key::MediaRecord)                  \
    X(SDLK_MEDIA_FAST_FORWARD, ::sf::Keyboard::Key::MediaFastForward)       \
    X(SDLK_MEDIA_REWIND, ::sf::Keyboard::Key::MediaRewind)                  \
    X(SDLK_MEDIA_NEXT_TRACK, ::sf::Keyboard::Key::MediaNextTrack)           \
    X(SDLK_MEDIA_PREVIOUS_TRACK, ::sf::Keyboard::Key::MediaPreviousTrack)   \
    X(SDLK_MEDIA_STOP, ::sf::Keyboard::Key::MediaStop)                      \
    X(SDLK_MEDIA_EJECT, ::sf::Keyboard::Key::MediaEject)                    \
    X(SDLK_MEDIA_PLAY_PAUSE, ::sf::Keyboard::Key::MediaPlayPause)           \
    X(SDLK_MEDIA_SELECT, ::sf::Keyboard::Key::LaunchMediaSelect)            \
    X(SDLK_AC_NEW, ::sf::Keyboard::Key::AcNew)                              \
    X(SDLK_AC_OPEN, ::sf::Keyboard::Key::AcOpen)                            \
    X(SDLK_AC_CLOSE, ::sf::Keyboard::Key::AcClose)                          \
    X(SDLK_AC_EXIT, ::sf::Keyboard::Key::AcExit)                            \
    X(SDLK_AC_SAVE, ::sf::Keyboard::Key::AcSave)                            \
    X(SDLK_AC_PRINT, ::sf::Keyboard::Key::AcPrint)                          \
    X(SDLK_AC_PROPERTIES, ::sf::Keyboard::Key::AcProperties)                \
    X(SDLK_AC_SEARCH, ::sf::Keyboard::Key::Search)                          \
    X(SDLK_AC_HOME, ::sf::Keyboard::Key::HomePage)                          \
    X(SDLK_AC_BACK, ::sf::Keyboard::Key::Back)                              \
    X(SDLK_AC_FORWARD, ::sf::Keyboard::Key::Forward)                        \
    X(SDLK_AC_STOP, ::sf::Keyboard::Key::AcStop)                            \
    X(SDLK_AC_REFRESH, ::sf::Keyboard::Key::Refresh)                        \
    X(SDLK_AC_BOOKMARKS, ::sf::Keyboard::Key::Favorites)                    \
    X(SDLK_SOFTLEFT, ::sf::Keyboard::Key::SoftLeft)                         \
    X(SDLK_SOFTRIGHT, ::sf::Keyboard::Key::SoftRight)                       \
    X(SDLK_CALL, ::sf::Keyboard::Key::Call)                                 \
    X(SDLK_ENDCALL, ::sf::Keyboard::Key::EndCall)                           \
    X(SDLK_LEFT_TAB, ::sf::Keyboard::Key::LTab)                             \
    X(SDLK_LEVEL5_SHIFT, ::sf::Keyboard::Key::Level5Shift)                  \
    X(SDLK_MULTI_KEY_COMPOSE, ::sf::Keyboard::Key::MultiKeyCompose)         \
    X(SDLK_LMETA, ::sf::Keyboard::Key::LMeta)                               \
    X(SDLK_RMETA, ::sf::Keyboard::Key::RMeta)                               \
    X(SDLK_LHYPER, ::sf::Keyboard::Key::LHyper)                             \
    X(SDLK_RHYPER, ::sf::Keyboard::Key::RHyper)


namespace sf::priv
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] Keyboard::Scan mapSDLScancodeToSFML(const SDL_Scancode sdlCode) noexcept
{
    return static_cast<Keyboard::Scan>(sdlCode);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] SDL_Scancode mapSFMLScancodeToSDL(const Keyboard::Scan scanCode) noexcept
{
    return static_cast<SDL_Scancode>(scanCode);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] Keyboard::Key mapSDLKeycodeToSFML(const SDL_Keycode sdlKey) noexcept
{
    switch (sdlKey)
    {
#define X(sdlKey, sfmlKey) \
    case sdlKey:           \
        return sfmlKey;
        SFML_PRIV_SFML_SDL_KEYCODE_MAPPING
#undef X
    }

    return Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] SDL_Keycode mapSFMLKeycodeToSDL(const Keyboard::Key key) noexcept
{
    switch (key)
    {
#define X(sdlKey, sfmlKey) \
    case sfmlKey:          \
        return sdlKey;
        SFML_PRIV_SFML_SDL_KEYCODE_MAPPING
#undef X
    }

    return SDLK_UNKNOWN;
}


////////////////////////////////////////////////////////////
#undef SFML_PRIV_SFML_SDL_KEYCODE_MAPPING


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] SDL_SystemCursor cursorTypeToSDLCursor(const sf::Cursor::Type type) noexcept
{
    // clang-format off
    switch (type)
    {
        case sf::Cursor::Type::Arrow:                  return SDL_SYSTEM_CURSOR_DEFAULT;
        case sf::Cursor::Type::ArrowWait:              return SDL_SYSTEM_CURSOR_PROGRESS;
        case sf::Cursor::Type::Wait:                   return SDL_SYSTEM_CURSOR_WAIT;
        case sf::Cursor::Type::Text:                   return SDL_SYSTEM_CURSOR_TEXT;
        case sf::Cursor::Type::Hand:                   return SDL_SYSTEM_CURSOR_POINTER;
        case sf::Cursor::Type::SizeHorizontal:         return SDL_SYSTEM_CURSOR_EW_RESIZE;
        case sf::Cursor::Type::SizeVertical:           return SDL_SYSTEM_CURSOR_NS_RESIZE;
        case sf::Cursor::Type::SizeTopLeftBottomRight: return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
        case sf::Cursor::Type::SizeBottomLeftTopRight: return SDL_SYSTEM_CURSOR_NESW_RESIZE;
        case sf::Cursor::Type::SizeLeft:               return SDL_SYSTEM_CURSOR_W_RESIZE;
        case sf::Cursor::Type::SizeRight:              return SDL_SYSTEM_CURSOR_E_RESIZE;
        case sf::Cursor::Type::SizeTop:                return SDL_SYSTEM_CURSOR_N_RESIZE;
        case sf::Cursor::Type::SizeBottom:             return SDL_SYSTEM_CURSOR_S_RESIZE;
        case sf::Cursor::Type::SizeTopLeft:            return SDL_SYSTEM_CURSOR_NW_RESIZE;
        case sf::Cursor::Type::SizeBottomRight:        return SDL_SYSTEM_CURSOR_SE_RESIZE;
        case sf::Cursor::Type::SizeBottomLeft:         return SDL_SYSTEM_CURSOR_SW_RESIZE;
        case sf::Cursor::Type::SizeTopRight:           return SDL_SYSTEM_CURSOR_NE_RESIZE;
        case sf::Cursor::Type::SizeAll:                return SDL_SYSTEM_CURSOR_MOVE;
        case sf::Cursor::Type::Cross:                  return SDL_SYSTEM_CURSOR_CROSSHAIR;
        case sf::Cursor::Type::NotAllowed:             return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
        case sf::Cursor::Type::Help:                   break; // not supported
    }
    // clang-format on

    return SDL_SYSTEM_CURSOR_DEFAULT;
}


////////////////////////////////////////////////////////////
[[nodiscard]] SDL_PropertiesID makeSDLWindowPropertiesFromHandle(const WindowHandle handle)
{
    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);

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
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_EMSCRIPTEN_CANVAS_ID, handle);
#endif

    return props;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::pure]] SDL_WindowFlags makeSDLWindowFlagsFromWindowSettings(const WindowSettings& windowSettings) noexcept
{
    SDL_WindowFlags flags{};

    flags |= SDL_WINDOW_OPENGL;

    if (windowSettings.fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;

    if (windowSettings.resizable)
        flags |= SDL_WINDOW_RESIZABLE;

    if (!windowSettings.hasTitlebar)
        flags |= SDL_WINDOW_BORDERLESS;

    return flags;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] Mouse::Button getButtonFromSDLButton(const base::U8 sdlButton) noexcept
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
[[nodiscard, gnu::const]] base::U8 getSDLButtonFromSFMLButton(const Mouse::Button button) noexcept
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
SDLLayer::SDLLayer()
{
    // Ensures window position is synced with DOM position on Emscripten.
    if (!SDL_SetHint(SDL_HINT_VIDEO_SYNC_WINDOW_OPERATIONS, "1"))
        err() << "`SDL_SetHint` failed: " << SDL_GetError();

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        err() << "`SDL_Init` failed: " << SDL_GetError();
        base::abort();
    }
}


////////////////////////////////////////////////////////////
SDLLayer::~SDLLayer()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


////////////////////////////////////////////////////////////
SDLAllocatedArray<SDL_DisplayID> SDLLayer::getDisplays() const
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
SDLAllocatedArray<SDL_DisplayMode*> SDLLayer::getFullscreenDisplayModesForDisplay(const SDL_DisplayID displayId)
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
const SDL_PixelFormatDetails* SDLLayer::getPixelFormatDetails(const SDL_PixelFormat format)
{
    const auto* result = SDL_GetPixelFormatDetails(format);

    if (result == nullptr)
    {
        err() << "`SDL_GetPixelFormatDetails` failed for format " << static_cast<int>(format) << ": " << SDL_GetError();
        return nullptr;
    }

    return result;
}


////////////////////////////////////////////////////////////
const SDL_DisplayMode* SDLLayer::getDesktopDisplayMode(const SDL_DisplayID displayId)
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
SDLAllocatedArray<SDL_TouchID> SDLLayer::getTouchDevices()
{
    int          idCount = 0;
    SDL_TouchID* ids     = SDL_GetTouchDevices(&idCount);

    if (ids == nullptr)
    {
        err() << "`SDL_GetTouchDevices` failed: " << SDL_GetError();
        return nullptr;
    }

    return SDLAllocatedArray<SDL_TouchID>{SDLUPtr<SDL_TouchID>(ids), static_cast<base::SizeT>(idCount)};
}


////////////////////////////////////////////////////////////
SDLAllocatedArray<SDL_Finger*> SDLLayer::getTouchFingers(const SDL_TouchID touchDeviceId)
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
SDL_TouchDeviceType SDLLayer::getTouchDeviceType(const SDL_TouchID touchDeviceId)
{
    if (touchDeviceId == 0)
    {
        err() << "`getTouchDeviceType` failed: invalid touch device ID";
        return SDL_TOUCH_DEVICE_INVALID;
    }

    return SDL_GetTouchDeviceType(touchDeviceId);
}


////////////////////////////////////////////////////////////
const char* SDLLayer::getTouchDeviceName(const SDL_TouchID touchDeviceId)
{
    if (touchDeviceId == 0)
    {
        err() << "`getTouchDeviceName` failed: invalid touch device ID";
        return "INVALID TOUCH DEVICE";
    }

    return SDL_GetTouchDeviceName(touchDeviceId);
}


////////////////////////////////////////////////////////////
Keyboard::Key SDLLayer::localizeScancode(const Keyboard::Scancode code) const noexcept
{
    const SDL_Scancode sdlScancode = mapSFMLScancodeToSDL(code);
    const SDL_Keycode  sdlKey      = SDL_GetKeyFromScancode(sdlScancode, SDL_GetModState(), true);

    return mapSDLKeycodeToSFML(sdlKey);
}


////////////////////////////////////////////////////////////
Keyboard::Scancode SDLLayer::delocalizeScancode(const Keyboard::Key key) const noexcept
{
    const SDL_Keycode sdlKey = mapSFMLKeycodeToSDL(key);

    SDL_Keymod         mod{};
    const SDL_Scancode sdlScancode = SDL_GetScancodeFromKey(sdlKey, &mod);

    return mapSDLScancodeToSFML(sdlScancode);
}


////////////////////////////////////////////////////////////
bool SDLLayer::isKeyPressedByScancode(const Keyboard::Scancode code) const noexcept
{
    const bool* keyboardState = SDL_GetKeyboardState(nullptr);
    SFML_BASE_ASSERT(keyboardState != nullptr);

    return keyboardState[priv::mapSFMLScancodeToSDL(code)];
}


////////////////////////////////////////////////////////////
const char* SDLLayer::getScancodeDescription(const Keyboard::Scancode code) const noexcept
{
    return SDL_GetKeyName(priv::mapSFMLKeycodeToSDL(localize(code)));
}


////////////////////////////////////////////////////////////
Vector2i SDLLayer::getGlobalMousePosition() const noexcept
{
    Vector2f result;
    SDL_GetGlobalMouseState(&result.x, &result.y);
    return result.toVector2i();
}


////////////////////////////////////////////////////////////
bool SDLLayer::setGlobalMousePosition(const Vector2i position) const noexcept
{
    if (!SDL_WarpMouseGlobal(static_cast<float>(position.x), static_cast<float>(position.y)))
    {
        err() << "`SDL_WarpMouseGlobal` failed: " << SDL_GetError();
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
String SDLLayer::getClipboardString() const noexcept
{
    if (!SDL_HasClipboardText())
        return String{};

    char* clipboardText = SDL_GetClipboardText();
    if (SFML_BASE_STRCMP(clipboardText, "") == 0)
    {
        err() << "`SDL_GetClipboardText` failed: " << SDL_GetError();
        return String{};
    }

    SFML_BASE_SCOPE_GUARD({ SDL_free(static_cast<void*>(clipboardText)); });
    return StringUtfUtils::fromUtf8(clipboardText, clipboardText + SFML_BASE_STRLEN(clipboardText));
}


////////////////////////////////////////////////////////////
bool SDLLayer::setClipboardString(const String& string) const noexcept
{
    if (!SDL_SetClipboardText(reinterpret_cast<const char*>(string.toUtf8<std::u8string>().c_str())))
    {
        err() << "`SDL_SetClipboardText` failed: " << SDL_GetError();
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
float SDLLayer::getDisplayContentScale() const
{
    auto displays = getDisplays();
    if (!displays.valid())
    {
        err() << "`getDisplayContentScale` failed: could not get displays";
        return 1.f;
    }

    const SDL_DisplayID primaryDisplayID = displays[0];

    const float result = SDL_GetDisplayContentScale(primaryDisplayID);
    if (result == 0.f)
    {
        err() << "`SDL_GetDisplayContentScale` failed:" << SDL_GetError();
        return 1.f;
    }

    return result;
}


////////////////////////////////////////////////////////////
float SDLLayer::getWindowDisplayScale(SDL_Window& window) const
{
    const float result = SDL_GetWindowDisplayScale(&window);

    if (result == 0.f)
    {
        err() << "`SDL_GetWindowDisplayScale` failed: " << SDL_GetError();
        return 1.f;
    }

    return result;
}


////////////////////////////////////////////////////////////
SDLSurfaceUPtr SDLLayer::createSurfaceFromPixels(Vector2u size, const base::U8* pixels) const
{
    SDL_Surface* surface = SDL_CreateSurfaceFrom(static_cast<int>(size.x),
                                                 static_cast<int>(size.y),
                                                 SDL_PIXELFORMAT_RGBA32,
                                                 const_cast<void*>(static_cast<const void*>(pixels)),
                                                 static_cast<int>(size.x * 4));

    if (surface == nullptr)
    {
        err() << "`SDL_CreateSurfaceFrom` faile: " << SDL_GetError();
        return nullptr;
    }

    return SDLSurfaceUPtr{surface};
}


////////////////////////////////////////////////////////////
unsigned int SDLLayer::getJoystickButtonCount(SDL_Joystick& handle)
{
    const int count = SDL_GetNumJoystickButtons(&handle);

    if (count == -1)
    {
        err() << "`SDL_GetNumJoystickButtons` failed: " << SDL_GetError();
        return 0u;
    }

    return static_cast<unsigned int>(count);
}


////////////////////////////////////////////////////////////
unsigned int SDLLayer::getJoystickAxisCount(SDL_Joystick& handle)
{
    const int count = SDL_GetNumJoystickAxes(&handle);

    if (count == -1)
    {
        err() << "`SDL_GetNumJoystickAxes` failed: " << SDL_GetError();
        return 0u;
    }

    return static_cast<unsigned int>(count);
}


////////////////////////////////////////////////////////////
unsigned int SDLLayer::getJoystickHatCount(SDL_Joystick& handle)
{
    const int count = SDL_GetNumJoystickHats(&handle);

    if (count == -1)
    {
        err() << "`SDL_GetNumJoystickHats` failed: " << SDL_GetError();
        return 0u;
    }

    return static_cast<unsigned int>(count);
}


////////////////////////////////////////////////////////////
[[nodiscard]] const char* SDLLayer::getJoystickName(SDL_Joystick& handle)
{
    const char* name = SDL_GetJoystickName(&handle);

    if (name == nullptr)
    {
        err() << "`SDL_GetJoystickName` failed: " << SDL_GetError();
        return nullptr;
    }

    return name;
}


////////////////////////////////////////////////////////////
[[nodiscard]] unsigned int SDLLayer::getJoystickVendor(SDL_Joystick& handle)
{
    const unsigned int vendor = SDL_GetJoystickVendor(&handle);

    if (vendor == 0u)
    {
        err() << "`SDL_GetJoystickVendor` failed: " << SDL_GetError();
        return 0u;
    }

    return vendor;
}


////////////////////////////////////////////////////////////
[[nodiscard]] unsigned int SDLLayer::getJoystickProduct(SDL_Joystick& handle)
{
    const unsigned int product = SDL_GetJoystickProduct(&handle);

    if (product == 0u)
    {
        err() << "`SDL_GetJoystickProduct` failed: " << SDL_GetError();
        return 0u;
    }

    return product;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool SDLLayer::areGUIDsEqual(const SDL_GUID& a, const SDL_GUID& b)
{
    return SFML_BASE_MEMCMP(&a, &b, sizeof(SDL_GUID)) == 0;
}


////////////////////////////////////////////////////////////
SDLLayer& getSDLLayerSingleton()
{
    static SDLLayer sdlLayer;
    return sdlLayer;
}

} // namespace sf::priv
