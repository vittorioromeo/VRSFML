////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2019 Laurent Gomila (laurent@sfml-dev.org)
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
#ifdef _WIN32_WINDOWS
    #undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#include <SFML/Window/Window.hpp>
#include <SFML/Window/Win32/InputImpl.hpp>
#include <windows.h>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(Keyboard::Key key)
{
    return isKeyPressed(unlocalize(key));
}

////////////////////////////////////////////////////////////
bool InputImpl::isKeyPressed(Keyboard::Scancode code)
{
    auto winCode = sfScanToWin(code);
    auto vkey = MapVirtualKey(winCode, MAPVK_VSC_TO_VK_EX);
    auto state = GetAsyncKeyState(vkey);
    return (state & 0x8000) != 0;
}

////////////////////////////////////////////////////////////
Keyboard::Key InputImpl::localize(Keyboard::Scancode code)
{
    // TODO
    return sf::Keyboard::Unknown;
}

////////////////////////////////////////////////////////////
Keyboard::Scancode InputImpl::unlocalize(Keyboard::Key code)
{
    // TODO
    return sf::Keyboard::ScanUnknown;
}

////////////////////////////////////////////////////////////
String InputImpl::getDescription(Keyboard::Scancode code)
{
    LONG lparam = sfScanToWin(code);
    const int bufSize(1024);
    WCHAR name[bufSize];
    int result = GetKeyNameText(lparam, name, bufSize);
    if (result > 0)
    {
        return name;
    }
    return "Unknown";
}

