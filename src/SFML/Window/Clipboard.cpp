#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/ClipboardImpl.hpp>

#include <SFML/System/String.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
String Clipboard::getString()
{
    return priv::ClipboardImpl::getString();
}


////////////////////////////////////////////////////////////
void Clipboard::setString(const String& text)
{
    priv::ClipboardImpl::setString(text);
}

} // namespace sf
