#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Win32/ClipboardImpl.hpp"
#include "SFML/Window/Win32/Utils.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/String.hpp"
#include "SFML/System/StringUtfUtils.hpp"
#include "SFML/System/Win32/WindowsHeader.hpp"

#include "SFML/Base/Builtins/Memcpy.hpp"

#include <string>


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

    const std::u16string_view string(static_cast<const char16_t*>(GlobalLock(clipboardHandle)));
    text = StringUtfUtils::fromUtf16(string.begin(), string.end());
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
    const auto        string     = text.toUtf16<std::u16string>();
    const std::size_t stringSize = (string.size() + 1) * sizeof(char16_t);

    if (const HANDLE stringHandle = GlobalAlloc(GMEM_MOVEABLE, stringSize))
    {
        SFML_BASE_MEMCPY(GlobalLock(stringHandle), string.data(), stringSize);
        GlobalUnlock(stringHandle);
        SetClipboardData(CF_UNICODETEXT, stringHandle);
    }

    CloseClipboard();
}

} // namespace sf::priv
