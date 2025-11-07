#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class UnicodeString;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Give access to the real-time state of the keyboard
///
////////////////////////////////////////////////////////////
namespace Keyboard
{
////////////////////////////////////////////////////////////
/// \brief Key codes
///
/// The enumerators refer to the "localized" key; i.e. depending
/// on the layout set by the operating system, a key can be mapped
/// to `Y` or `Z`.
///
////////////////////////////////////////////////////////////
enum class Key
{
    Unknown = -1,
    A       = 0,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Escape,
    LControl,
    LShift,
    LAlt,
    LSystem,
    RControl,
    RShift,
    RAlt,
    RSystem,
    Menu,
    LBracket,
    RBracket,
    Semicolon,
    Comma,
    Period,
    Apostrophe,
    Slash,
    Backslash,
    Grave,
    Equal,
    Hyphen,
    Space,
    Enter,
    Backspace,
    Tab,
    PageUp,
    PageDown,
    End,
    Home,
    Insert,
    Delete,
    Add,
    Subtract,
    Multiply,
    Divide,
    Left,
    Right,
    Up,
    Down,
    Numpad0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    Pause, // Until here should be compatible with SFML

    // From SDL3
    Exclaim,
    DoubleApostrophe,
    Hash,
    Dollar,
    Percent,
    Ampersand,
    LParenthesis,
    RParenthesis,
    Asterisk,
    Plus,
    Colon,
    Less,
    Greater,
    Question,
    At,
    Caret,
    Underscore,
    LBrace,
    Pipe,
    RBrace,
    Tilde,
    PlusMinus,
    CapsLock,
    PrintScreen,
    ScrollLock,
    NumLock,
    NumpadEnter,
    NumpadPeriod,
    Application,
    Power,
    NumpadEqual,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    Execute,
    Help,
    Select,
    Stop,
    Redo,
    Undo,
    Cut,
    Copy,
    Paste,
    Find,
    VolumeMute,
    VolumeUp,
    VolumeDown,
    NumpadComma,
    NumpadEqualAs400,
    AltErase,
    SysReq,
    Cancel,
    Clear,
    Prior,
    Enter2,
    Separator,
    Out,
    Oper,
    ClearAgain,
    CrSel,
    ExSel,
    Numpad00,
    Numpad000,
    ThousandsSeparator,
    DecimalSeparator,
    CurrencyUnit,
    CurrencySubUnit,
    NumpadLParenthesis,
    NumpadRParenthesis,
    NumpadLBrace,
    NumpadRBrace,
    NumpadTab,
    NumpadBackspace,
    NumpadA,
    NumpadB,
    NumpadC,
    NumpadD,
    NumpadE,
    NumpadF,
    NumpadXor,
    NumpadPower,
    NumpadPercent,
    NumpadLess,
    NumpadGreater,
    NumpadAmpersand,
    NumpadDoubleAmpersand,
    NumpadVerticalBar,
    NumpadDoubleVerticalBar,
    NumpadColon,
    NumpadHash,
    NumpadSpace,
    NumpadAt,
    NumpadExclamation,
    NumpadMemStore,
    NumpadMemRecall,
    NumpadMemClear,
    NumpadMemAdd,
    NumpadMemSubtract,
    NumpadMemMultiply,
    NumpadMemDivide,
    NumpadPlusMinus,
    NumpadClear,
    NumpadClearEntry,
    NumpadBinary,
    NumpadOctal,
    NumpadDecimal,
    NumpadHexadecimal,
    ModeChange,
    Sleep,
    Wake,
    ChannelIncrement,
    ChannelDecrement,
    MediaPlay,
    MediaPause,
    MediaRecord,
    MediaFastForward,
    MediaRewind,
    MediaNextTrack,
    MediaPreviousTrack,
    MediaStop,
    MediaEject,
    MediaPlayPause,
    LaunchMediaSelect,
    AcNew,
    AcOpen,
    AcClose,
    AcExit,
    AcSave,
    AcPrint,
    AcProperties,
    Search,
    HomePage,
    Back,
    Forward,
    AcStop,
    Refresh,
    Favorites,
    SoftLeft,
    SoftRight,
    Call,
    EndCall,
    LTab,
    Level5Shift,
    MultiKeyCompose,
    LMeta,
    RMeta,
    LHyper,
    RHyper,
};

////////////////////////////////////////////////////////////
enum : unsigned int
{
    KeyCount = static_cast<unsigned int>(Key::RHyper) + 1u //!< Total number of keyboard keys, ignoring `Key::Unknown`
};

////////////////////////////////////////////////////////////
/// \brief Scancodes
///
/// The enumerators are bound to a physical key and do not depend on
/// the keyboard layout used by the operating system. Usually, the AT-101
/// keyboard can be used as reference for the physical position of the keys.
///
////////////////////////////////////////////////////////////
enum class Scan
{
    Unknown = 0,

