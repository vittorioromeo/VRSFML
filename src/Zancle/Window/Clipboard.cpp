// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Clipboard.hpp"

#include "Zancle/Window/SDLLayer.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/String/Utf8String.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Utf8String Clipboard::getString()
{
    return WindowContext::getSDLLayer().getClipboardString();
}


////////////////////////////////////////////////////////////
bool Clipboard::setString(const Utf8String& text)
{
    return WindowContext::getSDLLayer().setClipboardString(text);
}

} // namespace za
