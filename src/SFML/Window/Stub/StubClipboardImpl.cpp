#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Stub/StubClipboardImpl.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/String.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
String StubClipboardImpl::getString()
{
    // err() << "Clipboard API not implemented";
    return {};
}


////////////////////////////////////////////////////////////
void StubClipboardImpl::setString(const String& /* text */)
{
    // err() << "Clipboard API not implemented";
}

} // namespace sf::priv