////////////////////////////////////////////////////////////
void InputImpl::setVirtualKeyboardVisible(bool visible)
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool InputImpl::isMouseButtonPressed(Mouse::Button button)
{
    int vkey = 0;
    switch (button)
    {
        case Mouse::Left:     vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON; break;
        case Mouse::Right:    vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON; break;
        case Mouse::Middle:   vkey = VK_MBUTTON;  break;
        case Mouse::XButton1: vkey = VK_XBUTTON1; break;
        case Mouse::XButton2: vkey = VK_XBUTTON2; break;
        default:              vkey = 0;           break;
    }

    return (GetAsyncKeyState(vkey) & 0x8000) != 0;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition()
{
    POINT point;
    GetCursorPos(&point);
    return Vector2i(point.x, point.y);
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getMousePosition(const WindowBase& relativeTo)
{
    WindowHandle handle = relativeTo.getSystemHandle();
    if (handle)
    {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(handle, &point);
        return Vector2i(point.x, point.y);
    }
    else
    {
        return Vector2i();
    }
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(const Vector2i& position)
{
    SetCursorPos(position.x, position.y);
}


////////////////////////////////////////////////////////////
void InputImpl::setMousePosition(const Vector2i& position, const WindowBase& relativeTo)
{
    WindowHandle handle = relativeTo.getSystemHandle();
    if (handle)
    {
        POINT point = {position.x, position.y};
        ClientToScreen(handle, &point);
        SetCursorPos(point.x, point.y);
    }
}


////////////////////////////////////////////////////////////
bool InputImpl::isTouchDown(unsigned int /*finger*/)
{
    // Not applicable
    return false;
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int /*finger*/)
{
    // Not applicable
    return Vector2i();
}


////////////////////////////////////////////////////////////
Vector2i InputImpl::getTouchPosition(unsigned int /*finger*/, const WindowBase& /*relativeTo*/)
{
    // Not applicable
    return Vector2i();
}

////////////////////////////////////////////////////////////
LONG InputImpl::sfScanToWin(Keyboard::Scancode code)
{
    // Windows scan codes
    // Reference: https://msdn.microsoft.com/en-us/library/aa299374(v=vs.60).aspx
    switch (code)
    {
    case Keyboard::ScanA: return 30;
    case Keyboard::ScanB: return 48;
    case Keyboard::ScanC: return 46;
    case Keyboard::ScanD: return 32;
    case Keyboard::ScanE: return 18;
    case Keyboard::ScanF: return 33;
    case Keyboard::ScanG: return 34;
    case Keyboard::ScanH: return 35;
    case Keyboard::ScanI: return 23;
    case Keyboard::ScanJ: return 36;
    case Keyboard::ScanK: return 37;
    case Keyboard::ScanL: return 38;
    case Keyboard::ScanM: return 50;
    case Keyboard::ScanN: return 49;
    case Keyboard::ScanO: return 24;
    case Keyboard::ScanP: return 25;
    case Keyboard::ScanQ: return 16;
    case Keyboard::ScanR: return 19;
    case Keyboard::ScanS: return 31;
    case Keyboard::ScanT: return 20;
    case Keyboard::ScanU: return 22;
    case Keyboard::ScanV: return 47;
    case Keyboard::ScanW: return 17;
    case Keyboard::ScanX: return 45;
    case Keyboard::ScanY: return 21;
    case Keyboard::ScanZ: return 44;

    case Keyboard::ScanNum1: return 2;
    case Keyboard::ScanNum2: return 3;
    case Keyboard::ScanNum3: return 4;
    case Keyboard::ScanNum4: return 5;
    case Keyboard::ScanNum5: return 6;
    case Keyboard::ScanNum6: return 7;
    case Keyboard::ScanNum7: return 8;
    case Keyboard::ScanNum8: return 9;
    case Keyboard::ScanNum9: return 10;
    case Keyboard::ScanNum0: return 11;

    case Keyboard::ScanEnter: return 28;
    case Keyboard::ScanEscape: return 1;
    case Keyboard::ScanBackspace: return 14;
    case Keyboard::ScanTab: return 15;
    case Keyboard::ScanSpace: return 57;
    case Keyboard::ScanHyphen: return 12;
    case Keyboard::ScanEquals: return 13;
    case Keyboard::ScanLBracket: return 26;
    case Keyboard::ScanRBracket: return 27;
    case Keyboard::ScanBackslash: return 43;
    case Keyboard::ScanDash: return 41;
    case Keyboard::ScanSemicolon: return 39;
    case Keyboard::ScanQuote: return 40;
    //case Keyboard::ScanGraveAccent: return ? ? ? 
    case Keyboard::ScanComma: return 51;
    case Keyboard::ScanPeriod: return 52;
    case Keyboard::ScanSlash: return 53;

    case Keyboard::ScanF1: return 59;
    case Keyboard::ScanF2: return 60;
    case Keyboard::ScanF3: return 61;
    case Keyboard::ScanF4: return 62;
    case Keyboard::ScanF5: return 63;
    case Keyboard::ScanF6: return 64;
    case Keyboard::ScanF7: return 65;
    case Keyboard::ScanF8: return 66;
    case Keyboard::ScanF9: return 67;
    case Keyboard::ScanF10: return 68;
    case Keyboard::ScanF11: return MAKELONG(87, KF_EXTENDED);
    case Keyboard::ScanF12: return MAKELONG(88, KF_EXTENDED);
    //case Keyboard::ScanF13: return ???
    //case Keyboard::ScanF14: return ???
    //case Keyboard::ScanF15: return ???

    case Keyboard::ScanCapsLock: return 58;
    case Keyboard::ScanPrintScreen: return 55;
    case Keyboard::ScanScrollLock: return 70;
    //case Keyboard::ScanPause: return ???;
    case Keyboard::ScanInsert: return 82;
    case Keyboard::ScanHome: return 71;
    case Keyboard::ScanPageUp: return 73;
    case Keyboard::ScanDelete: return 83;
    case Keyboard::ScanEnd: return 79;
    case Keyboard::ScanPageDown: return 81;
    case Keyboard::ScanRight: return 77;
    case Keyboard::ScanLeft: return 75;
    case Keyboard::ScanDown: return 80;
    case Keyboard::ScanUp: return 72;
    case Keyboard::ScanNumLock: return 69;
    case Keyboard::ScanDivide: return 53;
    //case Keyboard::ScanMultiply: return ??? ;
    case Keyboard::ScanMinus: return 74;
    case Keyboard::ScanPlus: return 78;
    //case Keyboard::ScanPadEquals: return ???;
    case Keyboard::ScanNumpadEnter: return MAKELONG(28, KF_EXTENDED);
    case Keyboard::ScanDecimal: return MAKELONG(83, KF_EXTENDED);

    // Num pad keys except 5 have the extended bit (0x100) set
    case Keyboard::ScanNumpad1: return MAKELONG(79, KF_EXTENDED);
    case Keyboard::ScanNumpad2: return MAKELONG(80, KF_EXTENDED);
    case Keyboard::ScanNumpad3: return MAKELONG(81, KF_EXTENDED);
    case Keyboard::ScanNumpad4: return MAKELONG(75, KF_EXTENDED);
    case Keyboard::ScanNumpad5: return 76;
    case Keyboard::ScanNumpad6: return MAKELONG(77, KF_EXTENDED);
    case Keyboard::ScanNumpad7: return MAKELONG(71, KF_EXTENDED);
    case Keyboard::ScanNumpad8: return MAKELONG(72, KF_EXTENDED);
    case Keyboard::ScanNumpad9: return MAKELONG(73, KF_EXTENDED);
    case Keyboard::ScanNumpad0: return MAKELONG(82, KF_EXTENDED);

    //case Keyboard::ScanReverseSolidus: return ? ? ? ;
    //case Keyboard::ScanApplication: return ? ? ? ;
    //case Keyboard::ScanExecute: return ? ? ? ;
    //case Keyboard::ScanHelp: return ? ? ? ;
    //case Keyboard::ScanMenu: return ? ? ? ;
    //case Keyboard::ScanSelect: return ? ? ? ;
    //case Keyboard::ScanStop: return ? ? ? ;
    //case Keyboard::ScanAgain: return ? ? ? ;
    //case Keyboard::ScanUndo: return ? ? ? ;
    //case Keyboard::ScanCut: return ? ? ? ;
    //case Keyboard::ScanCopy: return ? ? ? ;
    //case Keyboard::ScanPaste: return ? ? ? ;
    //case Keyboard::ScanFind: return ? ? ? ;
    //case Keyboard::ScanMute: return ? ? ? ;
    //case Keyboard::ScanVolumeUp: return ? ? ? ;
    //case Keyboard::ScanVolumeDown: return ? ? ? ;
    case Keyboard::ScanLControl: return 29;
    case Keyboard::ScanLShift: return 42;
    case Keyboard::ScanLAlt: return 56;
    //case Keyboard::ScanLSystem: return ? ? ? ;
    case Keyboard::ScanRControl: return MAKELONG(29 ,KF_EXTENDED);
    case Keyboard::ScanRShift: return 54;
    case Keyboard::ScanRAlt: return 56;
    //case Keyboard::ScanRSystem: return ? ? ? ;

    default: return 0; // Not sure what to return here?
    }
}

} // namespace priv

} // namespace sf