    A = 4,
    B = 5,
    C = 6,
    D = 7,
    E = 8,
    F = 9,
    G = 10,
    H = 11,
    I = 12,
    J = 13,
    K = 14,
    L = 15,
    M = 16,
    N = 17,
    O = 18,
    P = 19,
    Q = 20,
    R = 21,
    S = 22,
    T = 23,
    U = 24,
    V = 25,
    W = 26,
    X = 27,
    Y = 28,
    Z = 29,

    Num1 = 30,
    Num2 = 31,
    Num3 = 32,
    Num4 = 33,
    Num5 = 34,
    Num6 = 35,
    Num7 = 36,
    Num8 = 37,
    Num9 = 38,
    Num0 = 39,

    Enter     = 40,
    Escape    = 41,
    Backspace = 42,
    Tab       = 43,
    Space     = 44,

    Hyphen     = 45,
    Equal      = 46,
    LBracket   = 47,
    RBracket   = 48,
    Backslash  = 49,
    NonUsHash  = 50,
    Semicolon  = 51,
    Apostrophe = 52,
    Grave      = 53,
    Comma      = 54,
    Period     = 55,
    Slash      = 56,

    CapsLock = 57,

    F1  = 58,
    F2  = 59,
    F3  = 60,
    F4  = 61,
    F5  = 62,
    F6  = 63,
    F7  = 64,
    F8  = 65,
    F9  = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,

    PrintScreen = 70,
    ScrollLock  = 71,
    Pause       = 72,
    Insert      = 73,
    Home        = 74,
    PageUp      = 75,
    Delete      = 76,
    End         = 77,
    PageDown    = 78,
    Right       = 79,
    Left        = 80,
    Down        = 81,
    Up          = 82,

    NumLock        = 83,
    NumpadDivide   = 84,
    NumpadMultiply = 85,
    NumpadMinus    = 86,
    NumpadPlus     = 87,
    NumpadEnter    = 88,
    Numpad1        = 89,
    Numpad2        = 90,
    Numpad3        = 91,
    Numpad4        = 92,
    Numpad5        = 93,
    Numpad6        = 94,
    Numpad7        = 95,
    Numpad8        = 96,
    Numpad9        = 97,
    Numpad0        = 98,
    NumpadPeriod   = 99,

    NonUsBackslash   = 100,
    Application      = 101,
    Power            = 102,
    NumpadEqual      = 103,
    F13              = 104,
    F14              = 105,
    F15              = 106,
    F16              = 107,
    F17              = 108,
    F18              = 109,
    F19              = 110,
    F20              = 111,
    F21              = 112,
    F22              = 113,
    F23              = 114,
    F24              = 115,
    Execute          = 116,
    Help             = 117,
    Menu             = 118,
    Select           = 119,
    Stop             = 120,
    Redo             = 121,
    Undo             = 122,
    Cut              = 123,
    Copy             = 124,
    Paste            = 125,
    Find             = 126,
    VolumeMute       = 127,
    VolumeUp         = 128,
    VolumeDown       = 129,
    NumpadComma      = 133,
    NumpadEqualas400 = 134,

