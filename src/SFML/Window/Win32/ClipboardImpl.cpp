#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Win32/ClipboardImpl.hpp>
#include <SFML/Window/Win32/Utils.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Win32/WindowsHeader.hpp>

#include <string>

#include <cstring>


namespace sf::priv
{
////////////////////////////////////////////////////////////
String ClipboardImpl::getString()
{
    String text;

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        err() << "Failed to get the clipboard data in Unicode format: " << getErrorString(GetLastError());
        return text;
    }

    if (!OpenClipboard(nullptr))
    {
        err() << "Failed to open the Win32 clipboard: " << getErrorString(GetLastError());
        return text;
    }

    HANDLE clipboardHandle = GetClipboardData(CF_UNICODETEXT);

    if (!clipboardHandle)
    {
        err() << "Failed to get Win32 handle for clipboard content: " << getErrorString(GetLastError());
        CloseClipboard();

        return text;
    }

    text = String(static_cast<wchar_t*>(GlobalLock(clipboardHandle)));
    GlobalUnlock(clipboardHandle);

    CloseClipboard();
    return text;
}


////////////////////////////////////////////////////////////
void ClipboardImpl::setString(const String& text)
{
    if (!OpenClipboard(nullptr))
    {
        err() << "Failed to open the Win32 clipboard: " << getErrorString(GetLastError());
        return;
    }

    if (!EmptyClipboard())
    {
        err() << "Failed to empty the Win32 clipboard: " << getErrorString(GetLastError());
        CloseClipboard();

        return;
    }

    // Create a Win32-compatible string
    const std::size_t stringSize   = (text.getSize() + 1) * sizeof(WCHAR);
    HANDLE            stringHandle = GlobalAlloc(GMEM_MOVEABLE, stringSize);

    if (stringHandle)
    {
        std::memcpy(GlobalLock(stringHandle), text.toWideString().data(), stringSize);
        GlobalUnlock(stringHandle);
        SetClipboardData(CF_UNICODETEXT, stringHandle);
    }

    CloseClipboard();
}

} // namespace sf::priv
