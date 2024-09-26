#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Stub/StubCursorImpl.hpp"

#include "SFML/System/Err.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
bool StubCursorImpl::loadFromPixels(const base::U8* /* pixels */, Vector2u /* size */, Vector2u /* hotspot */)
{
    // err() << "Cursor API not implemented";
    return false;
}


////////////////////////////////////////////////////////////
bool StubCursorImpl::loadFromSystem(Cursor::Type /* type */)
{
    // err() << "Cursor API not implemented";
    return false;
}


} // namespace sf::priv