    International1 = 135,
    International2 = 136,
    International3 = 137,
    International4 = 138,
    International5 = 139,
    International6 = 140,
    International7 = 141,
    International8 = 142,
    International9 = 143,
    Lang1          = 144,
    Lang2          = 145,
    Lang3          = 146,
    Lang4          = 147,
    Lang5          = 148,
    Lang6          = 149,
    Lang7          = 150,
    Lang8          = 151,
    Lang9          = 152,

    AltErase   = 153,
    SysReq     = 154,
    Cancel     = 155,
    Clear      = 156,
    Prior      = 157,
    Enter2     = 158,
    Separator  = 159,
    Out        = 160,
    Oper       = 161,
    ClearAgain = 162,
    CrSel      = 163,
    ExSel      = 164,

    Numpad00                = 176,
    Numpad000               = 177,
    ThousandsSeparator      = 178,
    DecimalsSeparator       = 179,
    CurrencyUnit            = 180,
    CurrencySubUnit         = 181,
    NumpadLParenthesis      = 182,
    NumpadRParenthesis      = 183,
    NumpadLBrace            = 184,
    NumpadRBrace            = 185,
    NumpadTab               = 186,
    NumpadBackspace         = 187,
    NumpadA                 = 188,
    NumpadB                 = 189,
    NumpadC                 = 190,
    NumpadD                 = 191,
    NumpadE                 = 192,
    NumpadF                 = 193,
    NumpadXor               = 194,
    NumpadPower             = 195,
    NumpadPercent           = 196,
    NumpadLess              = 197,
    NumpadGreater           = 198,
    NumpadAmpersand         = 199,
    NumpadDoubleAmpersand   = 200,
    NumpadVerticalBar       = 201,
    NumpadDoubleVerticalBar = 202,
    NumpadColon             = 203,
    NumpadHash              = 204,
    NumpadSpace             = 205,
    NumpadAt                = 206,
    NumpadExclamation       = 207,
    NumpadMemStore          = 208,
    NumpadMemRecall         = 209,
    NumpadMemClear          = 210,
    NumpadMemAdd            = 211,
    NumpadMemSubtract       = 212,
    NumpadMemMultiply       = 213,
    NumpadMemDivide         = 214,
    NumpadPlusMinus         = 215,
    NumpadClear             = 216,
    NumpadClearEntry        = 217,
    NumpadBinary            = 218,
    NumpadOctal             = 219,
    NumpadDecimal           = 220,
    NumpadHexadecimal       = 221,

    LControl = 224,
    LShift   = 225,
    LAlt     = 226,
    LSystem  = 227,
    RControl = 228,
    RShift   = 229,
    RAlt     = 230,
    RSystem  = 231,

    ModeChange = 257,
    Sleep      = 258,
    Wake       = 259,

    ChannelIncrement = 260,
    ChannelDecrement = 261,

    MediaPlay          = 262,
    MediaPause         = 263,
    MediaRecord        = 264,
    MediaFastForward   = 265,
    MediaRewind        = 266,
    MediaNextTrack     = 267,
    MediaPreviousTrack = 268,
    MediaStop          = 269,
    MediaEject         = 270,
    MediaPlayPause     = 271,
    LaunchMediaSelect  = 272,

    AcNew        = 273,
    AcOpen       = 274,
    AcClose      = 275,
    AcExit       = 276,
    AcSave       = 277,
    AcPrint      = 278,
    AcProperties = 279,

    Search    = 280,
    HomePage  = 281,
    Back      = 282,
    Forward   = 283,
    AcStop    = 284,
    Refresh   = 285,
    Favorites = 286,

