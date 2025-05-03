#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Clipboard.hpp"
#include "SFML/Window/SDLLayer.hpp"

#include "SFML/System/String.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
String Clipboard::getString()
{
    return priv::getSDLLayerSingleton().getClipboardString();
}


////////////////////////////////////////////////////////////
bool Clipboard::setString(const String& text)
{
    return priv::getSDLLayerSingleton().setClipboardString(text);
}

} // namespace sf
