// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Clipboard.hpp"

#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Utf8String.hpp"


namespace sf
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

} // namespace sf
