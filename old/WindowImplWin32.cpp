#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/CursorImpl.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/Win32/WindowImplWin32.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowImpl.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/Utf.hpp"
#include "SFML/System/Win32/WindowsHeader.hpp"

// dbt.h is lowercase here, as a cross-compile on linux with mingw-w64
// expects lowercase, and a native compile on windows, whether via msvc
// or mingw-w64 addresses files in a case insensitive manner.
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TrivialVector.hpp"

#include <_mingw_stat64.h>
#include <dbt.h>
#include <string>
#include <unordered_map>
#include <winuser.h>

// MinGW lacks the definition of some Win32 constants
#ifndef XBUTTON1
    #define XBUTTON1 0x00'01
#endif
#ifndef XBUTTON2
    #define XBUTTON2 0x00'02
#endif
#ifndef WM_MOUSEHWHEEL
    #define WM_MOUSEHWHEEL 0x02'0E
#endif
#ifndef MAPVK_VK_TO_VSC
    #define MAPVK_VK_TO_VSC (0)
#endif

namespace
{
unsigned int               windowCount      = 0; // Windows owned by SFML
unsigned int               handleCount      = 0; // All window handles
const wchar_t*             className        = L"SFML_Window";
sf::priv::WindowImplWin32* fullscreenWindow = nullptr;

constexpr GUID guidDevinterfaceHid = {0x4d'1e'55'b2, 0xf1'6f, 0x11'cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};


////////////////////////////////////////////////////////////
void setProcessDpiAware()
{
    // Try SetProcessDpiAwareness first
    if (const HINSTANCE shCoreDll = LoadLibraryW(L"Shcore.dll"))
    {
        enum ProcessDpiAwareness
        {
            ProcessDpiUnaware         = 0,
            ProcessSystemDpiAware     = 1,
            ProcessPerMonitorDpiAware = 2
        };

        using SetProcessDpiAwarenessFuncType = HRESULT(WINAPI*)(ProcessDpiAwareness);
        auto setProcessDpiAwarenessFunc      = reinterpret_cast<SetProcessDpiAwarenessFuncType>(
            reinterpret_cast<void*>(GetProcAddress(shCoreDll, "SetProcessDpiAwareness")));

        if (setProcessDpiAwarenessFunc)
        {
            // We only check for E_INVALIDARG because we would get
            // E_ACCESSDENIED if the DPI was already set previously
            // and S_OK means the call was successful.
            // We intentionally don't use Per Monitor V2 which can be
            // enabled with SetProcessDpiAwarenessContext, because that
            // would scale the title bar and thus change window size
            // by default when moving the window between monitors.
            if (setProcessDpiAwarenessFunc(ProcessSystemDpiAware) == E_INVALIDARG)
            {
                sf::priv::err() << "Failed to set process DPI awareness";
            }
            else
            {
                FreeLibrary(shCoreDll);
                return;
            }
        }

        FreeLibrary(shCoreDll);
    }

    // Fall back to SetProcessDPIAware if SetProcessDpiAwareness
    // is not available on this system
    if (const HINSTANCE user32Dll = LoadLibraryW(L"user32.dll"))
    {
        using SetProcessDPIAwareFuncType = BOOL(WINAPI*)();
        auto setProcessDPIAwareFunc      = reinterpret_cast<SetProcessDPIAwareFuncType>(
            reinterpret_cast<void*>(GetProcAddress(user32Dll, "SetProcessDPIAware")));

        if (setProcessDPIAwareFunc)
        {
            if (!setProcessDPIAwareFunc())
                sf::priv::err() << "Failed to set process DPI awareness";
        }

        FreeLibrary(user32Dll);
    }
}


////////////////////////////////////////////////////////////
float getScalingFactor(HWND hwnd)
{
    // Try to use GetDpiForWindow from user32.dll (Windows 10, version 1607+)
    using GetDpiForWindowFunc = UINT(WINAPI*)(HWND);

    if (HMODULE hUser32 = GetModuleHandleW(L"user32.dll"))
    {
        auto pGetDpiForWindow = reinterpret_cast<GetDpiForWindowFunc>(
            reinterpret_cast<void*>(GetProcAddress(hUser32, "GetDpiForWindow")));

        if (pGetDpiForWindow)
        {
            const UINT dpi = pGetDpiForWindow(hwnd);
            return static_cast<float>(dpi) / 96.f;
        }
    }

    // Fallback: use GetDeviceCaps on the window's device context
    if (HDC hdc = GetDC(hwnd))
    {
        const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(hwnd, hdc);

        return static_cast<float>(dpiX) / 96.f;
    }

    // If all else fails, default to 1.0 (i.e., 96 DPI)
    return 1.f;
}


////////////////////////////////////////////////////////////
// Register a RAWINPUTDEVICE representing the mouse to receive raw
// mouse deltas using WM_INPUT
void initRawMouse(HWND handle)
{
    const RAWINPUTDEVICE rawMouse{0x01, 0x02, 0, nullptr}; // HID usage: mouse device class, no flags, follow keyboard focus

    if (RegisterRawInputDevices(&rawMouse, 1, sizeof(rawMouse)) != TRUE)
        sf::priv::err() << "Failed to initialize raw mouse input";

    if (RegisterTouchWindow(handle, TWF_WANTPALM) != TRUE)
        sf::priv::err() << "Failed to initialize touch input";

    using SetWindowFeedbackSettingType = BOOL(WINAPI*)(HWND, FEEDBACK_TYPE, DWORD, UINT, PVOID);

    if (HMODULE hUser32 = GetModuleHandleW(L"user32.dll"))
    {
        if (auto pSetWindowFeedbackSetting = reinterpret_cast<SetWindowFeedbackSettingType>(
                reinterpret_cast<void*>(GetProcAddress(hUser32, "SetWindowFeedbackSetting"))))
        {
            ULONG contactVisualization = TOUCH_FEEDBACK_NONE;
            if (pSetWindowFeedbackSetting(handle, FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, sizeof(ULONG), &contactVisualization) !=
                TRUE)
            {
                sf::priv::err() << "Failed to disable touch gestures";
            }
        }
    }
}

// TODO P0:
struct TouchInfo
{
    unsigned int     normalizedIndex;
    sf::Vector2i     position;
    sf::WindowHandle handle;
};

bool                                  touchIndexPool[10]{};
std::unordered_map<UINT32, TouchInfo> touchMap;

} // namespace


namespace sf::priv::InputImpl
{
////////////////////////////////////////////////////////////
bool isTouchDown(unsigned int finger)
{
    for (const auto& [index, info] : touchMap)
    {
        if (info.normalizedIndex == finger)
            return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger)
{
    for (const auto& [index, info] : touchMap)
    {
        if (info.normalizedIndex == finger)
            return info.position;
    }

    return {};
}


////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger, const WindowBase& window)
{
    // TODO P0: is window used correctly?

    for (const auto& [index, info] : touchMap)
    {
        if (info.normalizedIndex == finger && info.handle == window.getNativeHandle())
            return info.position;
    }

    return {};
}

} // namespace sf::priv::InputImpl

namespace sf::priv
{
////////////////////////////////////////////////////////////
WindowImplWin32::WindowImplWin32(WindowHandle handle) : m_handle(handle)
{
    // Set that this process is DPI aware and can handle DPI scaling
    setProcessDpiAware();

    if (m_handle)
    {
        // If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
        if (handleCount == 0)
        {
            initRawMouse(m_handle);
        }

        ++handleCount;

        // We change the event procedure of the control (it is important to save the old one)
        SetWindowLongPtrW(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        m_callback = SetWindowLongPtrW(m_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowImplWin32::globalOnEvent));
    }
}


////////////////////////////////////////////////////////////
WindowImplWin32::WindowImplWin32(const WindowSettings& windowSettings) :
m_lastSize(windowSettings.size),
m_fullscreen(windowSettings.fullscreen),
m_cursorGrabbed(m_fullscreen)
{
    // Set that this process is DPI aware and can handle DPI scaling
    setProcessDpiAware();

    // Register the window class at first call
    if (windowCount == 0)
        registerWindowClass();

    // Compute position and size
    HDC       screenDC   = GetDC(nullptr);
    const int left       = (GetDeviceCaps(screenDC, HORZRES) - static_cast<int>(windowSettings.size.x)) / 2;
    const int top        = (GetDeviceCaps(screenDC, VERTRES) - static_cast<int>(windowSettings.size.y)) / 2;
    auto [width, height] = windowSettings.size.toVector2i();
    ReleaseDC(nullptr, screenDC);

    // Choose the window style according to the Style parameter
    DWORD win32Style = WS_VISIBLE;

    if (windowSettings.hasTitlebar)
        win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
    if (windowSettings.resizable)
        win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    if (windowSettings.closable)
        win32Style |= WS_SYSMENU;


    // In windowed mode, adjust width and height so that window will have the requested client area
    if (!m_fullscreen)
    {
        RECT rectangle = {0, 0, width, height};
        AdjustWindowRect(&rectangle, win32Style, false);
        width  = rectangle.right - rectangle.left;
        height = rectangle.bottom - rectangle.top;
    }

    // Create the window
    m_handle = CreateWindowW(className,
                             reinterpret_cast<const wchar_t*>(windowSettings.title.toUtf16<std::u16string>().c_str()),
                             win32Style,
                             left,
                             top,
                             width,
                             height,
                             nullptr,
                             nullptr,
                             GetModuleHandle(nullptr),
                             this);

    // Register to receive device interface change notifications (used for joystick connection handling)
    DEV_BROADCAST_DEVICEINTERFACE deviceInterface =
        {sizeof(DEV_BROADCAST_DEVICEINTERFACE), DBT_DEVTYP_DEVICEINTERFACE, 0, guidDevinterfaceHid, {0}};
    RegisterDeviceNotification(m_handle, &deviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

    // If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
    if (m_handle)
    {
        if (handleCount == 0)
        {
            initRawMouse(m_handle);
        }

        if (!windowSettings.hasTitlebar) // TODO P0: review
        {
            SetWindowLongPtr(m_handle, GWL_STYLE, WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
            SetWindowLongPtr(m_handle, GWL_EXSTYLE, WS_EX_APPWINDOW);

            SetWindowPos(m_handle, HWND_TOP, left, top, width, height, SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
        }

        ++handleCount;
    }

    // By default, the OS limits the size of the window the the desktop size,
    // we have to resize it after creation to apply the real size
    setSize(windowSettings.size);

    // Switch to fullscreen if requested
    if (m_fullscreen)
        switchToFullscreen(VideoMode{windowSettings.size, windowSettings.bitsPerPixel});

    // Increment window count
    ++windowCount;
}


////////////////////////////////////////////////////////////
WindowImplWin32::~WindowImplWin32()
{
    // TODO P2: should we restore the cursor shape and visibility?

    // Destroy the custom icon, if any
    if (m_icon)
        DestroyIcon(m_icon);

    // If it's the last window handle we have to poll for joysticks again
    if (m_handle)
    {
        --handleCount;
    }

    if (!m_callback)
    {
        // Destroy the window
        if (m_handle)
            DestroyWindow(m_handle);

        // Decrement the window count
        --windowCount;

        // Unregister window class if we were the last window
        if (windowCount == 0)
            UnregisterClassW(className, GetModuleHandleW(nullptr));
    }
    else
    {
        // The window is external: remove the hook on its message callback
        SetWindowLongPtrW(m_handle, GWLP_WNDPROC, m_callback);
    }
}


////////////////////////////////////////////////////////////
WindowHandle WindowImplWin32::getNativeHandle() const
{
    return m_handle;
}


////////////////////////////////////////////////////////////
void WindowImplWin32::processEvents()
{
    // We process the window events only if we own it
    if (!m_callback)
    {
        MSG message;
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
}


////////////////////////////////////////////////////////////
Vector2i WindowImplWin32::getPosition() const
{
    RECT rect;
    GetWindowRect(m_handle, &rect);

    return {rect.left, rect.top};
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setPosition(Vector2i position)
{
    SetWindowPos(m_handle, nullptr, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    if (m_cursorGrabbed)
        grabCursor(true);
}


////////////////////////////////////////////////////////////
Vector2u WindowImplWin32::getSize() const
{
    RECT rect;
    GetClientRect(m_handle, &rect);

    return Vector2<LONG>(rect.right - rect.left, rect.bottom - rect.top).toVector2u();
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setSize(Vector2u size)
{
    const auto [width, height] = contentSizeToWindowSize(size);
    SetWindowPos(m_handle, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setTitle(const String& title)
{
    SetWindowTextW(m_handle, reinterpret_cast<const wchar_t*>(title.toUtf16<std::u16string>().c_str()));
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setIcon(Vector2u size, const base::U8* pixels)
{
    // First destroy the previous one
    if (m_icon)
        DestroyIcon(m_icon);

    // Windows wants BGRA pixels: swap red and blue channels
    base::TrivialVector<base::U8> iconPixels(size.x * size.y * 4);
    for (base::SizeT i = 0; i < iconPixels.size() / 4; ++i)
    {
        iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
        iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
        iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
        iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
    }

    // Create the icon from the pixel array
    m_icon = CreateIcon(GetModuleHandleW(nullptr),
                        static_cast<int>(size.x),
                        static_cast<int>(size.y),
                        1,
                        32,
                        nullptr,
                        iconPixels.data());

    // Set it as both big and small icon of the window
    if (m_icon)
    {
        SendMessageW(m_handle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(m_icon));
        SendMessageW(m_handle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_icon));
    }
    else
    {
        priv::err() << "Failed to set the window's icon";
    }
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setVisible(bool visible)
{
    ShowWindow(m_handle, visible ? SW_SHOW : SW_HIDE);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setMouseCursorVisible(bool visible)
{
    m_cursorVisible = visible;
    SetCursor(m_cursorVisible ? m_lastCursor : nullptr);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setMouseCursorGrabbed(bool grabbed)
{
    m_cursorGrabbed = grabbed;
    grabCursor(m_cursorGrabbed);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setMouseCursor(void* cursor)
{
    m_lastCursor = static_cast<HCURSOR>(cursor);
    SetCursor(m_cursorVisible ? m_lastCursor : nullptr);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setKeyRepeatEnabled(bool enabled)
{
    m_keyRepeatEnabled = enabled;
}


////////////////////////////////////////////////////////////
void WindowImplWin32::requestFocus()
{
    // Allow focus stealing only within the same process; compare PIDs of current and foreground window
    DWORD thisPid       = 0;
    DWORD foregroundPid = 0;
    GetWindowThreadProcessId(m_handle, &thisPid);
    GetWindowThreadProcessId(GetForegroundWindow(), &foregroundPid);

    if (thisPid == foregroundPid)
    {
        // The window requesting focus belongs to the same process as the current window: steal focus
        SetForegroundWindow(m_handle);
    }
    else
    {
        // Different process: don't steal focus, but create a taskbar notification ("flash")
        FLASHWINFO info;
        info.cbSize    = sizeof(info);
        info.hwnd      = m_handle;
        info.dwFlags   = FLASHW_TRAY;
        info.dwTimeout = 0;
        info.uCount    = 3;

        FlashWindowEx(&info);
    }
}


////////////////////////////////////////////////////////////
bool WindowImplWin32::hasFocus() const
{
    return m_handle == GetForegroundWindow();
}


////////////////////////////////////////////////////////////
float WindowImplWin32::getDPIAwareScalingFactor() const
{
    return getScalingFactor(m_handle);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::registerWindowClass()
{
    WNDCLASSW windowClass;
    windowClass.style         = 0;
    windowClass.lpfnWndProc   = &WindowImplWin32::globalOnEvent;
    windowClass.cbClsExtra    = 0;
    windowClass.cbWndExtra    = 0;
    windowClass.hInstance     = GetModuleHandleW(nullptr);
    windowClass.hIcon         = nullptr;
    windowClass.hCursor       = nullptr;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName  = nullptr;
    windowClass.lpszClassName = className;
    RegisterClassW(&windowClass);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::switchToFullscreen(const VideoMode& mode)
{
    DEVMODEW devMode;
    devMode.dmSize       = sizeof(devMode);
    devMode.dmPelsWidth  = mode.size.x;
    devMode.dmPelsHeight = mode.size.y;
    devMode.dmBitsPerPel = mode.bitsPerPixel;
    devMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

    // Apply fullscreen mode
    if (ChangeDisplaySettingsW(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
    {
        priv::err() << "Failed to change display mode for fullscreen";
        return;
    }

    // Make the window flags compatible with fullscreen mode
    SetWindowLongPtr(m_handle,
                     GWL_STYLE,
                     static_cast<LONG_PTR>(WS_POPUP) | static_cast<LONG_PTR>(WS_CLIPCHILDREN) |
                         static_cast<LONG_PTR>(WS_CLIPSIBLINGS));
    SetWindowLongPtr(m_handle, GWL_EXSTYLE, WS_EX_APPWINDOW);

    // Resize the window so that it fits the entire screen
    SetWindowPos(m_handle, HWND_TOP, 0, 0, static_cast<int>(mode.size.x), static_cast<int>(mode.size.y), SWP_FRAMECHANGED);
    ShowWindow(m_handle, SW_SHOW);

    // Set "this" as the current fullscreen window
    fullscreenWindow = this;
}


////////////////////////////////////////////////////////////
void WindowImplWin32::cleanup()
{
    // Restore the previous video mode (in case we were running in fullscreen)
    if (fullscreenWindow == this)
    {
        ChangeDisplaySettingsW(nullptr, 0);
        fullscreenWindow = nullptr;
    }

    // Unhide the mouse cursor (in case it was hidden)
    setMouseCursorVisible(true);

    // No longer track the cursor
    setTracking(false);

    // No longer capture the cursor
    ReleaseCapture();
}


////////////////////////////////////////////////////////////
void WindowImplWin32::setTracking(bool track)
{
    TRACKMOUSEEVENT mouseEvent;
    mouseEvent.cbSize      = sizeof(TRACKMOUSEEVENT);
    mouseEvent.dwFlags     = track ? TME_LEAVE : TME_CANCEL;
    mouseEvent.hwndTrack   = m_handle;
    mouseEvent.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&mouseEvent);
}


////////////////////////////////////////////////////////////
void WindowImplWin32::grabCursor(bool grabbed)
{
    if (grabbed)
    {
        RECT rect;
        GetClientRect(m_handle, &rect);
        MapWindowPoints(m_handle, nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
        ClipCursor(&rect);
    }
    else
    {
        ClipCursor(nullptr);
    }
}


////////////////////////////////////////////////////////////
Vector2i WindowImplWin32::contentSizeToWindowSize(Vector2u size)
{
    // SetWindowPos wants the total size of the window (including title bar, borders, and menu) so we have to compute it
    const auto style   = static_cast<DWORD>(GetWindowLongPtr(m_handle, GWL_STYLE));
    const BOOL hasMenu = ((style & WS_CHILD) == 0) && GetMenu(m_handle) != nullptr;
    const auto exStyle = static_cast<DWORD>(GetWindowLongPtr(m_handle, GWL_EXSTYLE));

    RECT rectangle = {0, 0, static_cast<long>(size.x), static_cast<long>(size.y)};
    AdjustWindowRectEx(&rectangle, style, hasMenu, exStyle);
    const auto width  = rectangle.right - rectangle.left;
    const auto height = rectangle.bottom - rectangle.top;

    return {width, height};
}


////////////////////////////////////////////////////////////
Keyboard::Scancode WindowImplWin32::toScancode(WPARAM wParam, LPARAM lParam)
{
    int code = (lParam & (0xFF << 16)) >> 16;

    // Retrieve the scancode from the VirtualKey for synthetic key messages
    if (code == 0)
    {
        code = static_cast<int>(MapVirtualKey(static_cast<UINT>(wParam), MAPVK_VK_TO_VSC));
    }

    // Windows scancodes
    // Reference: https://msdn.microsoft.com/en-us/library/aa299374(v=vs.60).aspx
    // clang-format off
    switch (code)
    {
        case 1: return Keyboard::Scan::Escape;
        case 2: return Keyboard::Scan::Num1;
        case 3: return Keyboard::Scan::Num2;
        case 4: return Keyboard::Scan::Num3;
        case 5: return Keyboard::Scan::Num4;
        case 6: return Keyboard::Scan::Num5;
        case 7: return Keyboard::Scan::Num6;
        case 8: return Keyboard::Scan::Num7;
        case 9: return Keyboard::Scan::Num8;
        case 10: return Keyboard::Scan::Num9;
        case 11: return Keyboard::Scan::Num0;
        case 12: return Keyboard::Scan::Hyphen;
        case 13: return Keyboard::Scan::Equal;
        case 14: return Keyboard::Scan::Backspace;
        case 15: return Keyboard::Scan::Tab;
        case 16: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::MediaPreviousTrack : Keyboard::Scan::Q;
        case 17: return Keyboard::Scan::W;
        case 18: return Keyboard::Scan::E;
        case 19: return Keyboard::Scan::R;
        case 20: return Keyboard::Scan::T;
        case 21: return Keyboard::Scan::Y;
        case 22: return Keyboard::Scan::U;
        case 23: return Keyboard::Scan::I;
        case 24: return Keyboard::Scan::O;
        case 25: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::MediaNextTrack     : Keyboard::Scan::P;
        case 26: return Keyboard::Scan::LBracket;
        case 27: return Keyboard::Scan::RBracket;
        case 28: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::NumpadEnter        : Keyboard::Scan::Enter;
        case 29: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::RControl           : Keyboard::Scan::LControl;
        case 30: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Select             : Keyboard::Scan::A;
        case 31: return Keyboard::Scan::S;
        case 32: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::VolumeMute         : Keyboard::Scan::D;
        case 33: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::LaunchApplication1 : Keyboard::Scan::F;
        case 34: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::MediaPlayPause     : Keyboard::Scan::G;
        case 35: return Keyboard::Scan::H;
        case 36: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::MediaStop          : Keyboard::Scan::J;
        case 37: return Keyboard::Scan::K;
        case 38: return Keyboard::Scan::L;
        case 39: return Keyboard::Scan::Semicolon;
        case 40: return Keyboard::Scan::Apostrophe;
        case 41: return Keyboard::Scan::Grave;
        case 42: return Keyboard::Scan::LShift;
        case 43: return Keyboard::Scan::Backslash;
        case 44: return Keyboard::Scan::Z;
        case 45: return Keyboard::Scan::X;
        case 46: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::VolumeDown   : Keyboard::Scan::C;
        case 47: return Keyboard::Scan::V;
        case 48: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::VolumeUp     : Keyboard::Scan::B;
        case 49: return Keyboard::Scan::N;
        case 50: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::HomePage     : Keyboard::Scan::M;
        case 51: return Keyboard::Scan::Comma;
        case 52: return Keyboard::Scan::Period;
        case 53: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::NumpadDivide : Keyboard::Scan::Slash;
        case 54: return Keyboard::Scan::RShift;
        case 55: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::PrintScreen  : Keyboard::Scan::NumpadMultiply;
        case 56: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::RAlt         : Keyboard::Scan::LAlt;
        case 57: return Keyboard::Scan::Space;
        case 58: return Keyboard::Scan::CapsLock;
        case 59: return Keyboard::Scan::F1;
        case 60: return Keyboard::Scan::F2;
        case 61: return Keyboard::Scan::F3;
        case 62: return Keyboard::Scan::F4;
        case 63: return Keyboard::Scan::F5;
        case 64: return Keyboard::Scan::F6;
        case 65: return Keyboard::Scan::F7;
        case 66: return Keyboard::Scan::F8;
        case 67: return Keyboard::Scan::F9;
        case 68: return Keyboard::Scan::F10;
        case 69: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::NumLock  : Keyboard::Scan::Pause;
        case 70: return Keyboard::Scan::ScrollLock;
        case 71: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Home     : Keyboard::Scan::Numpad7;
        case 72: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Up       : Keyboard::Scan::Numpad8;
        case 73: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::PageUp   : Keyboard::Scan::Numpad9;
        case 74: return Keyboard::Scan::NumpadMinus;
        case 75: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Left     : Keyboard::Scan::Numpad4;
        case 76: return Keyboard::Scan::Numpad5;
        case 77: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Right    : Keyboard::Scan::Numpad6;
        case 78: return Keyboard::Scan::NumpadPlus;
        case 79: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::End      : Keyboard::Scan::Numpad1;
        case 80: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Down     : Keyboard::Scan::Numpad2;
        case 81: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::PageDown : Keyboard::Scan::Numpad3;
        case 82: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Insert   : Keyboard::Scan::Numpad0;
        case 83: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Delete   : Keyboard::Scan::NumpadDecimal;

        case 86: return Keyboard::Scan::NonUsBackslash;
        case 87: return Keyboard::Scan::F11;
        case 88: return Keyboard::Scan::F12;

        case 91: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::LSystem : Keyboard::Scan::Unknown;
        case 92: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::RSystem : Keyboard::Scan::Unknown;
        case 93: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Menu    : Keyboard::Scan::Unknown;

        case 99: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Help    : Keyboard::Scan::Unknown;
        case 100: return Keyboard::Scan::F13;
        case 101: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Search             : Keyboard::Scan::F14;
        case 102: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Favorites          : Keyboard::Scan::F15;
        case 103: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Refresh            : Keyboard::Scan::F16;
        case 104: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Stop               : Keyboard::Scan::F17;
        case 105: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Forward            : Keyboard::Scan::F18;
        case 106: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::Back               : Keyboard::Scan::F19;
        case 107: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::LaunchApplication1 : Keyboard::Scan::F20;
        case 108: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::LaunchMail         : Keyboard::Scan::F21;
        case 109: return (HIWORD(lParam) & KF_EXTENDED) ? Keyboard::Scan::LaunchMediaSelect  : Keyboard::Scan::F22;
        case 110: return Keyboard::Scan::F23;

        case 118: return Keyboard::Scan::F24;

        default: return Keyboard::Scan::Unknown;
    }
    // clang-format on
}


////////////////////////////////////////////////////////////
void WindowImplWin32::processEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
    // Don't process any message until window is created
    if (m_handle == nullptr)
        return;

    switch (message)
    {
        // Destroy event
        case WM_DESTROY:
        {
            // Here we must cleanup resources !
            cleanup();
            break;
        }

        // Set cursor event
        case WM_SETCURSOR:
        {
            // The mouse has moved, if the cursor is in our window we must refresh the cursor
            if (LOWORD(lParam) == HTCLIENT)
            {
                SetCursor(m_cursorVisible ? m_lastCursor : nullptr);
            }

            break;
        }

        // Close event
        case WM_CLOSE:
        {
            pushEvent(Event::Closed{});
            break;
        }

        // Resize event
        case WM_SIZE:
        {
            // Consider only events triggered by a maximize or a un-maximize
            if (wParam != SIZE_MINIMIZED && !m_resizing && m_lastSize != getSize())
            {
                // Update the last handled size
                m_lastSize = getSize();

                // Push a resize event
                pushEvent(Event::Resized{m_lastSize});

                // Restore/update cursor grabbing
                grabCursor(m_cursorGrabbed);
            }
            break;
        }

        // Start resizing
        case WM_ENTERSIZEMOVE:
        {
            m_resizing = true;
            grabCursor(false);
            break;
        }

        // Stop resizing
        case WM_EXITSIZEMOVE:
        {
            m_resizing = false;

            // Ignore cases where the window has only been moved
            if (m_lastSize != getSize())
            {
                // Update the last handled size
                m_lastSize = getSize();

                // Push a resize event
                pushEvent(Event::Resized{m_lastSize});
            }

            // Restore/update cursor grabbing
            grabCursor(m_cursorGrabbed);
            break;
        }

        // Fix violations of minimum or maximum size
        case WM_GETMINMAXINFO:
        {
            // We override the returned information to remove the default limit
            // (the OS doesn't allow windows bigger than the desktop by default)

            const auto maximumSize = contentSizeToWindowSize(getMaximumSize().valueOr(Vector2u(50'000, 50'000)));

            MINMAXINFO& minMaxInfo      = *reinterpret_cast<PMINMAXINFO>(lParam);
            minMaxInfo.ptMaxTrackSize.x = maximumSize.x;
            minMaxInfo.ptMaxTrackSize.y = maximumSize.y;
            if (getMaximumSize().hasValue())
            {
                minMaxInfo.ptMaxSize.x = maximumSize.x;
                minMaxInfo.ptMaxSize.y = maximumSize.y;
            }
            if (getMinimumSize().hasValue())
            {
                const auto minimumSize      = contentSizeToWindowSize(getMinimumSize().value());
                minMaxInfo.ptMinTrackSize.x = minimumSize.x;
                minMaxInfo.ptMinTrackSize.y = minimumSize.y;
            }
            break;
        }

        // Gain focus event
        case WM_SETFOCUS:
        {
            // Restore cursor grabbing
            grabCursor(m_cursorGrabbed);

            pushEvent(Event::FocusGained{});
            break;
        }

        // Lost focus event
        case WM_KILLFOCUS:
        {
            // Ungrab the cursor
            grabCursor(false);

            pushEvent(Event::FocusLost{});
            break;
        }

        // Text event
        case WM_CHAR:
        {
            if (m_keyRepeatEnabled || ((lParam & (1 << 30)) == 0))
            {
                // Get the code of the typed character
                auto character = static_cast<char32_t>(wParam);

                // Check if it is the first part of a surrogate pair, or a regular character
                if ((character >= 0xD8'00) && (character <= 0xDB'FF))
                {
                    // First part of a surrogate pair: store it and wait for the second one
                    m_surrogate = static_cast<char16_t>(character);
                }
                else
                {
                    // Check if it is the second part of a surrogate pair, or a regular character
                    if ((character >= 0xDC'00) && (character <= 0xDF'FF))
                    {
                        // Convert the UTF-16 surrogate pair to a single UTF-32 value
                        char16_t utf16[]{m_surrogate, static_cast<char16_t>(character)};
                        sf::Utf16::toUtf32(utf16, utf16 + 2, &character);
                        m_surrogate = 0;
                    }

                    // Send a TextEntered event
                    pushEvent(Event::TextEntered{character});
                }
            }
            break;
        }

        // Keydown event
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if (m_keyRepeatEnabled || ((HIWORD(lParam) & KF_REPEAT) == 0))
            {
                Event::KeyPressed event;
                event.alt      = HIWORD(GetKeyState(VK_MENU)) != 0;
                event.control  = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                event.shift    = HIWORD(GetKeyState(VK_SHIFT)) != 0;
                event.system   = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                event.code     = virtualKeyCodeToSF(wParam, lParam);
                event.scancode = toScancode(wParam, lParam);
                pushEvent(event);
            }
            break;
        }

        // Keyup event
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Event::KeyReleased event;
            event.alt      = HIWORD(GetKeyState(VK_MENU)) != 0;
            event.control  = HIWORD(GetKeyState(VK_CONTROL)) != 0;
            event.shift    = HIWORD(GetKeyState(VK_SHIFT)) != 0;
            event.system   = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
            event.code     = virtualKeyCodeToSF(wParam, lParam);
            event.scancode = toScancode(wParam, lParam);
            pushEvent(event);
            break;
        }

        // Vertical mouse wheel event
        case WM_MOUSEWHEEL:
        {
            // Mouse position is in screen coordinates, convert it to window coordinates
            POINT position;
            position.x = static_cast<base::I16>(LOWORD(lParam));
            position.y = static_cast<base::I16>(HIWORD(lParam));
            ScreenToClient(m_handle, &position);

            auto delta = static_cast<base::I16>(HIWORD(wParam));

            Event::MouseWheelScrolled event;
            event.wheel    = Mouse::Wheel::Vertical;
            event.delta    = static_cast<float>(delta) / 120.f;
            event.position = {position.x, position.y};
            pushEvent(event);
            break;
        }

        // Horizontal mouse wheel event
        case WM_MOUSEHWHEEL:
        {
            // Mouse position is in screen coordinates, convert it to window coordinates
            POINT position;
            position.x = static_cast<base::I16>(LOWORD(lParam));
            position.y = static_cast<base::I16>(HIWORD(lParam));
            ScreenToClient(m_handle, &position);

            auto delta = static_cast<base::I16>(HIWORD(wParam));

            Event::MouseWheelScrolled event;
            event.wheel    = Mouse::Wheel::Horizontal;
            event.delta    = -static_cast<float>(delta) / 120.f;
            event.position = {position.x, position.y};
            pushEvent(event);
            break;
        }

        // Mouse left button down event
        case WM_LBUTTONDOWN:
        {
            Event::MouseButtonPressed event;
            event.button   = Mouse::Button::Left;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        // Mouse left button up event
        case WM_LBUTTONUP:
        {
            Event::MouseButtonReleased event;
            event.button   = Mouse::Button::Left;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        // Mouse right button down event
        case WM_RBUTTONDOWN:
        {
            Event::MouseButtonPressed event;
            event.button   = Mouse::Button::Right;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        // Mouse right button up event
        case WM_RBUTTONUP:
        {
            Event::MouseButtonReleased event;
            event.button   = Mouse::Button::Right;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        // Mouse wheel button down event
        case WM_MBUTTONDOWN:
        {
            Event::MouseButtonPressed event;
            event.button   = Mouse::Button::Middle;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        // Mouse wheel button up event
        case WM_MBUTTONUP:
        {
            Event::MouseButtonReleased event;
            event.button   = Mouse::Button::Middle;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        // Mouse X button down event
        case WM_XBUTTONDOWN:
        {
            Event::MouseButtonPressed event;
            event.button   = HIWORD(wParam) == XBUTTON1 ? Mouse::Button::Extra1 : Mouse::Button::Extra2;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        // Mouse X button up event
        case WM_XBUTTONUP:
        {
            Event::MouseButtonReleased event;
            event.button   = HIWORD(wParam) == XBUTTON1 ? Mouse::Button::Extra1 : Mouse::Button::Extra2;
            event.position = {static_cast<base::I16>(LOWORD(lParam)), static_cast<base::I16>(HIWORD(lParam))};
            pushEvent(event);
            break;
        }

        case WM_POINTERDOWN:
        case WM_POINTERUPDATE:
        case WM_POINTERUP:
        {
            UINT32       pointerId = GET_POINTERID_WPARAM(wParam);
            POINTER_INFO pointerInfo;
            if (GetPointerInfo(pointerId, &pointerInfo))
            {
                if (pointerInfo.pointerType == PT_TOUCH)
                {
                    // Convert screen coordinates to client area coordinates
                    POINT pt = {pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y};
                    ScreenToClient(m_handle, &pt);

                    const sf::Vector2i touchPos{pt.x, pt.y};

                    // Get normalized index
                    const auto findFirstNormalizedIndex = [&]
                    {
                        for (base::SizeT i = 0; i < 10; ++i)
                            if (!touchIndexPool[i])
                                return static_cast<int>(i);

                        sf::priv::err() << "No available touch index\n";
                        return -1;
                    };

                    switch (message) // 'msg' is the current message being processed
                    {
                        case WM_POINTERDOWN:
                        {
                            SFML_BASE_ASSERT(!touchMap.contains(pointerId));

                            const int normalizedIndex = findFirstNormalizedIndex();
                            if (normalizedIndex == -1)
                                break;

                            const auto fingerIdx = static_cast<unsigned int>(normalizedIndex);

                            touchIndexPool[normalizedIndex] = true;
                            touchMap.emplace(pointerId, TouchInfo{fingerIdx, touchPos, m_handle});

                            pushEvent(sf::Event::TouchBegan{fingerIdx, touchPos});
                            break;
                        }
                        case WM_POINTERUPDATE:
                        {
                            SFML_BASE_ASSERT(touchMap.contains(pointerId));
                            const auto [fingerIdx, pos, handle] = touchMap[pointerId];

                            pushEvent(sf::Event::TouchMoved{fingerIdx, touchPos});
                            break;
                        }
                        case WM_POINTERUP:
                        {
                            SFML_BASE_ASSERT(touchMap.contains(pointerId));
                            const auto [fingerIdx, pos, handle] = touchMap[pointerId];

                            touchIndexPool[fingerIdx] = false;
                            touchMap.erase(pointerId);

                            pushEvent(sf::Event::TouchEnded{fingerIdx, touchPos});
                            break;
                        }
                    }
                }
            }
            break;
        }

        // Mouse leave event
        case WM_MOUSELEAVE:
        {
            // Avoid this firing a second time in case the cursor is dragged outside
            if (m_mouseInside)
            {
                m_mouseInside = false;

                // Generate a MouseLeft event
                pushEvent(Event::MouseLeft{});
            }
            break;
        }

        // Mouse move event
        case WM_MOUSEMOVE:
        {
            // Extract the mouse local coordinates
            const int x = static_cast<base::I16>(LOWORD(lParam));
            const int y = static_cast<base::I16>(HIWORD(lParam));

            // Get the client area of the window
            RECT area;
            GetClientRect(m_handle, &area);

            // Capture the mouse in case the user wants to drag it outside
            if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) == 0)
            {
                // Only release the capture if we really have it
                if (GetCapture() == m_handle)
                    ReleaseCapture();
            }
            else if (GetCapture() != m_handle)
            {
                // Set the capture to continue receiving mouse events
                SetCapture(m_handle);
            }

            // If the cursor is outside the client area...
            if ((x < area.left) || (x > area.right) || (y < area.top) || (y > area.bottom))
            {
                // and it used to be inside, the mouse left it.
                if (m_mouseInside)
                {
                    m_mouseInside = false;

                    // No longer care for the mouse leaving the window
                    setTracking(false);

                    // Generate a MouseLeft event
                    pushEvent(Event::MouseLeft{});
                }
            }
            else
            {
                // and vice-versa
                if (!m_mouseInside)
                {
                    m_mouseInside = true;

                    // Look for the mouse leaving the window
                    setTracking(true);

                    // Generate a MouseEntered event
                    pushEvent(Event::MouseEntered{});
                }
            }

            // Generate a MouseMove event
            pushEvent(Event::MouseMoved{{x, y}});
            break;
        }

        // Raw input event
        case WM_INPUT:
        {
            RAWINPUT input;
            UINT     size = sizeof(input);

            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER));

            if (input.header.dwType == RIM_TYPEMOUSE)
            {
                if (const RAWMOUSE* rawMouse = &input.data.mouse; (rawMouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
                    pushEvent(Event::MouseMovedRaw{{rawMouse->lLastX, rawMouse->lLastY}});
            }

            break;
        }

        // Work around Windows 10 bug
        // When a maximum size is specified and the window is snapped to the edge of the display the window size is subtly too big
        case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS& pos = *reinterpret_cast<PWINDOWPOS>(lParam);
            if (pos.flags & SWP_NOSIZE)
                break;

            if (!getMaximumSize().hasValue())
                break;
            const auto maximumSize = contentSizeToWindowSize(getMaximumSize().value());

            bool shouldResize = false;
            if (pos.cx > maximumSize.x)
            {
                pos.cx       = maximumSize.x;
                shouldResize = true;
            }
            if (pos.cy > maximumSize.y)
            {
                pos.cy       = maximumSize.y;
                shouldResize = true;
            }

            if (shouldResize)
                SetWindowPos(m_handle, pos.hwndInsertAfter, pos.x, pos.y, pos.cx, pos.cy, 0);

            break;
        }
    }
}


////////////////////////////////////////////////////////////
Keyboard::Key WindowImplWin32::virtualKeyCodeToSF(WPARAM key, LPARAM flags)
{
    // clang-format off
    switch (key)
    {
        // Check the scancode to distinguish between left and right shift
        case VK_SHIFT:
        {
            static const UINT lShift = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
            const UINT scancode = static_cast<UINT>((flags & (0xFF << 16)) >> 16);
            return scancode == lShift ? Keyboard::Key::LShift : Keyboard::Key::RShift;
        }

        // Check the "extended" flag to distinguish between left and right alt
        case VK_MENU : return (HIWORD(flags) & KF_EXTENDED) ? Keyboard::Key::RAlt : Keyboard::Key::LAlt;

        // Check the "extended" flag to distinguish between left and right control
        case VK_CONTROL : return (HIWORD(flags) & KF_EXTENDED) ? Keyboard::Key::RControl : Keyboard::Key::LControl;

        // Other keys are reported properly
        case VK_LWIN:       return Keyboard::Key::LSystem;
        case VK_RWIN:       return Keyboard::Key::RSystem;
        case VK_APPS:       return Keyboard::Key::Menu;
        case VK_OEM_1:      return Keyboard::Key::Semicolon;
        case VK_OEM_2:      return Keyboard::Key::Slash;
        case VK_OEM_PLUS:   return Keyboard::Key::Equal;
        case VK_OEM_MINUS:  return Keyboard::Key::Hyphen;
        case VK_OEM_4:      return Keyboard::Key::LBracket;
        case VK_OEM_6:      return Keyboard::Key::RBracket;
        case VK_OEM_COMMA:  return Keyboard::Key::Comma;
        case VK_OEM_PERIOD: return Keyboard::Key::Period;
        case VK_OEM_7:      return Keyboard::Key::Apostrophe;
        case VK_OEM_5:      return Keyboard::Key::Backslash;
        case VK_OEM_3:      return Keyboard::Key::Grave;
        case VK_ESCAPE:     return Keyboard::Key::Escape;
        case VK_SPACE:      return Keyboard::Key::Space;
        case VK_RETURN:     return Keyboard::Key::Enter;
        case VK_BACK:       return Keyboard::Key::Backspace;
        case VK_TAB:        return Keyboard::Key::Tab;
        case VK_PRIOR:      return Keyboard::Key::PageUp;
        case VK_NEXT:       return Keyboard::Key::PageDown;
        case VK_END:        return Keyboard::Key::End;
        case VK_HOME:       return Keyboard::Key::Home;
        case VK_INSERT:     return Keyboard::Key::Insert;
        case VK_DELETE:     return Keyboard::Key::Delete;
        case VK_ADD:        return Keyboard::Key::Add;
        case VK_SUBTRACT:   return Keyboard::Key::Subtract;
        case VK_MULTIPLY:   return Keyboard::Key::Multiply;
        case VK_DIVIDE:     return Keyboard::Key::Divide;
        case VK_PAUSE:      return Keyboard::Key::Pause;
        case VK_F1:         return Keyboard::Key::F1;
        case VK_F2:         return Keyboard::Key::F2;
        case VK_F3:         return Keyboard::Key::F3;
        case VK_F4:         return Keyboard::Key::F4;
        case VK_F5:         return Keyboard::Key::F5;
        case VK_F6:         return Keyboard::Key::F6;
        case VK_F7:         return Keyboard::Key::F7;
        case VK_F8:         return Keyboard::Key::F8;
        case VK_F9:         return Keyboard::Key::F9;
        case VK_F10:        return Keyboard::Key::F10;
        case VK_F11:        return Keyboard::Key::F11;
        case VK_F12:        return Keyboard::Key::F12;
        case VK_F13:        return Keyboard::Key::F13;
        case VK_F14:        return Keyboard::Key::F14;
        case VK_F15:        return Keyboard::Key::F15;
        case VK_LEFT:       return Keyboard::Key::Left;
        case VK_RIGHT:      return Keyboard::Key::Right;
        case VK_UP:         return Keyboard::Key::Up;
        case VK_DOWN:       return Keyboard::Key::Down;
        case VK_NUMPAD0:    return Keyboard::Key::Numpad0;
        case VK_NUMPAD1:    return Keyboard::Key::Numpad1;
        case VK_NUMPAD2:    return Keyboard::Key::Numpad2;
        case VK_NUMPAD3:    return Keyboard::Key::Numpad3;
        case VK_NUMPAD4:    return Keyboard::Key::Numpad4;
        case VK_NUMPAD5:    return Keyboard::Key::Numpad5;
        case VK_NUMPAD6:    return Keyboard::Key::Numpad6;
        case VK_NUMPAD7:    return Keyboard::Key::Numpad7;
        case VK_NUMPAD8:    return Keyboard::Key::Numpad8;
        case VK_NUMPAD9:    return Keyboard::Key::Numpad9;
        case 'A':           return Keyboard::Key::A;
        case 'Z':           return Keyboard::Key::Z;
        case 'E':           return Keyboard::Key::E;
        case 'R':           return Keyboard::Key::R;
        case 'T':           return Keyboard::Key::T;
        case 'Y':           return Keyboard::Key::Y;
        case 'U':           return Keyboard::Key::U;
        case 'I':           return Keyboard::Key::I;
        case 'O':           return Keyboard::Key::O;
        case 'P':           return Keyboard::Key::P;
        case 'Q':           return Keyboard::Key::Q;
        case 'S':           return Keyboard::Key::S;
        case 'D':           return Keyboard::Key::D;
        case 'F':           return Keyboard::Key::F;
        case 'G':           return Keyboard::Key::G;
        case 'H':           return Keyboard::Key::H;
        case 'J':           return Keyboard::Key::J;
        case 'K':           return Keyboard::Key::K;
        case 'L':           return Keyboard::Key::L;
        case 'M':           return Keyboard::Key::M;
        case 'W':           return Keyboard::Key::W;
        case 'X':           return Keyboard::Key::X;
        case 'C':           return Keyboard::Key::C;
        case 'V':           return Keyboard::Key::V;
        case 'B':           return Keyboard::Key::B;
        case 'N':           return Keyboard::Key::N;
        case '0':           return Keyboard::Key::Num0;
        case '1':           return Keyboard::Key::Num1;
        case '2':           return Keyboard::Key::Num2;
        case '3':           return Keyboard::Key::Num3;
        case '4':           return Keyboard::Key::Num4;
        case '5':           return Keyboard::Key::Num5;
        case '6':           return Keyboard::Key::Num6;
        case '7':           return Keyboard::Key::Num7;
        case '8':           return Keyboard::Key::Num8;
        case '9':           return Keyboard::Key::Num9;
    }
    // clang-format on

    return Keyboard::Key::Unknown;
}


////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowImplWin32::globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Associate handle and Window instance when the creation message is received
    if (message == WM_CREATE)
    {
        // Get WindowImplWin32 instance (it was passed as the last argument of CreateWindow)
        auto window = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

        // Set as the "user data" parameter of the window
        SetWindowLongPtrW(handle, GWLP_USERDATA, window);
    }

    // Get the WindowImpl instance corresponding to the window handle
    WindowImplWin32* window = handle ? reinterpret_cast<WindowImplWin32*>(GetWindowLongPtr(handle, GWLP_USERDATA)) : nullptr;

    // Forward the event to the appropriate function
    if (window)
    {
        window->processEvent(message, wParam, lParam);

        if (message == WM_POINTERDOWN || message == WM_POINTERUPDATE || message == WM_POINTERUP)
            return 0;

        if (window->m_callback)
            return CallWindowProcW(reinterpret_cast<WNDPROC>(window->m_callback), handle, message, wParam, lParam);
    }

    // We don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
    if (message == WM_CLOSE)
        return 0;

    // Don't forward the menu system command, so that pressing ALT or F10 doesn't steal the focus
    if ((message == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
        return 0;

    return DefWindowProcW(handle, message, wParam, lParam);
}

} // namespace sf::priv
