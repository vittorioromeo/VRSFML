// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Utf.hpp"
#include "Zancle/System/WindowsHeader.hpp"
#include "Zancle/Window/Win32/Utils.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/BackInserter.hpp"
#include "ZancleBase/String.hpp"

#include <cwchar>


namespace za::priv
{
////////////////////////////////////////////////////////////
zb::String getErrorString(DWORD error)
{
    PTCHAR buffer = nullptr;
    if (FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      nullptr,
                      error,
                      0,
                      reinterpret_cast<PTCHAR>(&buffer),
                      0,
                      nullptr) == 0)
    {
        return "Unknown error.";
    }

    const auto srcLen = std::wcslen(buffer);

    zb::String message;
    message.reserve(srcLen * 3u);

    Utf<16>::toUtf8(buffer, buffer + srcLen, zb::BackInserter{message});

    LocalFree(buffer);
    return message;
}


////////////////////////////////////////////////////////////
void setWindowBorderless(void* const hwnd, const unsigned int width, const unsigned int height)
{
    ZB_ASSERT(hwnd != nullptr);

    auto* hwndPtr = reinterpret_cast<HWND>(hwnd);

    SetWindowLongPtr(hwndPtr, GWL_STYLE, WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    SetWindowLongPtr(hwndPtr, GWL_EXSTYLE, WS_EX_APPWINDOW);
    SetWindowPos(hwndPtr,
                 HWND_TOP,
                 0,
                 0,
                 static_cast<int>(width),
                 static_cast<int>(height),
                 SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
}

} // namespace za::priv