    SoftLeft  = 287,
    SoftRight = 288,
    Call      = 289,
    EndCall   = 290,
};

using Scancode = Scan;

////////////////////////////////////////////////////////////
enum : unsigned int
{
    ScancodeCount = static_cast<unsigned int>(Scan::EndCall) + 1u
};

////////////////////////////////////////////////////////////
/// \brief Check if a key is pressed
///
/// \warning On macOS you're required to grant input monitoring access for
///          your application in order for `isKeyPressed` to work.
///
/// \param key Key to check
///
/// \return `true` if the key is pressed, `false` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool isKeyPressed(Key key);

////////////////////////////////////////////////////////////
/// \brief Check if a key is pressed
///
/// \warning On macOS you're required to grant input monitoring access for
///          your application in order for `isKeyPressed` to work.
///
/// \param code Scancode to check
///
/// \return `true` if the physical key is pressed, `false` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool isKeyPressed(Scancode code);

////////////////////////////////////////////////////////////
/// \brief Localize a physical key to a logical one
///
/// \param code Scancode to localize
///
/// \return The key corresponding to the scancode under the current
///         keyboard layout used by the operating system, or
///         `sf::Keyboard::Key::Unknown` when the scancode cannot be mapped
///         to a Key.
///
/// \see `delocalize`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API Key localize(Scancode code);

////////////////////////////////////////////////////////////
/// \brief Identify the physical key corresponding to a logical one
///
/// \param key Key to "delocalize"
///
/// \return The scancode corresponding to the key under the current
///         keyboard layout used by the operating system, or
///         `sf::Keyboard::Scan::Unknown` when the key cannot be mapped
///         to a `sf::Keyboard::Scancode`.
///
/// \see `localize`
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API Scancode delocalize(Key key);

////////////////////////////////////////////////////////////
/// \brief Provide a string representation for a given scancode
///
/// The returned string is a short, non-technical description of
/// the key represented with the given scancode. Most effectively
/// used in user interfaces, as the description for the key takes
/// the users keyboard layout into consideration.
///
/// \warning The result is OS-dependent: for example, `sf::Keyboard::Scan::LSystem`
///          is "Left Meta" on Linux, "Left Windows" on Windows and
///          "Left Command" on macOS.
///
/// The current keyboard layout set by the operating system is used to
/// interpret the scancode: for example, `sf::Keyboard::Key::Semicolon` is
/// mapped to ";" for layout and to "é" for others.
///
/// \param code Scancode to check
///
/// \return The localized description of the code
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API UnicodeString getDescription(Scancode code);

////////////////////////////////////////////////////////////
/// \brief Show or hide the virtual keyboard
///
/// \warning The virtual keyboard is not supported on all
///          systems. It will typically be implemented on mobile OSes
///          (Android, iOS) but not on desktop OSes (Windows, Linux, etc...).
///
/// If the virtual keyboard is not available, this function does
/// nothing.
///
/// \param visible `true` to show, `false` to hide
///
////////////////////////////////////////////////////////////
SFML_WINDOW_API void setVirtualKeyboardVisible(bool visible);
} // namespace Keyboard

} // namespace sf


////////////////////////////////////////////////////////////
/// \namespace sf::Keyboard
/// \ingroup window
///
/// `sf::Keyboard` provides an interface to the state of the
/// keyboard.
///
/// This namespace allows users to query the keyboard state at any
/// time and directly, without having to deal with a window and
/// its events. Compared to the `KeyPressed` and `KeyReleased` events,
/// `sf::Keyboard` can retrieve the state of a key at any time
/// (you don't need to store and update a boolean on your side
/// in order to know if a key is pressed or released), and you
/// always get the real state of the keyboard, even if keys are
/// pressed or released when your window is out of focus and no
/// event is triggered.
///
/// Usage example:
/// \code
/// if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
/// {
///     // move left...
/// }
/// else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
/// {
///     // move right...
/// }
/// else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
/// {
///     // quit...
/// }
/// else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Grave))
/// {
///     // open in-game command line (if it's not already open)
/// }
/// \endcode
///
/// \see `sf::Joystick`, `sf::Mouse`, `sf::Touch`
///
////////////////////////////////////////////////////////////
